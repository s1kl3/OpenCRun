
#include "opencrun/Device/CPU/CPUDevice.h"
#include "opencrun/Core/CommandQueue.h"
#include "opencrun/Core/Event.h"
#include "opencrun/Core/Platform.h"
#include "opencrun/Device/CPU/InternalCalls.h"
#include "opencrun/Device/CPUPasses/AllPasses.h"
#include "opencrun/Device/Devices.h"
#include "opencrun/Passes/AggressiveInliner.h"
#include "opencrun/Passes/AllPasses.h"
#include "opencrun/Util/BuiltinInfo.h"

#include "CPUKernelInfo.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Linker/Linker.h"
#include "llvm/PassManager.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ThreadLocal.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace opencrun;

namespace {

void SignalJITCallStart(CPUDevice &Dev);
void SignalJITCallEnd();

} // End anonymous namespace.

//
// CPUDevice implementation.
//

CPUDevice::CPUDevice(const sys::HardwareMachine &Machine) :
  Device(CPUType, "CPU", llvm::sys::getDefaultTargetTriple()),
  Machine(Machine),
  Global(Machine.GetTotalMemorySize()) {
  HardwareCPUsContainer CPUs;
  for(auto I = Machine.cpu_begin(), E = Machine.cpu_end(); I != E; ++I)
    CPUs.insert(&*I);

  InitDeviceInfo();
  InitSubDeviceInfo(CPUs);

  InitMultiprocessors();
  InitJIT();
}

CPUDevice::CPUDevice(CPUDevice &Parent, const DevicePartition &Part,
                     const HardwareCPUsContainer &CPUs) :
  Device(Parent, Part),
  Machine(Parent.GetHardwareMachine()),
  Global(Parent.GetHardwareMachine().GetTotalMemorySize()) {
  InitSubDeviceInfo(CPUs);
  InitMultiprocessors(CPUs);
  InitJIT();
}

