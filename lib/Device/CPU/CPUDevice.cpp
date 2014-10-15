
#include "opencrun/Device/CPU/CPUDevice.h"
#include "opencrun/Core/Event.h"
#include "opencrun/Device/CPU/InternalCalls.h"
#include "opencrun/Device/CPUPasses/AllPasses.h"
#include "opencrun/Passes/AggressiveInliner.h"
#include "opencrun/Passes/AllPasses.h"
#include "opencrun/Util/BuiltinInfo.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Linker/Linker.h"
#include "llvm/PassManager.h"
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
// Supported image formats.
//

static const cl_image_format CPUImgFmts[] = {
  { CL_R, CL_SNORM_INT8 },
  { CL_R, CL_SNORM_INT16 },
  { CL_R, CL_UNORM_INT8 },
  { CL_R, CL_UNORM_INT16 },
  { CL_R, CL_SIGNED_INT8 },
  { CL_R, CL_SIGNED_INT16 },
  { CL_R, CL_SIGNED_INT32 },
  { CL_R, CL_UNSIGNED_INT8 },
  { CL_R, CL_UNSIGNED_INT16 },
  { CL_R, CL_UNSIGNED_INT32 },
  { CL_R, CL_HALF_FLOAT },
  { CL_R, CL_FLOAT },
  { CL_A, CL_SNORM_INT8 },
  { CL_A, CL_SNORM_INT16 },
  { CL_A, CL_UNORM_INT8 },
  { CL_A, CL_UNORM_INT16 },
  { CL_A, CL_SIGNED_INT8 },
  { CL_A, CL_SIGNED_INT16 },
  { CL_A, CL_SIGNED_INT32 },
  { CL_A, CL_UNSIGNED_INT8 },
  { CL_A, CL_UNSIGNED_INT16 },
  { CL_A, CL_UNSIGNED_INT32 },
  { CL_A, CL_HALF_FLOAT },
  { CL_A, CL_FLOAT },
  { CL_RG, CL_SNORM_INT8 },
  { CL_RG, CL_SNORM_INT16 },
  { CL_RG, CL_UNORM_INT8 },
  { CL_RG, CL_UNORM_INT16 },
  { CL_RG, CL_SIGNED_INT8 },
  { CL_RG, CL_SIGNED_INT16 },
  { CL_RG, CL_SIGNED_INT32 },
  { CL_RG, CL_UNSIGNED_INT8 },
  { CL_RG, CL_UNSIGNED_INT16 },
  { CL_RG, CL_UNSIGNED_INT32 },
  { CL_RG, CL_HALF_FLOAT },
  { CL_RG, CL_FLOAT },
  { CL_RGBA, CL_SNORM_INT8 },
  { CL_RGBA, CL_SNORM_INT16 },
  { CL_RGBA, CL_UNORM_INT8 },
  { CL_RGBA, CL_UNORM_INT16 },
  { CL_RGBA, CL_SIGNED_INT8 },
  { CL_RGBA, CL_SIGNED_INT16 },
  { CL_RGBA, CL_SIGNED_INT32 },
  { CL_RGBA, CL_UNSIGNED_INT8 },
  { CL_RGBA, CL_UNSIGNED_INT16 },
  { CL_RGBA, CL_UNSIGNED_INT32 },
  { CL_RGBA, CL_HALF_FLOAT },
  { CL_RGBA, CL_FLOAT },
  { CL_ARGB, CL_SNORM_INT8 },
  { CL_ARGB, CL_UNORM_INT8 },
  { CL_ARGB, CL_SIGNED_INT8 },
  { CL_ARGB, CL_UNSIGNED_INT8 },
  { CL_BGRA, CL_SNORM_INT8 },
  { CL_BGRA, CL_UNORM_INT8 },
  { CL_BGRA, CL_SIGNED_INT8 },
  { CL_BGRA, CL_UNSIGNED_INT8 },
  { CL_LUMINANCE, CL_SNORM_INT8 },
  { CL_LUMINANCE, CL_SNORM_INT16 },
  { CL_LUMINANCE, CL_UNORM_INT8 },
  { CL_LUMINANCE, CL_UNORM_INT16 },
  { CL_LUMINANCE, CL_HALF_FLOAT },
  { CL_LUMINANCE, CL_FLOAT },
  { CL_INTENSITY, CL_SNORM_INT8 },
  { CL_INTENSITY, CL_SNORM_INT16 },
  { CL_INTENSITY, CL_UNORM_INT8 },
  { CL_INTENSITY, CL_UNORM_INT16 },
  { CL_INTENSITY, CL_HALF_FLOAT },
  { CL_INTENSITY, CL_FLOAT },
  { CL_RA, CL_SNORM_INT8 },
  { CL_RA, CL_SNORM_INT16 },
  { CL_RA, CL_UNORM_INT8 },
  { CL_RA, CL_UNORM_INT16 },
  { CL_RA, CL_SIGNED_INT8 },
  { CL_RA, CL_SIGNED_INT16 },
  { CL_RA, CL_SIGNED_INT32 },
  { CL_RA, CL_UNSIGNED_INT8 },
  { CL_RA, CL_UNSIGNED_INT16 },
  { CL_RA, CL_UNSIGNED_INT32 },
  { CL_RA, CL_HALF_FLOAT },
  { CL_RA, CL_FLOAT }
};

