
#include "CPUDevice.h"
#include "CPUCompiler.h"
#include "CPUPasses.h"
#include "CPUThread.h"
#include "ImageSupport.h"
#include "InternalCalls.h"

#include "opencrun/Core/CommandQueue.h"
#include "opencrun/Core/Event.h"
#include "opencrun/Core/Platform.h"
#include "opencrun/Device/Devices.h"
#include "opencrun/Passes/AllPasses.h"
#include "opencrun/Util/BuiltinInfo.h"


#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Linker/Linker.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ThreadLocal.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"

#include <limits>

using namespace opencrun;
using namespace opencrun::cpu;

//
// CPUMemoryDescriptor implementation.
//

CPUMemoryDescriptor::~CPUMemoryDescriptor() {
  if (Obj.getParentObject())
    return;

  // If UseHostPtr is set, we do not own the storage.
  if (Obj.getFlags() & MemoryObject::UseHostPtr)
    return;

  sys::Free(Ptr);
}

static MemoryDescriptor &getParentDescriptor(const Device &Dev,
                                             const MemoryObject &Obj) {
  if (const auto *Img = llvm::dyn_cast<Image>(&Obj))
    if (Img->getType() == Image::Image1D_Buffer)
      return Img->getBuffer()->getDescriptorFor(Dev);

  const auto *Buf = llvm::cast<Buffer>(&Obj);
  return Buf->getParent()->getDescriptorFor(Dev);
}

void *CPUMemoryDescriptor::allocateStorage() {
  // Handle sub-objects.
  if (auto *Parent = Obj.getParentObject()) {
    auto &ParentDesc = getParentDescriptor(Dev, Obj);
    auto &CPUParentDesc = static_cast<CPUMemoryDescriptor&>(ParentDesc);
    if (void *Base = CPUParentDesc.allocateStorage())
      return reinterpret_cast<uint8_t*>(Base) + Parent->getParentOffset();

    return nullptr;
  }

  assert(!isAllocated());

  // If UseHostPtr is set, do not duplicate the storage.
  if (Obj.getFlags() & MemoryObject::UseHostPtr)
    return Obj.getHostPtr();

  return sys::CacheAlignedAlloc(Obj.getSize());
}

bool CPUMemoryDescriptor::allocate() {
  Ptr = allocateStorage();
  return Allocated = Ptr != nullptr;
}

bool CPUMemoryDescriptor::aliasWithHostPtr() const {
  return isAllocated() && (Obj.getFlags() & MemoryObject::UseHostPtr);
}

void *CPUMemoryDescriptor::map() {
  tryAllocate();

  return Ptr;
}

void CPUMemoryDescriptor::unmap() {
}

CPUImageDescriptor::~CPUImageDescriptor() {
  sys::Free(ImgHeader);
}

bool CPUImageDescriptor::allocate() {
  Ptr = allocateStorage();
  if (!Ptr)
    return Allocated = false;

  auto *Hdr = (cpu_image_t*)sys::Alloc(sizeof(cpu_image_t));
  if (!Hdr)
    return Allocated = false;

  auto fixup = [](size_t X) {
    return X == 0 ? 1 : X;
  };

  auto &Img = *llvm::cast<Image>(&Obj);
  Hdr->image_channel_order = Img.getImageFormat().image_channel_order;
  Hdr->image_channel_data_type = Img.getImageFormat().image_channel_data_type;
  Hdr->num_channels = Img.getNumChannels();
  Hdr->element_size = Img.getElementSize();
  Hdr->width = fixup(Img.getWidth());
  Hdr->height = fixup(Img.getHeight());
  Hdr->depth = fixup(Img.getDepth());
  Hdr->row_pitch = fixup(Img.getRowPitch());
  Hdr->slice_pitch = fixup(Img.getSlicePitch());
  Hdr->array_size = fixup(Img.getArraySize());
  Hdr->num_mip_levels = 0;
  Hdr->num_samples = 0;
  Hdr->data = Ptr;
  ImgHeader = Hdr;

  return Allocated = true;
}

//
// CPUDevice implementation.
//

CPUDevice::CPUDevice(const sys::HardwareMachine &Machine)
 : Device(CPUType, "CPU"), Machine(Machine) {
  for (auto I = Machine.cpu_begin(), E = Machine.cpu_end(); I != E; ++I)
    HWCPUs.push_back(&*I);

  InitDeviceInfo();
  InitSubDeviceInfo();
  InitThreads();
  InitCompiler();
}