CPUDevice::~CPUDevice() {
  DestroyMultiprocessors();
  DestroyJIT();
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

bool CPUDevice::CreateHostBuffer(HostBuffer &Buf) {
  return Global.Alloc(Buf);
}

bool CPUDevice::CreateHostAccessibleBuffer(HostAccessibleBuffer &Buf) {
  return Global.Alloc(Buf);
}

bool CPUDevice::CreateDeviceBuffer(DeviceBuffer &Buf) {
  return Global.Alloc(Buf);
}

bool CPUDevice::CreateHostImage(HostImage &Img) {
  return Global.Alloc(Img);
}

bool CPUDevice::CreateHostAccessibleImage(HostAccessibleImage &Img) {
  return Global.Alloc(Img);
}

bool CPUDevice::CreateDeviceImage(DeviceImage &Img) {
  return Global.Alloc(Img);
}

void CPUDevice::DestroyMemoryObj(MemoryObj &MemObj) {
  Global.Free(MemObj);
}

bool CPUDevice::MappingDoesAllocation(MemoryObj::Type MemObjTy) {
  // Memory objects are directly accessible by the host, so mapping
  // nevere requires memory allocation on the host side.
  return false;
}

void *CPUDevice::CreateMapBuffer(MemoryObj &MemObj, 
                                 MemoryObj::MappingInfo &MapInfo) {
  void *MapBuf;
 
  // A CPU device has only one physical address space so host-side
  // code can access directly to memory object's storage area.
  if(llvm::isa<Buffer>(MemObj)) {
    MapBuf = reinterpret_cast<void *>(
        reinterpret_cast<uintptr_t>(Global[MemObj]) + MapInfo.Offset);
  } else if(Image *Img = llvm::dyn_cast<Image>(&MemObj)) {
    MapBuf = reinterpret_cast<void *>(
        reinterpret_cast<uintptr_t>(Global[MemObj]) +
        Img->GetElementSize() * MapInfo.Origin[0] +
        Img->GetRowPitch() * MapInfo.Origin[1] +
        Img->GetSlicePitch() * MapInfo.Origin[2]);
  }

  if(!MemObj.AddNewMapping(MapBuf, MapInfo))
    MapBuf = NULL;

  return MapBuf;
}

void CPUDevice::FreeMapBuffer(void *MapBuf) {
  // This method does nothing in particular for CPU target, but for
  // those target having separated physical address spaces it would be
  // used inside clEnqueueMapBuffer and clEnqueueMapImage to free
  // host allocated memory in case of errors before returning to the caller.
}

bool CPUDevice::Submit(Command &Cmd) {
  bool Submitted = false;
  std::unique_ptr<ProfileSample> Sample;

  // Take the profiling information here, in order to force this sample
  // happening before the subsequents samples.
  unsigned Counters = Cmd.IsProfiled() ? Profiler::Time : Profiler::None;
  Sample.reset(GetProfilerSample(*this, Counters,
                                 ProfileSample::CommandSubmitted));

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
  // TODO: modules must be ref-counted -- unregister a kernel does not
  // necessary enforce module unloading?
  llvm::sys::ScopedLock Lock(ThisLock);

  // Remove module from the JIT.
  llvm::Module &Mod = *Kern.getFunction(this)->getParent();
  JIT->removeModule(&Mod);

  // Erase kernel from the cache.
  BlockParallelEntriesCache.erase(&Kern);
  BlockParallelStaticLocalsCache.erase(&Kern);
  BlockParallelStaticLocalVectorsCache.erase(&Kern);
  KernelFootprints.erase(&Kern);

  // Invoke static destructors.
  JIT->runStaticConstructorsDestructors(&Mod, true);
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
    ProfileSample *Sample = GetProfilerSample(*this,
                                              Counters,
                                              ProfileSample::CommandCompleted,
                                              MultiCmd->GetId());
    Ev.MarkSubCompleted(Sample);
  }

  // All acknowledgment received.
  if(Cmd->RegisterCompleted(ExitStatus)) {
    ProfileSample *Sample = GetProfilerSample(*this,
                                              Counters,
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

  CompilerAvailable = false;
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

void CPUDevice::InitJIT() {
  // Init the native target.
  llvm::InitializeNativeTarget();

  // Create the JIT.
  llvm::EngineBuilder Bld(&*BitCodeLibrary);
  llvm::ExecutionEngine *Engine = Bld.setEngineKind(llvm::EngineKind::JIT)
                                     .setOptLevel(llvm::CodeGenOpt::Default)
                                     .create();

  // Configure the JIT.
  Engine->InstallLazyFunctionCreator(LibLinker);

  intptr_t AddrInt;
  void *Addr;

  llvm::Function *Func;

  #define INTERNAL_CALL(N, Fmt, F)                                    \
    AddrInt = reinterpret_cast<intptr_t>(F);                          \
    Addr = reinterpret_cast<void *>(AddrInt);                         \
    Func = DeviceBuiltinInfo::getPrototype(*BitCodeLibrary, "__internal_" #N, Fmt); \
    Engine->addGlobalMapping(Func, Addr);
  #include "InternalCalls.def"
  #undef INTERNAL_CALL

  // Save pointer.
  JIT.reset(Engine);
}

void CPUDevice::InitMultiprocessors() {
  for (auto I = Machine.socket_begin(), E = Machine.socket_end(); I != E; ++I)
    Multiprocessors.insert(new Multiprocessor(*this, *I));
}

void CPUDevice::InitMultiprocessors(const HardwareCPUsContainer &CPUs) {
  Multiprocessors.insert(new Multiprocessor(*this, CPUs));
}

void CPUDevice::DestroyJIT() {
  JIT->removeModule(&*BitCodeLibrary);
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

bool CPUDevice::Submit(EnqueueReadBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new ReadBufferCPUCommand(Cmd, Global[Cmd.GetSource()]));
}

bool CPUDevice::Submit(EnqueueWriteBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new WriteBufferCPUCommand(Cmd, Global[Cmd.GetTarget()]));
}

bool CPUDevice::Submit(EnqueueCopyBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new CopyBufferCPUCommand(Cmd, Global[Cmd.GetTarget()], Global[Cmd.GetSource()]));
}

bool CPUDevice::Submit(EnqueueReadImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new ReadImageCPUCommand(Cmd, Global[Cmd.GetSource()]));
}

bool CPUDevice::Submit(EnqueueWriteImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new WriteImageCPUCommand(Cmd, Global[Cmd.GetTarget()]));
}

