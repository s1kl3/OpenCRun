
#include "CPUDevice.h"
#include "CPUCompiler.h"
#include "CPUKernelInfo.h"
#include "CPUPasses.h"
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

using namespace opencrun;
using namespace opencrun::cpu;

//
// CPUMemoryDescriptor implementation.
//

CPUMemoryDescriptor::~CPUMemoryDescriptor() {
  // If UseHostPtr is set, we do not own the storage.
  if (getMemoryObject().getFlags() & MemoryObject::UseHostPtr)
    return;

  sys::Free(Ptr);
}

bool CPUMemoryDescriptor::allocate() {
  assert(!isAllocated());

  // If UseHostPtr is set, do not duplicate the storage.
  if (getMemoryObject().getFlags() & MemoryObject::UseHostPtr) {
    Ptr = getMemoryObject().getHostPtr();
    return Allocated = true;
  }

  Ptr = sys::CacheAlignedAlloc(getMemoryObject().getSize());
  return Allocated = Ptr != nullptr;
}

bool CPUMemoryDescriptor::aliasWithHostPtr() const {
  return isAllocated() &&
         getMemoryObject().getFlags() & MemoryObject::UseHostPtr;
}

void *CPUMemoryDescriptor::map() {
  tryAllocate();

  return Ptr;
}

void CPUMemoryDescriptor::unmap() {
}

//
// CPUDevice implementation.
//

CPUDevice::CPUDevice(const sys::HardwareMachine &Machine) :
  Device(CPUType, "CPU"), Machine(Machine) {
  HardwareCPUsContainer CPUs;
  for(auto I = Machine.cpu_begin(), E = Machine.cpu_end(); I != E; ++I)
    CPUs.insert(&*I);

  InitDeviceInfo();
  InitSubDeviceInfo(CPUs);

  InitMultiprocessors();
  InitCompiler();
}

CPUDevice::CPUDevice(CPUDevice &Parent, const DevicePartition &Part,
                     const HardwareCPUsContainer &CPUs) :
  Device(Parent, Part), Machine(Parent.GetHardwareMachine()) {
  InitSubDeviceInfo(CPUs);
  InitMultiprocessors(CPUs);
  InitCompiler();
}

CPUDevice::~CPUDevice() {
  DestroyMultiprocessors();
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
  return llvm::make_unique<CPUMemoryDescriptor>(*this, Obj);
}

bool CPUDevice::Submit(Command &Cmd) {
  bool Submitted = false;
  std::unique_ptr<ProfileSample> Sample;

  // Take the profiling information here, in order to force this sample
  // happening before the subsequents samples.
  unsigned Counters = Cmd.IsProfiled() ? Profiler::Time : Profiler::None;
  Sample.reset(GetProfilerSample(Counters, ProfileSample::CommandSubmitted));

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
    Cmd.GetNotifyEvent().MarkSubmitted(Sample.release());

  return Submitted;
}

void CPUDevice::UnregisterKernel(const KernelDescriptor &Kern) {
  llvm::sys::ScopedLock Lock(ThisLock);

  // Erase kernel from the cache.
  BlockParallelEntriesCache.erase(&Kern);
  BlockParallelStaticLocalsCache.erase(&Kern);
  BlockParallelStaticLocalVectorsCache.erase(&Kern);
  KernelFootprints.erase(&Kern);

  getCompilerAs<CPUCompiler>().removeKernel(Kern.getFunction(this));
}