CPUDevice::CPUDevice(CPUDevice &Parent, const DevicePartition &Part,
                     llvm::ArrayRef<const sys::HardwareCPU*> CPUs)
 : Device(Parent, Part), Machine(Parent.Machine),
   HWCPUs(CPUs.begin(), CPUs.end()) {
  InitSubDeviceInfo();
  InitThreads();
  InitCompiler();
}

CPUDevice::~CPUDevice() {}

void CPUDevice::InitThreads() {
  Threads.reserve(HWCPUs.size());
  for (auto *CPU : HWCPUs)
    Threads.push_back(llvm::make_unique<CPUThread>(*this, *CPU));
}

bool CPUDevice::ComputeGlobalWorkPartition(const WorkSizes &GW,
                                           WorkSizes &LW) const {
  static const size_t Presets[3][3] = {
    {1024, 0, 0}, {32, 32, 0}, {16, 8, 8}
  };

  size_t N = GW.size();

  for (unsigned i = 0; i != N; ++i) {
    size_t Dim = GW[i];
    if (Presets[N][i] < Dim)
      Dim = llvm::GreatestCommonDivisor64(Dim, Presets[N][i]);

    LW.push_back(Dim);
  }

  return true;
}

std::unique_ptr<MemoryDescriptor>
CPUDevice::createMemoryDescriptor(const MemoryObject &Obj) {
  if (auto *Img = llvm::dyn_cast<Image>(&Obj))
    return llvm::make_unique<CPUImageDescriptor>(*this, *Img);
  return llvm::make_unique<CPUMemoryDescriptor>(*this, Obj);
}