bool CPUDevice::Submit(EnqueueCopyImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new CopyImageCPUCommand(Cmd, Global[Cmd.GetTarget()], Global[Cmd.GetSource()]));
}

bool CPUDevice::Submit(EnqueueCopyImageToBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new CopyImageToBufferCPUCommand(Cmd, Global[Cmd.GetTarget()], Global[Cmd.GetSource()]));
}

bool CPUDevice::Submit(EnqueueCopyBufferToImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new CopyBufferToImageCPUCommand(Cmd, Global[Cmd.GetTarget()], Global[Cmd.GetSource()]));
}

bool CPUDevice::Submit(EnqueueMapBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new MapBufferCPUCommand(Cmd, Global[Cmd.GetSource()]));
}

bool CPUDevice::Submit(EnqueueMapImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new MapImageCPUCommand(Cmd, Global[Cmd.GetSource()]));
}

bool CPUDevice::Submit(EnqueueUnmapMemObject &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new UnmapMemObjectCPUCommand(Cmd, Global[Cmd.GetMemObj()], Cmd.GetMappedPtr()));
}

bool CPUDevice::Submit(EnqueueReadBufferRect &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new ReadBufferRectCPUCommand(Cmd, Cmd.GetTarget(), Global[Cmd.GetSource()]));
}

bool CPUDevice::Submit(EnqueueWriteBufferRect &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new WriteBufferRectCPUCommand(Cmd, Global[Cmd.GetTarget()], Cmd.GetSource()));
}

bool CPUDevice::Submit(EnqueueCopyBufferRect &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new CopyBufferRectCPUCommand(Cmd, Global[Cmd.GetTarget()], Global[Cmd.GetSource()]));
}

bool CPUDevice::Submit(EnqueueFillBuffer &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new FillBufferCPUCommand(Cmd, Global[Cmd.GetTarget()]));
}

bool CPUDevice::Submit(EnqueueFillImage &Cmd) {
  // TODO: implement a smarter selection policy.
  Multiprocessor &MP = **Multiprocessors.begin();

  return MP.Submit(new FillImageCPUCommand(Cmd, Global[Cmd.GetTarget()]));
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

  Memory::MappingsContainer Mappings;
  Global.GetMappings(Mappings);

  Cmd.RemapMemoryObjAddresses(Mappings);

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
  llvm::Module &Mod = *KernDesc.getFunction(this)->getParent();
  llvm::StringRef KernName = KernDesc.getFunction(this)->getName();

  // Link opencrunCPULib.bc with kernel module.
  llvm::Linker::LinkModules(&Mod,
                            &(*BitCodeLibrary),
                            llvm::Linker::PreserveSource,
                            NULL);

  // The aggressive inliner cache info about call graph shape.
  AggressiveInliner *Inliner = CreateAggressiveInlinerPass(KernName);

  // Build the entry point and optimize.
  llvm::PassManager PM;
  PM.add(Inliner);
  PM.add(CreateGroupParallelStubPass(KernName));
  PM.run(Mod);

  // Check whether there was a problem at inline time.
  if(!Inliner->IsAllInlined())
    return NULL;

  // Retrieve it.
  std::string EntryName = MangleBlockParallelKernelName(KernName);
  llvm::Function *EntryFn = Mod.getFunction(EntryName);


  // Force translation to native code.
  SignalJITCallStart(*this);
  JIT->addModule(&Mod);
  void *EntryPtr = JIT->getPointerToFunction(EntryFn);
  SignalJITCallEnd();

  // Invoke static constructors -- we do not expect static constructors in
  // OpenCL code, run only for safety.
  JIT->runStaticConstructorsDestructors(&Mod, false);

  // Cache it. In order to correctly cast the EntryPtr to a function pointer we
  // must pass through a uintptr_t. Otherwise, a warning is issued by the
  // compiler.
  uintptr_t EntryPtrInt = reinterpret_cast<uintptr_t>(EntryPtr);
  CPUDevice::BlockParallelEntryPoint Entry;
  Entry = reinterpret_cast<CPUDevice::BlockParallelEntryPoint>(EntryPtrInt);
  BlockParallelEntriesCache[&KernDesc] = Entry;

  return Entry;
}