void CPUDevice::NotifyDone(CPUExecCommand *Cmd, int ExitStatus) {
  // Get counters to profile.
  Command &QueueCmd = Cmd->GetQueueCommand();
  InternalEvent &Ev = QueueCmd.GetNotifyEvent();
  CommandQueue &Queue = Ev.GetCommandQueue();
  unsigned Counters = Cmd->IsProfiled() ? Profiler::Time : Profiler::None;

  // This command does not directly translate to an OpenCL command. Register
  // partial acknowledgment.
  if(CPUMultiExecCommand *MultiCmd = llvm::dyn_cast<CPUMultiExecCommand>(Cmd)) {
    ProfileSample *Sample = GetProfilerSample(Counters,
                                              ProfileSample::CommandCompleted,
                                              MultiCmd->GetId());
    Ev.MarkSubCompleted(Sample);
  }

  // All acknowledgment received.
  if(Cmd->RegisterCompleted(ExitStatus)) {
    ProfileSample *Sample = GetProfilerSample(Counters,
                                              ProfileSample::CommandCompleted);
    Ev.MarkCompleted(Cmd->GetExitStatus(), Sample);
    Queue.CommandDone(QueueCmd);
  }

  delete Cmd;
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

void CPUDevice::InitSubDeviceInfo(const HardwareCPUsContainer &CPUs) {
  MaxComputeUnits = CPUs.size();
  MaxSubDevices = MaxComputeUnits;

  computeSubDeviceInfo(CPUs);
}

void CPUDevice::computeSubDeviceInfo(const HardwareCPUsContainer &CPUs) {
  assert(!CPUs.empty());

  MaxComputeUnits = CPUs.size();
  MaxSubDevices = MaxComputeUnits;
  SupportedPartitionTypes.clear();

  if (MaxComputeUnits == 1) {
    SupportedAffinityDomains = 0;
    return;
  }

  std::set<const void*> Nodes;
  std::vector<std::set<const void*>> Caches(4);

  for (auto *C : CPUs) {
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
}

void CPUDevice::InitMultiprocessors() {
  for (auto I = Machine.socket_begin(), E = Machine.socket_end(); I != E; ++I)
    Multiprocessors.insert(new Multiprocessor(*this, *I));
}

void CPUDevice::InitMultiprocessors(const HardwareCPUsContainer &CPUs) {
  Multiprocessors.insert(new Multiprocessor(*this, CPUs));
}

void CPUDevice::DestroyMultiprocessors() {
  llvm::DeleteContainerPointers(Multiprocessors);
}

void CPUDevice::GetPinnedCPUs(HardwareCPUsContainer &CPUs) const {
  for(MultiprocessorsContainer::iterator I = Multiprocessors.begin(),
                                         E = Multiprocessors.end();
                                         I != E;
                                         ++I)
    (*I)->GetPinnedCPUs(CPUs);
}

MemoryDescriptor &CPUDevice::getMemoryDescriptor(const MemoryObject &Obj) {
  return Obj.getDescriptorFor(*this);
}

bool CPUDevice::Submit(EnqueueReadBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  return MP.Submit(new ReadBufferCPUCommand(Cmd, SrcPtr));
}

bool CPUDevice::Submit(EnqueueWriteBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return MP.Submit(new WriteBufferCPUCommand(Cmd, TgtPtr));
}

bool CPUDevice::Submit(EnqueueCopyBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return MP.Submit(new CopyBufferCPUCommand(Cmd, TgtPtr, SrcPtr));
}

bool CPUDevice::Submit(EnqueueReadImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  return MP.Submit(new ReadImageCPUCommand(Cmd, SrcPtr));
}

bool CPUDevice::Submit(EnqueueWriteImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return MP.Submit(new WriteImageCPUCommand(Cmd, TgtPtr));
}

bool CPUDevice::Submit(EnqueueCopyImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return MP.Submit(new CopyImageCPUCommand(Cmd, TgtPtr, SrcPtr));
}

bool CPUDevice::Submit(EnqueueCopyImageToBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return MP.Submit(new CopyImageToBufferCPUCommand(Cmd, TgtPtr, SrcPtr));
}

bool CPUDevice::Submit(EnqueueCopyBufferToImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return MP.Submit(new CopyBufferToImageCPUCommand(Cmd, TgtPtr, SrcPtr));
}

bool CPUDevice::Submit(EnqueueMapBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  // FIXME: Map operations are no-op!
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  return MP.Submit(new MapBufferCPUCommand(Cmd, SrcPtr));
}

bool CPUDevice::Submit(EnqueueMapImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  // FIXME: Map operations are no-op!
  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  return MP.Submit(new MapImageCPUCommand(Cmd, SrcPtr));
}

bool CPUDevice::Submit(EnqueueUnmapMemObject &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  // FIXME: Unmap operations are no-op!
  void *DataPtr = getMemoryDescriptor(Cmd.GetMemObj()).ptr();
  return MP.Submit(new UnmapMemObjectCPUCommand(Cmd, DataPtr, Cmd.GetMappedPtr()));
}

bool CPUDevice::Submit(EnqueueReadBufferRect &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  return MP.Submit(new ReadBufferRectCPUCommand(Cmd, Cmd.GetTarget(), SrcPtr));
}

bool CPUDevice::Submit(EnqueueWriteBufferRect &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return MP.Submit(new WriteBufferRectCPUCommand(Cmd, TgtPtr, Cmd.GetSource()));
}

bool CPUDevice::Submit(EnqueueCopyBufferRect &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *SrcPtr = getMemoryDescriptor(Cmd.GetSource()).ptr();
  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return MP.Submit(new CopyBufferRectCPUCommand(Cmd, TgtPtr, SrcPtr));
}

bool CPUDevice::Submit(EnqueueFillBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return MP.Submit(new FillBufferCPUCommand(Cmd, TgtPtr));
}

bool CPUDevice::Submit(EnqueueFillImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  void *TgtPtr = getMemoryDescriptor(Cmd.GetTarget()).ptr();
  return MP.Submit(new FillImageCPUCommand(Cmd, TgtPtr));
}

bool CPUDevice::Submit(EnqueueNDRangeKernel &Cmd) {
  // Get global and constant buffer mappings.
  GlobalArgMappingsContainer GlobalArgs;
  LocateMemoryObjArgAddresses(Cmd.GetKernel(), GlobalArgs);

  // TODO: analyze kernel and decide the scheduling policy to use.
  return BlockParallelSubmit(Cmd, GlobalArgs);
}

bool CPUDevice::Submit(EnqueueNativeKernel &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  auto Base = reinterpret_cast<uintptr_t>(Cmd.GetArgumentsPointer());

  // Patching arguments buffer copy.
  for (const auto &P : Cmd.GetMemoryLocations()) {
    auto Addr = reinterpret_cast<void**>(Base + P.first);
    *Addr = getMemoryDescriptor(*P.second).ptr();
  }

  return MP.Submit(new NativeKernelCPUCommand(Cmd));
}

bool CPUDevice::Submit(EnqueueMarker &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();
  return MP.Submit(new NoOpCPUCommand(Cmd));
}

bool CPUDevice::Submit(EnqueueBarrier &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();
  return MP.Submit(new NoOpCPUCommand(Cmd));
}

bool CPUDevice::BlockParallelSubmit(EnqueueNDRangeKernel &Cmd,
                                    GlobalArgMappingsContainer &GlobalArgs) {
  const KernelDescriptor &KernDesc = Cmd.GetKernel().getDescriptor();
  // Native launcher address.
  BlockParallelEntryPoint Entry = GetBlockParallelEntryPoint(KernDesc);

  // Index space.
  DimensionInfo &DimInfo = Cmd.GetDimensionInfo();

  // Static local size
  unsigned StaticLocalSize = GetBlockParallelStaticLocalSize(KernDesc);

  // Collect <Index, Offset> pairs for each kernel local storage.
  BlockParallelStaticLocalVector StaticLocalInfos;
  GetBlockParallelStaticLocalVector(KernDesc, StaticLocalInfos);

  // Decide the work group size.
  if(!Cmd.IsLocalWorkGroupSizeSpecified()) {
    llvm::SmallVector<size_t, 4> Sizes;

    for(unsigned I = 0; I < DimInfo.GetDimensions(); ++I)
      Sizes.push_back(DimInfo.GetGlobalWorkItems(I));

    DimInfo.SetLocalWorkItems(Sizes);
  }

  // Holds data about kernel result.
  llvm::IntrusiveRefCntPtr<CPUCommand::ResultRecorder> Result;
  Result = new CPUCommand::ResultRecorder(Cmd.GetWorkGroupsCount());

  MultiprocessorsContainer::iterator S = Multiprocessors.begin(),
                                     F = Multiprocessors.end(),
                                     J = S;
  size_t WorkGroupSize = DimInfo.GetLocalWorkItems();
  bool AllSent = true;

  // Perfect load balancing.
  for(DimensionInfo::iterator I = DimInfo.begin(),
                              E = DimInfo.end();
                              I != E;
                              I += WorkGroupSize) {
    NDRangeKernelBlockCPUCommand *NDRangeCmd;
    NDRangeCmd = new NDRangeKernelBlockCPUCommand(Cmd,
                                                  Entry,
                                                  GlobalArgs,
                                                  I,
                                                  I + WorkGroupSize,
                                                  StaticLocalSize,
                                                  StaticLocalInfos,
                                                  *Result);

    // Submit command.
    AllSent = AllSent && (*J)->Submit(NDRangeCmd);

    // Reset counter, round robin.
    if(++J == F)
      J = S;
  }

  return AllSent;
}

CPUDevice::BlockParallelEntryPoint
CPUDevice::GetBlockParallelEntryPoint(const KernelDescriptor &KernDesc) {
  llvm::sys::ScopedLock Lock(ThisLock);

  // Cache hit.
  BlockParallelEntryPoints::iterator I =
    BlockParallelEntriesCache.find(&KernDesc);

  if (I != BlockParallelEntriesCache.end())
    return I->second;

  // Cache miss.
  auto *KernFn = KernDesc.getFunction(this);
  void *EntryPtr = getCompilerAs<CPUCompiler>().addKernel(KernFn);

  return BlockParallelEntriesCache[&KernDesc] =
            reinterpret_cast<CPUDevice::BlockParallelEntryPoint>(EntryPtr);
}

unsigned CPUDevice::GetBlockParallelStaticLocalSize(const KernelDescriptor &KernDesc) {
  llvm::sys::ScopedLock Lock(ThisLock);

  // Cache hit.
  BlockParallelStaticLocalSizes::iterator I =
    BlockParallelStaticLocalsCache.find(&KernDesc);
  if (I != BlockParallelStaticLocalsCache.end())
    return I->second;

  // Cache miss.
  CPUKernelInfo Info(KernDesc.getKernelInfo(this));

  unsigned StaticLocalSize = Info.getStaticLocalSize();
  BlockParallelStaticLocalsCache[&KernDesc] = StaticLocalSize;

  return StaticLocalSize;
}

void
CPUDevice::GetBlockParallelStaticLocalVector(const KernelDescriptor &KernDesc,
                                             BlockParallelStaticLocalVector &SLVec) {
  llvm::sys::ScopedLock Lock(ThisLock);

  // Cache hit.
  BlockParallelStaticLocalVectors::iterator I =
    BlockParallelStaticLocalVectorsCache.find(&KernDesc);
  if (I != BlockParallelStaticLocalVectorsCache.end())
    SLVec = I->second;
  else {
    // Cache miss.
    CPUKernelInfo Info(KernDesc.getKernelInfo(this));
    CPUKernelInfo Info(KernDesc.getKernelInfo());

    for (unsigned i = 0; i < Info.getNumStaticLocalAreas(); ++i) {
      SLVec.push_back(std::make_pair(Info.getStaticLocalIndex(i),
                                     Info.getStaticLocalOffset(i)));
    }

    BlockParallelStaticLocalVectorsCache[&KernDesc] = SLVec;
  }
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

  // Local memory usage updated thanks to "static_local_infos" additional
  // metadata from the AutomaticLocalVariables pass.
  if (Fun->arg_size() > 1) {
    const llvm::Argument &Arg = *(--Fun->arg_end());
    std::string ArgName = Fun->getName().str() + ".locals";
    if (Arg.getName() == ArgName) {
      CPUKernelInfo Info(Kern.getKernelInfo());
      Pass->AddLocalMemoryUsage(Info.getStaticLocalSize());
    }
  }

  // Local memory allocated for kernel __local pointer arguments by
  // creating VirtualBuffer objects.
  Pass->AddLocalMemoryUsage(Kern.getLocalArgsSize());
  
  KernelFootprints[&Kern] = *Pass;

  return KernelFootprints[&Kern];
}

void CPUDevice::LocateMemoryObjArgAddresses(
                  Kernel &Kern,
                  GlobalArgMappingsContainer &GlobalArgs) {
  for(auto I = Kern.arg_begin(), E = Kern.arg_end(); I != E; ++I) {
    void *Ptr = nullptr;
    switch (I->getKind()) {
    default: break;
    case KernelArg::BufferArg:
      if (auto *Buf = I->getBuffer())
        Ptr = getMemoryDescriptor(*Buf).ptr();
      break;
    case KernelArg::ImageArg:
      if (auto *Img = I->getImage())
        Ptr = getMemoryDescriptor(*Img).ptr();
      break;
    }
    GlobalArgs[I->getIndex()] = Ptr;
  }
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

  HardwareCPUsContainer CPUs;
  GetPinnedCPUs(CPUs);

  switch (Part.getType()) {
  case PartitionByAffinityDomain: {
    auto Affinity = Part.getAffinityDomain();
    if (!computeAffinityDomain(SupportedAffinityDomains, Affinity))
      return false;

    std::map<sys::HardwareComponent*, HardwareCPUsContainer> Partitions;
    if (Affinity == AffinityDomainNUMA) {
      for (auto C : CPUs)
        if (auto *NUMANode = C->GetNUMANode())
          Partitions[NUMANode].insert(C);
    } else {
      unsigned Level = getCacheLevel(Affinity);
      for (auto C : CPUs)
        if (auto *Cache = C->GetCache(Level))
          Partitions[Cache].insert(C);
    }

    for (auto &P : Partitions) {
      std::unique_ptr<Device> D(new CPUDevice(*this, Part, P.second));
      Devs.push_back(std::move(D));
    }

    return true;
  }
  case PartitionEqually: {
    unsigned N = Part.getNumComputeUnits();
    unsigned Groups = CPUs.size() / N;

    assert(Groups > 0);

    auto NextCPU = CPUs.begin();
    for (unsigned i = 0; i != Groups; ++i) {
      HardwareCPUsContainer SubDevCPUs;
      for (unsigned j = 0; j != N; ++j)
        SubDevCPUs.insert(*NextCPU++);
      std::unique_ptr<Device> D(new CPUDevice(*this, Part, SubDevCPUs));
      Devs.push_back(std::move(D));
    }

    return true;
  }
  case PartitionByCounts: {
    auto NextCPU = CPUs.begin();
    for (unsigned i = 0, e = Part.getNumCounts(); i != e; ++i) {
      HardwareCPUsContainer SubDevCPUs;
      for (unsigned j = 0; j != Part.getCount(i); ++j)
        SubDevCPUs.insert(*NextCPU++);
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