bool CPUDevice::Submit(Command &Cmd) {
  bool Submitted = false;

  // Take the profiling information here, in order to force this sample
  // happening before the subsequents samples.
  auto Sample = ProfileSample::getSubmitted();

#define DISPATCH(CmdType)                                          \
  case Command::CmdType:                                           \
    Submitted = Submit(*llvm::cast<Enqueue ## CmdType>(&Cmd)); \
    break;

  switch (Cmd.GetType()) {
  default: llvm_unreachable("Unknown command type!");
  DISPATCH(ReadBuffer)
  DISPATCH(WriteBuffer)
  DISPATCH(CopyBuffer)
  DISPATCH(ReadImage)
  DISPATCH(WriteImage)
  DISPATCH(CopyImage)
  DISPATCH(CopyImageToBuffer)
  DISPATCH(CopyBufferToImage)
  DISPATCH(MapBuffer)
  DISPATCH(MapImage)
  DISPATCH(UnmapMemObject)
  DISPATCH(ReadBufferRect)
  DISPATCH(WriteBufferRect)
  DISPATCH(CopyBufferRect)
  DISPATCH(FillBuffer)
  DISPATCH(FillImage)
  DISPATCH(NDRangeKernel)
  DISPATCH(NativeKernel)
  DISPATCH(Marker)
  DISPATCH(Barrier)
  }

#undef DISPATCH

  // The command has been submitted, register the sample.
  if (Submitted)
    Cmd.GetNotifyEvent().MarkSubmitted(Sample);

  return Submitted;
}

void CPUDevice::UnregisterKernel(const KernelDescriptor &Kern) {
  llvm::sys::ScopedLock Lock(ThisLock);

  // Erase kernel from the cache.
  KernelFootprints.erase(&Kern);

  getCompilerAs<CPUCompiler>().removeKernel(Kern.getFunction(this));
}

void CPUDevice::InitDeviceInfo() {
  // Assuming symmetric systems.
  const sys::HardwareCache &L1Cache = Machine.socket_front().l1dc_front();
  const sys::HardwareCache &LLCache = Machine.socket_front().llc_front();

  // TODO: define device geometry and set all properties!

  VendorID = 0;
  MaxWorkItemDimensions = 3;
  MaxWorkItemSizes.assign(3, 1024);
  MaxWorkGroupSize = 1024;

  Version = "OpenCL 1.2 OpenCRun";
  OpenCLCVersion = "OpenCL C 1.2";
  Profile = "FULL_PROFILE";

  // TODO: AddressBits should be detected at runtime and consequently
  // Extensions should be set accordingly.
#if defined(__x86_64__)
  AddressBits = 64;
  Extensions = "cl_khr_fp64 "
               "cl_khr_global_int32_base_atomics "
               "cl_khr_global_int32_extended_atomics "
               "cl_khr_local_int32_base_atomics "
               "cl_khr_local_int32_extended_atomics "
               "cl_khr_int64_base_atomics "
               "cl_khr_int64_extended_atomics "
               "cl_khr_3d_image_writes";
#else
  AddressBits = 32;
  Extensions = "cl_khr_global_int32_base_atomics "
               "cl_khr_global_int32_extended_atomics "
               "cl_khr_local_int32_base_atomics "
               "cl_khr_local_int32_extended_atomics "
               "cl_khr_3d_image_writes";
#endif

  // TODO: CPU extensions (AVX, SSE, SSE2, SSE3, SSE4, ...) should
  // be detected at runtime and preferred and native widths should
  // be set accordingly.
#if defined(__AVX__)
  PreferredCharVectorWidth = 16;
  PreferredShortVectorWidth = 8;
  PreferredIntVectorWidth = 4;
  PreferredLongVectorWidth = 2;
  PreferredFloatVectorWidth = 4;
  PreferredDoubleVectorWidth = 2;

  NativeCharVectorWidth = 16;
  NativeShortVectorWidth = 8;
  NativeIntVectorWidth = 4;
  NativeLongVectorWidth = 2;
  NativeFloatVectorWidth = 8;
  NativeDoubleVectorWidth = 4;
#elif defined(__SSE2__)
  PreferredCharVectorWidth = 16;
  PreferredShortVectorWidth = 8;
  PreferredIntVectorWidth = 4;
  PreferredLongVectorWidth = 2;
  PreferredFloatVectorWidth = 4;
  PreferredDoubleVectorWidth = 2;

  NativeCharVectorWidth = 16;
  NativeShortVectorWidth = 8;
  NativeIntVectorWidth = 4;
  NativeLongVectorWidth = 2;
  NativeFloatVectorWidth = 4;
  NativeDoubleVectorWidth = 2;
#else
  PreferredCharVectorWidth = 1;
  PreferredShortVectorWidth = 1;
  PreferredIntVectorWidth = 1;
  PreferredLongVectorWidth = 1;
  PreferredFloatVectorWidth = 1;
  PreferredDoubleVectorWidth = 1;

  NativeCharVectorWidth = 1;
  NativeShortVectorWidth = 1;
  NativeIntVectorWidth = 1;
  NativeLongVectorWidth = 1;
  NativeFloatVectorWidth = 1;
  NativeDoubleVectorWidth = 1;
#endif

  PreferredHalfVectorWidth = PreferredShortVectorWidth;
  NativeHalfVectorWidth = NativeShortVectorWidth;

  // TODO: set MaxClockFrequency.

  MaxMemoryAllocSize = Machine.GetTotalMemorySize();

  // Image properties set to the minimum values for CPU target.
  static const cl_image_format CPUImgFmts[] = {
    { CL_R, CL_SNORM_INT8 }, { CL_R, CL_SNORM_INT16 },
    { CL_R, CL_UNORM_INT8 }, { CL_R, CL_UNORM_INT16 },
    { CL_R, CL_SIGNED_INT8 }, { CL_R, CL_SIGNED_INT16 },
    { CL_R, CL_SIGNED_INT32 }, { CL_R, CL_UNSIGNED_INT8 },
    { CL_R, CL_UNSIGNED_INT16 }, { CL_R, CL_UNSIGNED_INT32 },
    { CL_R, CL_HALF_FLOAT }, { CL_R, CL_FLOAT },
    { CL_A, CL_SNORM_INT8 }, { CL_A, CL_SNORM_INT16 },
    { CL_A, CL_UNORM_INT8 }, { CL_A, CL_UNORM_INT16 },
    { CL_A, CL_SIGNED_INT8 }, { CL_A, CL_SIGNED_INT16 },
    { CL_A, CL_SIGNED_INT32 }, { CL_A, CL_UNSIGNED_INT8 },
    { CL_A, CL_UNSIGNED_INT16 }, { CL_A, CL_UNSIGNED_INT32 },
    { CL_A, CL_HALF_FLOAT }, { CL_A, CL_FLOAT },
    { CL_RG, CL_SNORM_INT8 }, { CL_RG, CL_SNORM_INT16 },
    { CL_RG, CL_UNORM_INT8 }, { CL_RG, CL_UNORM_INT16 },
    { CL_RG, CL_SIGNED_INT8 }, { CL_RG, CL_SIGNED_INT16 },
    { CL_RG, CL_SIGNED_INT32 }, { CL_RG, CL_UNSIGNED_INT8 },
    { CL_RG, CL_UNSIGNED_INT16 }, { CL_RG, CL_UNSIGNED_INT32 },
    { CL_RG, CL_HALF_FLOAT }, { CL_RG, CL_FLOAT },
    { CL_RGBA, CL_SNORM_INT8 }, { CL_RGBA, CL_SNORM_INT16 },
    { CL_RGBA, CL_UNORM_INT8 }, { CL_RGBA, CL_UNORM_INT16 },
    { CL_RGBA, CL_SIGNED_INT8 }, { CL_RGBA, CL_SIGNED_INT16 },
    { CL_RGBA, CL_SIGNED_INT32 }, { CL_RGBA, CL_UNSIGNED_INT8 },
    { CL_RGBA, CL_UNSIGNED_INT16 }, { CL_RGBA, CL_UNSIGNED_INT32 },
    { CL_RGBA, CL_HALF_FLOAT }, { CL_RGBA, CL_FLOAT },
    { CL_ARGB, CL_SNORM_INT8 }, { CL_ARGB, CL_UNORM_INT8 },
    { CL_ARGB, CL_SIGNED_INT8 }, { CL_ARGB, CL_UNSIGNED_INT8 },
    { CL_BGRA, CL_SNORM_INT8 }, { CL_BGRA, CL_UNORM_INT8 },
    { CL_BGRA, CL_SIGNED_INT8 }, { CL_BGRA, CL_UNSIGNED_INT8 },
    { CL_LUMINANCE, CL_SNORM_INT8 }, { CL_LUMINANCE, CL_SNORM_INT16 },
    { CL_LUMINANCE, CL_UNORM_INT8 }, { CL_LUMINANCE, CL_UNORM_INT16 },
    { CL_LUMINANCE, CL_HALF_FLOAT }, { CL_LUMINANCE, CL_FLOAT },
    { CL_INTENSITY, CL_SNORM_INT8 }, { CL_INTENSITY, CL_SNORM_INT16 },
    { CL_INTENSITY, CL_UNORM_INT8 }, { CL_INTENSITY, CL_UNORM_INT16 },
    { CL_INTENSITY, CL_HALF_FLOAT }, { CL_INTENSITY, CL_FLOAT },
    { CL_RA, CL_SNORM_INT8 }, { CL_RA, CL_SNORM_INT16 },
    { CL_RA, CL_UNORM_INT8 }, { CL_RA, CL_UNORM_INT16 },
    { CL_RA, CL_SIGNED_INT8 }, { CL_RA, CL_SIGNED_INT16 },
    { CL_RA, CL_SIGNED_INT32 }, { CL_RA, CL_UNSIGNED_INT8 },
    { CL_RA, CL_UNSIGNED_INT16 }, { CL_RA, CL_UNSIGNED_INT32 },
    { CL_RA, CL_HALF_FLOAT }, { CL_RA, CL_FLOAT }
  };
  NumImgFmts = llvm::array_lengthof(CPUImgFmts);
  ImgFmts = CPUImgFmts;
  SupportImages = true;
  MaxReadableImages = 128;
  MaxWriteableImages = 8;
  Image2DMaxWidth = 8192;
  Image2DMaxHeight = 8192;
  Image3DMaxWidth = 2048;
  Image3DMaxHeight = 2048;
  Image3DMaxDepth = 2048;
  ImageMaxBufferSize = 65536;
  ImageMaxArraySize = 2048;
  MaxSamplers = 16;

  // TODO: set MaxParameterSize.

  PrintfBufferSize = 65536;

  // MemoryBaseAddressAlignment is the size (in bits) of the largest
  // OpenCL built-in data type supported by the device, that is long16
  // for CPUDevice.
  MemoryBaseAddressAlignment = sizeof(cl_long16) * 8;

  // TODO: set MinimumDataTypeAlignment (Deprecated in OpenCL 1.2).

  SinglePrecisionFPCapabilities = DeviceInfo::FPDenormalization | 
                                  DeviceInfo::FPInfNaN |
                                  DeviceInfo::FPRoundToNearest |
                                  DeviceInfo::FPRoundToZero |
                                  DeviceInfo::FPRoundToInf |
                                  DeviceInfo::FPFusedMultiplyAdd |
                                  DeviceInfo::FPCorrectlyRoundedDivideSqrt;

  DoublePrecisionFPCapabilities = DeviceInfo::FPDenormalization | 
                                  DeviceInfo::FPInfNaN |
                                  DeviceInfo::FPRoundToNearest |
                                  DeviceInfo::FPRoundToZero |
                                  DeviceInfo::FPRoundToInf |
                                  DeviceInfo::FPFusedMultiplyAdd;

  GlobalMemoryCacheType = DeviceInfo::ReadWriteCache;

  GlobalMemoryCachelineSize = LLCache.GetLineSize();
  GlobalMemoryCacheSize = LLCache.GetSize();

  GlobalMemorySize = Machine.GetTotalMemorySize();

  // TODO: set MaxConstantBufferSize.
  // TODO: set MaxConstantArguments.

  LocalMemoryMapping = DeviceInfo::SharedLocal;
  LocalMemorySize = L1Cache.GetSize();
  SupportErrorCorrection = true;

  HostUnifiedMemory = true;

  // TODO: set ProfilingTimerResolution.
  
  // TODO: set LittleEndian, by the compiler?
  // Available is a virtual property.

  CompilerAvailable = true;
  LinkerAvailable = false;

  ExecutionCapabilities = DeviceInfo::CanExecKernel |
                          DeviceInfo::CanExecNativeKernel;

  // TODO: set SizeTypeMax, by the compiler?
  PrivateMemorySize = LocalMemorySize;
}

void CPUDevice::InitSubDeviceInfo() {
  assert(!HWCPUs.empty());

  MaxComputeUnits = HWCPUs.size();
  MaxSubDevices = MaxComputeUnits;
  SupportedPartitionTypes.clear();

  if (MaxComputeUnits == 1) {
    SupportedAffinityDomains = 0;
    return;
  }

  std::set<const void*> Nodes;
  std::vector<std::set<const void*>> Caches(4);

  for (auto *C : HWCPUs) {
    if (auto *NUMANode = C->GetNUMANode())
      Nodes.insert(NUMANode);

    for (unsigned Level = 1; Level <= 4; ++Level)
      if (auto *Cache = C->GetCache(Level))
        Caches[Level - 1].insert(Cache);
  }

  SupportedPartitionTypes.push_back(PartitionEqually);
  SupportedPartitionTypes.push_back(PartitionByCounts);

  if (Nodes.size() > 1)
    SupportedAffinityDomains |= DeviceInfo::AffinityDomainNUMA;

  static const PartitionAffinity Affinities[] = {
    AffinityDomainL1Cache, AffinityDomainL2Cache,
    AffinityDomainL3Cache, AffinityDomainL4Cache
  };

  for (unsigned Level = 0; Level < 4; ++Level)
    if (Caches[Level].size() > 1)
      SupportedAffinityDomains |= Affinities[Level];

  if (!llvm::isPowerOf2_32(SupportedAffinityDomains) &&
      SupportedAffinityDomains)
    SupportedAffinityDomains |= AffinityDomainNext;

  if (SupportedAffinityDomains)
    SupportedPartitionTypes.push_back(PartitionByAffinityDomain);
}

void CPUDevice::InitCompiler() {
  Compiler = llvm::make_unique<CPUCompiler>();

  void *Addr;
  #define INTERNAL_CALL(N, Fmt, F)               \
    Addr = reinterpret_cast<void *>(F);          \
    getCompilerAs<CPUCompiler>().addSymbolMapping("__internal_" #N, Addr);
  #include "InternalCalls.def"
  #undef INTERNAL_CALL

#ifdef CLANG_GT_4
  // For each program-scope sampler variable usage and function-scope sampler
  // variable initialization, Clang generates a call to the following function:
  //
  //    constant opencl.sampler_t *__attribute__((always_inline)) 
  //    __translate_sampler_initializer(unsigned);
  //
  // This function is provided internally by the runtime.
  Addr = reinterpret_cast<void *>(opencrun::cpu::TranslateSamplerInitializer);
  getCompilerAs<CPUCompiler>().addSymbolMapping("__translate_sampler_initializer", Addr);
#endif
}

CPUMemoryDescriptor &CPUDevice::getMemoryDescriptor(const MemoryObject &Obj) {
  return static_cast<CPUMemoryDescriptor&>(Obj.getDescriptorFor(*this));
}

CPUImageDescriptor &CPUDevice::getMemoryDescriptor(const Image &Img) {
  return static_cast<CPUImageDescriptor&>(Img.getDescriptorFor(*this));
}

CPUThread &CPUDevice::pickThread() {
  // FIXME: maybe this is a too much dumb policy.
  return *Threads.front();
}

CPUThread &CPUDevice::pickLeastLoadedThread() {
  float MinLoad = std::numeric_limits<float>::max();

  CPUThread *Thr = nullptr;

  for (const auto &T : Threads) {
    float CurLoad = T->GetLoadIndicator();
    if (CurLoad < MinLoad) {
      Thr = T.get();
      MinLoad = CurLoad;
    }
  }

  return *Thr;
}

bool CPUDevice::Submit(EnqueueReadBuffer &Cmd) {
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  return pickThread().submit<ReadBufferCPUCommand>(Cmd, SrcPtr);
}

bool CPUDevice::Submit(EnqueueWriteBuffer &Cmd) {
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return pickThread().submit<WriteBufferCPUCommand>(Cmd, TgtPtr);
}

bool CPUDevice::Submit(EnqueueCopyBuffer &Cmd) {
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return pickThread().submit<CopyBufferCPUCommand>(Cmd, TgtPtr, SrcPtr);
}

bool CPUDevice::Submit(EnqueueReadImage &Cmd) {
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  return pickThread().submit<ReadImageCPUCommand>(Cmd, SrcPtr);
}

bool CPUDevice::Submit(EnqueueWriteImage &Cmd) {
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return pickThread().submit<WriteImageCPUCommand>(Cmd, TgtPtr);
}

bool CPUDevice::Submit(EnqueueCopyImage &Cmd) {
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return pickThread().submit<CopyImageCPUCommand>(Cmd, TgtPtr, SrcPtr);
}

bool CPUDevice::Submit(EnqueueCopyImageToBuffer &Cmd) {
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return pickThread().submit<CopyImageToBufferCPUCommand>(Cmd, TgtPtr, SrcPtr);
}

bool CPUDevice::Submit(EnqueueCopyBufferToImage &Cmd) {
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return pickThread().submit<CopyBufferToImageCPUCommand>(Cmd, TgtPtr, SrcPtr);
}

bool CPUDevice::Submit(EnqueueMapBuffer &Cmd) {
  // FIXME: Map operations are no-op!
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  return pickThread().submit<MapBufferCPUCommand>(Cmd, SrcPtr);
}

bool CPUDevice::Submit(EnqueueMapImage &Cmd) {
  // FIXME: Map operations are no-op!
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  return pickThread().submit<MapImageCPUCommand>(Cmd, SrcPtr);
}

bool CPUDevice::Submit(EnqueueUnmapMemObject &Cmd) {
  // FIXME: Unmap operations are no-op!
  void *DataPtr = getMemoryDescriptor(Cmd.GetMemObj()).ptr();
  return pickThread().submit<UnmapMemObjectCPUCommand>(Cmd, DataPtr, Cmd.GetMappedPtr());
}

bool CPUDevice::Submit(EnqueueReadBufferRect &Cmd) {
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  return pickThread().submit<ReadBufferRectCPUCommand>(Cmd, Cmd.GetTarget(), SrcPtr);
}

bool CPUDevice::Submit(EnqueueWriteBufferRect &Cmd) {
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return pickThread().submit<WriteBufferRectCPUCommand>(Cmd, TgtPtr, Cmd.GetSource());
}

bool CPUDevice::Submit(EnqueueCopyBufferRect &Cmd) {
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return pickThread().submit<CopyBufferRectCPUCommand>(Cmd, TgtPtr, SrcPtr);
}

bool CPUDevice::Submit(EnqueueFillBuffer &Cmd) {
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return pickThread().submit<FillBufferCPUCommand>(Cmd, TgtPtr);
}

bool CPUDevice::Submit(EnqueueFillImage &Cmd) {
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return pickThread().submit<FillImageCPUCommand>(Cmd, TgtPtr);
}

bool CPUDevice::Submit(EnqueueNDRangeKernel &Cmd) {
  // TODO: analyze kernel and decide the scheduling policy to use.
  return BlockParallelSubmit(Cmd);
}

bool CPUDevice::Submit(EnqueueNativeKernel &Cmd) {
  auto Base = reinterpret_cast<uintptr_t>(Cmd.GetArgumentsPointer());

  // Patching arguments buffer copy.
  for (const auto &P : Cmd.GetMemoryLocations()) {
    auto Addr = reinterpret_cast<void**>(Base + P.first);
    *Addr = getMemoryDescriptor(*P.second).ptr();
  }

  return pickThread().submit<NativeKernelCPUCommand>(Cmd);
}

bool CPUDevice::Submit(EnqueueMarker &Cmd) {
  return pickThread().submit<NoOpCPUCommand>(Cmd);
}

bool CPUDevice::Submit(EnqueueBarrier &Cmd) {
  return pickThread().submit<NoOpCPUCommand>(Cmd);
}

CPUKernelArguments CPUDevice::createKernelArguments(Kernel &Kern, size_t ALS) {
  auto Args = std::vector<void*>{Kern.GetArgCount()};
  auto LocalBuffers = std::vector<std::pair<size_t, size_t>>{};

  auto Samplers = std::vector<cpu_sampler_t>{};
  auto NumSamplers =
    std::count_if(Kern.arg_begin(), Kern.arg_end(), [](const KernelArg &A) {
      return A.getKind() == KernelArg::SamplerArg;
    });
  Samplers.resize(NumSamplers);

  auto *CurSampler = &Samplers.front();
  for (auto I = Kern.arg_begin(), E = Kern.arg_end(); I != E; ++I) {
    void *Ptr = nullptr;
    switch (I->getKind()) {
    default: break;
    case KernelArg::LocalBufferArg:
      Ptr = nullptr;
      LocalBuffers.emplace_back(I->getIndex(), I->getLocalSize());
      break;
    case KernelArg::BufferArg:
      if (auto *Buf = I->getBuffer())
        Ptr = getMemoryDescriptor(*Buf).ptr();
      break;
    case KernelArg::ImageArg:
      if (auto *Img = I->getImage())
        Ptr = getMemoryDescriptor(*Img).imageHeader();
      break;
    case KernelArg::SamplerArg:
      *CurSampler = getCPUSampler(I->getSampler());
      Ptr = CurSampler++;
      break;
    case KernelArg::ByValueArg:
      Ptr = I->getByValPtr();
      break;
    }
    Args[I->getIndex()] = Ptr;
  }

  return {std::move(Args), ALS, std::move(LocalBuffers), std::move(Samplers)};
}

bool CPUDevice::BlockParallelSubmit(EnqueueNDRangeKernel &Cmd) {
  auto &Kern = Cmd.GetKernel();
  auto &KernDesc = Kern.getDescriptor();
  auto *KernFn = KernDesc.getFunction(this);
  auto &Cmplr = getCompilerAs<CPUCompiler>();

  Cmplr.addKernel(KernFn);
  auto Entry = Cmplr.getEntryPoint(KernFn);
  auto AutoLocalsSize = Cmplr.getAutomaticLocalsSize(KernFn);
  auto KernArgs = createKernelArguments(Kern, AutoLocalsSize);

  llvm::IntrusiveRefCntPtr<NDRangeKernelBlockContext> CmdContext;
  CmdContext = new NDRangeKernelBlockContext(Cmd.GetWorkGroupsCount(),
                                             std::move(KernArgs));

  DimensionInfo &DimInfo = Cmd.GetDimensionInfo();
  size_t WGSize = DimInfo.GetLocalWorkItems();

  for (auto I = DimInfo.begin(), E = DimInfo.end(); I != E; I += WGSize) {
    auto &Thr = pickLeastLoadedThread();

    if (!Thr.submit<NDRangeKernelBlockCPUCommand>(Cmd, Entry, I, I + WGSize,
                                                  CmdContext))
      return false;
  }

  return true;
}

const Footprint &
CPUDevice::getKernelFootprint(const KernelDescriptor &Kern) const {
  llvm::sys::ScopedLock Lock(ThisLock);

  FootprintsContainer::iterator I = KernelFootprints.find(&Kern);

  if (I != KernelFootprints.end())
    return I->second;

  FootprintEstimate *Pass = CreateFootprintEstimatePass(GetName());
  llvm::Function *Fun = Kern.getFunction(this);

  llvm::legacy::PassManager PM;
  PM.add(Pass);
  PM.run(*Fun->getParent());

  KernelFootprints[&Kern] = *Pass;

  return KernelFootprints[&Kern];
}

bool CPUDevice::isPartitionSupported(const DevicePartition &Part) const {
  // Check for partitioning type support.
  if (!std::count(SupportedPartitionTypes.begin(),
                  SupportedPartitionTypes.end(), Part.getType()))
    return false;

  switch (Part.getType()) {
  case PartitionByAffinityDomain:
    if ((SupportedAffinityDomains & Part.getAffinityDomain()) == 0)
      return false;
    return true;
  case PartitionByCounts:
    if (Part.getNumCounts() > GetMaxSubDevices() ||
        Part.getTotalComputeUnits() > GetMaxComputeUnits())
      return false;
    return true;
  case PartitionEqually:
    if (Part.getNumComputeUnits() > GetMaxComputeUnits())
      return false;
    return true;
  default: llvm_unreachable(0);
  }
}

static bool computeAffinityDomain(unsigned SupportedDomains,
                                  DeviceInfo::PartitionAffinity &Affinity) {
  if (Affinity != DeviceInfo::AffinityDomainNext)
    return Affinity & SupportedDomains;

  static const DeviceInfo::PartitionAffinity Domains[] = {
    DeviceInfo::AffinityDomainNUMA,
    DeviceInfo::AffinityDomainL4Cache,
    DeviceInfo::AffinityDomainL3Cache,
    DeviceInfo::AffinityDomainL2Cache,
    DeviceInfo::AffinityDomainL1Cache
  };

  for (unsigned i = 0; i != llvm::array_lengthof(Domains); ++i)
    if (Domains[i] & SupportedDomains) {
      Affinity = Domains[i];
      return true;
    }

  return false;
}

static unsigned getCacheLevel(DeviceInfo::PartitionAffinity Affinity) {
  switch (Affinity) {
  case DeviceInfo::AffinityDomainL1Cache: return 1;
  case DeviceInfo::AffinityDomainL2Cache: return 2;
  case DeviceInfo::AffinityDomainL3Cache: return 3;
  case DeviceInfo::AffinityDomainL4Cache: return 4;
  default: llvm_unreachable(0);
  }
}

bool CPUDevice::createSubDevices(const DevicePartition &Part,
                         llvm::SmallVectorImpl<std::unique_ptr<Device>> &Devs) {
  if (!isPartitionSupported(Part))
    return false;

  using HardwareCPUsContainer = llvm::SmallVector<const sys::HardwareCPU*, 8>;

  switch (Part.getType()) {
  case PartitionByAffinityDomain: {
    auto Affinity = Part.getAffinityDomain();
    if (!computeAffinityDomain(SupportedAffinityDomains, Affinity))
      return false;

    std::map<sys::HardwareComponent*, HardwareCPUsContainer> Partitions;
    if (Affinity == AffinityDomainNUMA) {
      for (auto C : HWCPUs)
        if (auto *NUMANode = C->GetNUMANode())
          Partitions[NUMANode].push_back(C);
    } else {
      unsigned Level = getCacheLevel(Affinity);
      for (auto C : HWCPUs)
        if (auto *Cache = C->GetCache(Level))
          Partitions[Cache].push_back(C);
    }

    for (auto &P : Partitions) {
      std::unique_ptr<Device> D(new CPUDevice(*this, Part, P.second));
      Devs.push_back(std::move(D));
    }

    return true;
  }
  case PartitionEqually: {
    unsigned N = Part.getNumComputeUnits();
    unsigned Groups = HWCPUs.size() / N;

    assert(Groups > 0);

    auto NextCPU = HWCPUs.begin();
    for (unsigned i = 0; i != Groups; ++i) {
      HardwareCPUsContainer SubDevCPUs;
      for (unsigned j = 0; j != N; ++j)
        SubDevCPUs.push_back(*NextCPU++);
      std::unique_ptr<Device> D(new CPUDevice(*this, Part, SubDevCPUs));
      Devs.push_back(std::move(D));
    }

    return true;
  }
  case PartitionByCounts: {
    auto NextCPU = HWCPUs.begin();
    for (unsigned i = 0, e = Part.getNumCounts(); i != e; ++i) {
      HardwareCPUsContainer SubDevCPUs;
      for (unsigned j = 0; j != Part.getCount(i); ++j)
        SubDevCPUs.push_back(*NextCPU++);
      std::unique_ptr<Device> D(new CPUDevice(*this, Part, SubDevCPUs));
      Devs.push_back(std::move(D));
    }

    return true;
  }
  default: llvm_unreachable(0);
  }
}

void opencrun::initializeCPUDevice(Platform &P) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  sys::Hardware &HW = sys::GetHardware();

  // A device for each Machine in the System (it may be a cluster system).
  for (sys::Hardware::machine_iterator I = HW.machine_begin(),
       E = HW.machine_end(); I != E; ++I)
    P.addDevice(new CPUDevice(*I));
}