unsigned CPUDevice::GetBlockParallelStaticLocalSize(const KernelDescriptor &KernDesc) {
  llvm::sys::ScopedLock Lock(ThisLock);

  // Cache hit.
  BlockParallelStaticLocalSizes::iterator I =
    BlockParallelStaticLocalsCache.find(&KernDesc);
  if (I != BlockParallelStaticLocalsCache.end())
    return I->second;

  // Cache miss.
  CPUKernelInfo Info(KernDesc.getKernelInfo());

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

  llvm::PassManager PM;
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

void *CPUDevice::LinkLibFunction(const std::string &Name) {
  // Bit-code function.
  if(llvm::Function *Func = JIT->FindFunctionNamed(Name.c_str()))
    return JIT->getPointerToFunction(Func);

  return NULL;
}

void CPUDevice::LocateMemoryObjArgAddresses(
                  Kernel &Kern,
                  GlobalArgMappingsContainer &GlobalArgs) {
  for(Kernel::arg_iterator I = Kern.arg_begin(),
                           E = Kern.arg_end();
                           I != E;
                           ++I)
    if(BufferKernelArg *Arg = llvm::dyn_cast<BufferKernelArg>(*I)) {
      // Local mappings handled by Multiprocessor.
      if(Arg->OnLocalAddressSpace())
        continue;

      unsigned I = Arg->GetPosition();

      if(Buffer *Buf = Arg->GetBuffer())
        GlobalArgs[I] = Global[*Buf];
      else
        GlobalArgs[I] = NULL;
    } else if(ImageKernelArg *Arg = llvm::dyn_cast<ImageKernelArg>(*I)) {
      // Images are always allocated in __global AS.
      unsigned I = Arg->GetPosition();

      if(Image *Img = Arg->GetImage())
        GlobalArgs[I] = Global[*Img];
      else
        GlobalArgs[I] = NULL;
    }
}

static void createAutoLocalVarsPass(const llvm::PassManagerBuilder &PMB,
                                    llvm::PassManagerBase &PM) {
  PM.add(createAutomaticLocalVariablesPass());
}

void CPUDevice::addOptimizerExtensions(llvm::PassManagerBuilder &PMB,
                                       LLVMOptimizerParams &Params) const {
  PMB.addExtension(llvm::PassManagerBuilder::EP_ModuleOptimizerEarly,
                   createAutoLocalVarsPass);
  PMB.addExtension(llvm::PassManagerBuilder::EP_EnabledOnOptLevel0,
                   createAutoLocalVarsPass);
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

//
// LibLinker implementation.
//

namespace {

llvm::sys::ThreadLocal<const CPUDevice> CurDevice;

} // End anonymous namespace.

void *opencrun::LibLinker(const std::string &Name) {
  void *Func = NULL;

  if(CPUDevice *Dev = const_cast<CPUDevice *>(CurDevice.get()))
    Func = Dev->LinkLibFunction(Name);

  return Func;
}

namespace {

void SignalJITCallStart(CPUDevice &Dev) {
  // Save current device, it can be used by the JIT trampoline later.
  CurDevice.set(&Dev);
}

void SignalJITCallEnd() {
  // JIT is done, remove current device.
  CurDevice.erase();
}

} // End anonymous namespace.

void opencrun::initializeCPUDevice(Platform &P) {
  sys::Hardware &HW = sys::GetHardware();

  // A device for each Machine in the System (it may be a cluster system).
  for (sys::Hardware::machine_iterator I = HW.machine_begin(),
       E = HW.machine_end(); I != E; ++I)
    P.addDevice(new CPUDevice(*I));
}