//
// CPUDevice implementation.
//

CPUDevice::CPUDevice(const sys::HardwareMachine &Machine) :
  Device("CPU", llvm::sys::getDefaultTargetTriple()),
  Machine(Machine),
  Global(Machine.GetTotalMemorySize()) {
  InitDeviceInfo(Machine);
  InitMultiprocessors(Machine);
  InitJIT();
}

CPUDevice::CPUDevice(CPUDevice &Parent,
                     const PartitionPropertiesContainer &PartProps,
                     const HardwareCPUsContainer &CPUs) :
  Device(Parent, PartProps),
  Machine(Parent.GetHardwareMachine()),
  Global(Parent.GetHardwareMachine().GetTotalMemorySize()) {
    InitDeviceInfo(Parent.GetHardwareMachine(), &CPUs);
    InitMultiprocessors(CPUs);
    InitJIT();
}

CPUDevice::~CPUDevice() {
  DestroyMultiprocessors();
  DestroyJIT();
}

bool CPUDevice::IsSupportedPartitionSchema(const PartitionPropertiesContainer &PartProps,
                                           cl_int &ErrCode) const {
  // Check for partitioning type support.
  if(!std::count(PartTys.begin(), PartTys.end(), PartProps[0])) {
    ErrCode = CL_INVALID_VALUE;
    return false;
  }

  // Check for affinity domain support.
  if(PartProps[0] == DeviceInfo::PartitionByAffinityDomain) {
    if(!(AffinityDomains & static_cast<cl_device_affinity_domain>(PartProps[1]))) {
      ErrCode = CL_INVALID_VALUE;
      return false;
    }
  }

  // For partitioning by counts check the number of sub-devices and compute units
  // requestes.
  if(PartProps[0] == CL_DEVICE_PARTITION_BY_COUNTS) {
    // Number of sub-devices requested.
    unsigned NumSubDevs = 0;
    // Number of CUs requested.
    unsigned NumCUs = 0;
    for(unsigned i = 1; PartProps[i] != DeviceInfo::PartitionByCountsListEnd; ++i) {
      if(PartProps[i] < 0) {
        ErrCode = CL_INVALID_DEVICE_PARTITION_COUNT;
        return 0;
      }

      ++NumSubDevs;
      NumCUs += PartProps[i];
    }

    if(NumSubDevs > GetMaxSubDevices() ||
       NumCUs > GetMaxComputeUnits()) {
      ErrCode = CL_INVALID_DEVICE_PARTITION_COUNT;
      return false;
    }

  }

  return true;
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
  Sample.reset(GetProfilerSample(*this,
                                 Counters,
                                 ProfileSample::CommandSubmitted));

  if(EnqueueReadBuffer *Read = llvm::dyn_cast<EnqueueReadBuffer>(&Cmd))
    Submitted = Submit(*Read);

  else if(EnqueueWriteBuffer *Write = llvm::dyn_cast<EnqueueWriteBuffer>(&Cmd))
    Submitted = Submit(*Write);

  else if(EnqueueCopyBuffer *Copy = llvm::dyn_cast<EnqueueCopyBuffer>(&Cmd))
    Submitted = Submit(*Copy);

  else if(EnqueueReadImage *ReadImg = llvm::dyn_cast<EnqueueReadImage>(&Cmd))
    Submitted = Submit(*ReadImg);

  else if(EnqueueWriteImage *WriteImg = llvm::dyn_cast<EnqueueWriteImage>(&Cmd))
    Submitted = Submit(*WriteImg);

  else if(EnqueueCopyImage *CopyImg = llvm::dyn_cast<EnqueueCopyImage>(&Cmd))
    Submitted = Submit(*CopyImg);

  else if(EnqueueCopyImageToBuffer *CopyImgToBuf = 
      llvm::dyn_cast<EnqueueCopyImageToBuffer>(&Cmd))
    Submitted = Submit(*CopyImgToBuf);

  else if(EnqueueCopyBufferToImage *CopyBufToImg = 
      llvm::dyn_cast<EnqueueCopyBufferToImage>(&Cmd))
    Submitted = Submit(*CopyBufToImg);

  else if(EnqueueMapBuffer *MapBuf = llvm::dyn_cast<EnqueueMapBuffer>(&Cmd))
    Submitted = Submit(*MapBuf);
  
  else if(EnqueueMapImage *MapImg = llvm::dyn_cast<EnqueueMapImage>(&Cmd))
    Submitted = Submit(*MapImg);

  else if(EnqueueUnmapMemObject *Unmap = llvm::dyn_cast<EnqueueUnmapMemObject>(&Cmd))
    Submitted = Submit(*Unmap);

  else if(EnqueueReadBufferRect *ReadRect = llvm::dyn_cast<EnqueueReadBufferRect>(&Cmd))
    Submitted = Submit(*ReadRect);

  else if(EnqueueWriteBufferRect *WriteRect = llvm::dyn_cast<EnqueueWriteBufferRect>(&Cmd))
    Submitted = Submit(*WriteRect);
    
  else if(EnqueueCopyBufferRect *CopyRect = llvm::dyn_cast<EnqueueCopyBufferRect>(&Cmd))
    Submitted = Submit(*CopyRect);

  else if(EnqueueFillBuffer *Fill = llvm::dyn_cast<EnqueueFillBuffer>(&Cmd))
    Submitted = Submit(*Fill);
    
  else if(EnqueueFillImage *Fill = llvm::dyn_cast<EnqueueFillImage>(&Cmd))
    Submitted = Submit(*Fill);

  else if(EnqueueNDRangeKernel *NDRange =
            llvm::dyn_cast<EnqueueNDRangeKernel>(&Cmd))
    Submitted = Submit(*NDRange);

  else if(EnqueueNativeKernel *Native =
            llvm::dyn_cast<EnqueueNativeKernel>(&Cmd))
    Submitted = Submit(*Native);

  else
    llvm::report_fatal_error("unknown command submitted");

  // The command has been submitted, register the sample. On failure, the
  // std::unique_ptr destructor will reclaim the sample.
  if(Submitted) {
    InternalEvent &Ev = Cmd.GetNotifyEvent();
    Ev.MarkSubmitted(Sample.release());
  }

  return Submitted;
}

void CPUDevice::UnregisterKernel(Kernel &Kern) {
  // TODO: modules must be ref-counted -- unregister a kernel does not
  // necessary enforce module unloading?
  llvm::sys::ScopedLock Lock(ThisLock);

  // Remove module from the JIT.
  llvm::Module &Mod = *Kern.GetModule(*this);
  JIT->removeModule(&Mod);

  KernelID K = Kern.GetFunction(*this);

  // Erase kernel from the cache.
  BlockParallelEntriesCache.erase(K);
  BlockParallelStaticLocalsCache.erase(K);
  KernelFootprints.erase(K);

  // Invoke static destructors.
  JIT->runStaticConstructorsDestructors(&Mod, true);
}

void CPUDevice::NotifyDone(CPUExecCommand *Cmd, int ExitStatus) {
  // Get counters to profile.
  Command &QueueCmd = Cmd->GetQueueCommand();
  InternalEvent &Ev = QueueCmd.GetNotifyEvent();
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
  }

  delete Cmd;
}

void CPUDevice::InitDeviceInfo(const sys::HardwareMachine &Machine, const HardwareCPUsContainer *CPUs) {
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
  NumImgFmts = sizeof(CPUImgFmts)/sizeof(cl_image_format);
  ImgFmts = CPUImgFmts;

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

  // Pointers to HardwareCPUs which are part of the current device (root/sub-device)
  // are copied inside the following container.
  HardwareCPUsContainer DevCPUs;
  if(Parent && CPUs) {
    MaxComputeUnits = CPUs->size();
    DevCPUs = *CPUs;
  } else {
    MaxComputeUnits = Machine.GetNumCoveredCPUs();
    // The CPU iterators from sys::HardwareMachine class are preferred over the
    // GetPinnedCPUs method in order to avoid dependency from the InitMultiprocessors
    // method.
    for(sys::HardwareMachine::const_cpu_iterator I = Machine.cpu_begin(),
                                                 E = Machine.cpu_end();
                                                 I != E;
                                                 ++I)
      DevCPUs.insert(&(*I));
  }

  MaxSubDevices = MaxComputeUnits;

  // Partitioning makes sense only if the current device refers to more than one
  // compute unit.
  if(MaxComputeUnits > 1) {
    PartTys.push_back(DeviceInfo::PartitionEqually); 
    PartTys.push_back(DeviceInfo::PartitionByCounts);

    std::map<sys::HardwareComponent *, HardwareCPUsContainer> AffinityPartitions;
    for(HardwareCPUsContainer::iterator I = DevCPUs.begin(), E = DevCPUs.end(); I != E; ++I) {
      if(sys::HardwareNode *NUMANode = (*I)->GetNUMANode())
        AffinityPartitions[NUMANode].insert(*I);

      for(unsigned J = 1; J <= 4; ++J) {
        if(sys::HardwareCache *Cache = (*I)->GetCache(J))
          AffinityPartitions[Cache].insert(*I);
      }
    }
  
    for(std::map<sys::HardwareComponent *, HardwareCPUsContainer>::iterator I = AffinityPartitions.begin(),
                                                                            E = AffinityPartitions.end();
                                                                            I != E;
                                                                            ++I) {
      if(llvm::isa<sys::HardwareNode>(I->first))
        Partitions[DeviceInfo::AffinityDomainNUMA].push_back(I->second);
      else if(sys::HardwareCache *Cache = llvm::dyn_cast<sys::HardwareCache>(I->first)) {
        switch(Cache->GetLevel()) {
        case 4:
          Partitions[DeviceInfo::AffinityDomainL4Cache].push_back(I->second);
          break;
        case 3:
          Partitions[DeviceInfo::AffinityDomainL3Cache].push_back(I->second);
          break;
        case 2:
          Partitions[DeviceInfo::AffinityDomainL2Cache].push_back(I->second);
          break;
        case 1:
          Partitions[DeviceInfo::AffinityDomainL1Cache].push_back(I->second);
          break;
        }
      }
    }

    for(PartitionsContainer::iterator I = Partitions.begin(),
                                      E = Partitions.end();
                                      I != E;
                                      ++I) {
      if(I->second.size() > 1) {
        AffinityDomains |= I->first;
      }
    }

    if(AffinityDomains) {
      AffinityDomains |= DeviceInfo::AffinityDomainNext;
      PartTys.push_back(DeviceInfo::PartitionByAffinityDomain);
    }

  }

}

void CPUDevice::InitJIT() {
  // Init the native target.
  llvm::InitializeNativeTarget();

  // Create the JIT.
  llvm::EngineBuilder Bld(&*BitCodeLibrary);
  llvm::ExecutionEngine *Engine = Bld.setEngineKind(llvm::EngineKind::JIT)
                                     .setOptLevel(llvm::CodeGenOpt::None)
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

void CPUDevice::InitMultiprocessors(const sys::HardwareMachine &Machine) {
  for(sys::HardwareMachine::const_socket_iterator I = Machine.socket_begin(),
                                                  E = Machine.socket_end();
                                                  I != E;
                                                  ++I)
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

bool CPUDevice::BlockParallelSubmit(EnqueueNDRangeKernel &Cmd,
                                    GlobalArgMappingsContainer &GlobalArgs) {
  // Native launcher address.
  BlockParallelEntryPoint Entry = GetBlockParallelEntryPoint(Cmd.GetKernel());

  // Index space.
  DimensionInfo &DimInfo = Cmd.GetDimensionInfo();

  // Static local size
  unsigned StaticLocalSize = GetBlockParallelStaticLocalSize(Cmd.GetKernel());

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
CPUDevice::GetBlockParallelEntryPoint(Kernel &Kern) {
  llvm::sys::ScopedLock Lock(ThisLock);

  KernelID K = Kern.GetFunction(*this);

  // Cache hit.
  BlockParallelEntryPoints::iterator I = BlockParallelEntriesCache.find(K);
  if (I != BlockParallelEntriesCache.end())
    return I->second;

  // Cache miss.
  llvm::Module &Mod = *Kern.GetModule(*this);
  llvm::StringRef KernName = Kern.GetName();

  // The aggressive inliner cache info about call graph shape.
  AggressiveInliner *Inliner = CreateAggressiveInlinerPass(KernName);

  // Build the entry point and optimize.
  llvm::PassManager PM;
  PM.add(Inliner);
  PM.add(CreateGroupParallelStubPass(this, KernName));
  PM.run(Mod);

  // Check whether there was a problem at inline time.
  if(!Inliner->IsAllInlined())
    return NULL;

  // Retrieve it.
  std::string EntryName = MangleBlockParallelKernelName(KernName);
  llvm::Function *EntryFn = Mod.getFunction(EntryName);

  // Link opencrunCPULib.bc with kernel module.
  llvm::Linker::LinkModules(&Mod,
                            &(*BitCodeLibrary),
                            llvm::Linker::PreserveSource,
                            NULL);

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
  BlockParallelEntriesCache[K] = Entry;

  return Entry;
}

unsigned CPUDevice::GetBlockParallelStaticLocalSize(Kernel &Kern) {
  llvm::sys::ScopedLock Lock(ThisLock);

  KernelID K = Kern.GetFunction(*this);

  BlockParallelStaticLocalSizes::iterator I =
    BlockParallelStaticLocalsCache.find(K);
  if (I != BlockParallelStaticLocalsCache.end())
    return I->second;

  llvm::Module &Mod = *Kern.GetModule(*this);

  llvm::KernelInfo Info = ModuleInfo(Mod).getKernelInfo(Kern.GetName());

  unsigned StaticLocalSize = Info.getStaticLocalSize();
  BlockParallelStaticLocalsCache[K] = StaticLocalSize;

  return StaticLocalSize;
}

const Footprint &CPUDevice::ComputeKernelFootprint(Kernel &Kern) {
  llvm::sys::ScopedLock Lock(ThisLock);

  KernelID K = Kern.GetFunction(*this);
  FootprintsContainer::iterator I = KernelFootprints.find(K);

  if (I != KernelFootprints.end())
    return I->second;

  FootprintEstimate *Pass = CreateFootprintEstimatePass(GetName());
  llvm::Function *Fun = Kern.GetFunction(*this);

  llvm::PassManager PM;
  PM.add(Pass);
  PM.run(*Fun->getParent());

  KernelFootprints[K] = *Pass;

  return KernelFootprints[K];
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

//
// CPUSubDevicesBuilder implementation.
//

unsigned CPUSubDevicesBuilder::Create(SubDevicesContainer *SubDevs, cl_int &ErrCode) {
  CPUDevice::HardwareCPUsContainer CPUs;
  cl_uint NumSubDevs = 0;

  CPUDevice *ParentCPUDev = llvm::dyn_cast<CPUDevice>(&Parent);
  if(!ParentCPUDev) {
    // Parent device is not a CPUDevice.
    ErrCode = CL_INVALID_DEVICE;
    return 0;
  }

  if(!ParentCPUDev->IsSupportedPartitionSchema(PartProps, ErrCode))
    return 0;

  ParentCPUDev->GetPinnedCPUs(CPUs);
  
  switch(PartProps[0]) {
    case DeviceInfo::PartitionEqually:
      {
        CPUDevice::HardwareCPUsContainer SubDevCPUs;
        unsigned K = 1; 
        for(CPUDevice::HardwareCPUsContainer::iterator I = CPUs.begin(),
                                                       E = CPUs.end();
                                                       I != E;
                                                       ++I) {
          SubDevCPUs.insert(*I);
          if(K == static_cast<unsigned>(PartProps[1])) {
            if(SubDevs)
              SubDevs->insert(new CPUDevice(*ParentCPUDev, PartProps, SubDevCPUs));

            SubDevCPUs.clear();
            ++NumSubDevs;
            K = 1;
          } else
            ++K;
        }

        break;
      }

    case DeviceInfo::PartitionByCounts:
      {
        CPUDevice::HardwareCPUsContainer SubDevCPUs;
        CPUDevice::HardwareCPUsContainer::iterator I = CPUs.begin(),
                                                   E = CPUs.end();

        for(unsigned J = 1; static_cast<unsigned>(PartProps[J]) != DeviceInfo::PartitionByCountsListEnd; ++J) {
          unsigned K = 1;
          for(; I != E && K <= static_cast<unsigned>(PartProps[J]); ++I, ++K) {
            SubDevCPUs.insert(*I);
            if(K == static_cast<unsigned>(PartProps[J])) {
              if(SubDevs)
                SubDevs->insert(new CPUDevice(*ParentCPUDev, PartProps, SubDevCPUs));
              SubDevCPUs.clear();
              ++NumSubDevs;
            }
          }
        }

        break;
      }

    case DeviceInfo::PartitionByAffinityDomain:
      {
        cl_device_affinity_domain AffinityTy;

        if(static_cast<cl_device_affinity_domain>(PartProps[1]) == DeviceInfo::AffinityDomainNext) {
          // Split the CPUDevice along the first available partitionable affinity domain, in the order
          // NUMA, L4, L3, L2, L1.
          if(ParentCPUDev->AffinityDomains & DeviceInfo::AffinityDomainNUMA)
            AffinityTy = DeviceInfo::AffinityDomainNUMA;
          else if(ParentCPUDev->AffinityDomains & DeviceInfo::AffinityDomainL4Cache)
            AffinityTy = DeviceInfo::AffinityDomainL4Cache;
          else if(ParentCPUDev->AffinityDomains & DeviceInfo::AffinityDomainL3Cache)
            AffinityTy = DeviceInfo::AffinityDomainL3Cache;
          else if(ParentCPUDev->AffinityDomains & DeviceInfo::AffinityDomainL2Cache)
            AffinityTy = DeviceInfo::AffinityDomainL2Cache;
          else if(ParentCPUDev->AffinityDomains & DeviceInfo::AffinityDomainL1Cache)
            AffinityTy = DeviceInfo::AffinityDomainL1Cache;
          else {
            ErrCode = CL_INVALID_VALUE;
            return 0;
          }
        } else
          AffinityTy = static_cast<cl_device_affinity_domain>(PartProps[1]);

        if(!ParentCPUDev->Partitions.count(AffinityTy)) {
          // Marked as supported by the CPUDevice but no partition is found inside
          // the Partitions container.
          ErrCode = CL_INVALID_VALUE;
          return 0;
        }

        // Get the reference to the pre-calculated partition.
        llvm::SmallVector<CPUDevice::HardwareCPUsContainer, 4> &Partition = ParentCPUDev->Partitions[AffinityTy];
        for(unsigned J = 0; J < Partition.size(); ++J) {
          if(SubDevs)
            SubDevs->insert(new CPUDevice(*ParentCPUDev, PartProps, Partition[J]));
          ++NumSubDevs;
        }

        break;
      }
  }

  ErrCode = CL_SUCCESS;
  return NumSubDevs;
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
