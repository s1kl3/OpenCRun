
#include "opencrun/Device/CPU/CPUDevice.h"
#include "opencrun/Core/Event.h"
#include "opencrun/Device/CPU/InternalCalls.h"
#include "opencrun/Device/CPUPasses/AllPasses.h"
#include "opencrun/Passes/AggressiveInliner.h"
#include "opencrun/Passes/AllPasses.h"
#include "opencrun/Util/BuiltinInfo.h"

#include "llvm/Linker.h"
#include "llvm/ADT/STLExtras.h"
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

CPUDevice::CPUDevice(sys::HardwareNode &Node) :
  Device("CPU", llvm::sys::getDefaultTargetTriple()),
  Global(Node.GetMemorySize()) {
  InitDeviceInfo(Node);
  InitMultiprocessors(Node);
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
  llvm::OwningPtr<ProfileSample> Sample;

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
  // llvm::OwningPtr destructor will reclaim the sample.
  if(Submitted) {
    InternalEvent &Ev = Cmd.GetNotifyEvent();
    Ev.MarkSubmitted(Sample.take());
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

void CPUDevice::InitDeviceInfo(sys::HardwareNode &Node) {
  // Assuming symmetric systems.
  const sys::HardwareCache &L1Cache = Node.l1c_front();
  const sys::HardwareCache &LLCache = Node.llc_front();

  // TODO: define device geometry and set all properties!

  VendorID = 0;
  MaxComputeUnits = Node.CPUsCount();
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
  Extensions = "cl_khr_fp64"
               "cl_khr_global_int32_base_atomics"
               "cl_khr_global_int32_extended_atomics"
               "cl_khr_local_int32_base_atomics"
               "cl_khr_local_int32_extended_atomics"
               "cl_khr_int64_base_atomics"
               "cl_khr_int64_extended_atomics"
               "cl_khr_3d_image_writes";
#else
  AddressBits = 32;
  Extensions = "cl_khr_global_int32_base_atomics"
               "cl_khr_global_int32_extended_atomics"
               "cl_khr_local_int32_base_atomics"
               "cl_khr_local_int32_extended_atomics"
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

  MaxMemoryAllocSize = Node.GetMemorySize();

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

  SinglePrecisionFPCapabilities = Device::FPDenormalization | 
                                  Device::FPInfNaN |
                                  Device::FPRoundToNearest |
                                  Device::FPRoundToZero |
                                  Device::FPRoundToInf |
                                  Device::FPFusedMultiplyAdd |
                                  FPCorrectlyRoundedDivideSqrt;

  DoublePrecisionFPCapabilities = Device::FPDenormalization | 
                                  Device::FPInfNaN |
                                  Device::FPRoundToNearest |
                                  Device::FPRoundToZero |
                                  Device::FPRoundToInf |
                                  Device::FPFusedMultiplyAdd;

  GlobalMemoryCacheType = DeviceInfo::ReadWriteCache;

  GlobalMemoryCachelineSize = LLCache.GetLineSize();
  GlobalMemoryCacheSize = LLCache.GetSize();

  GlobalMemorySize = Node.GetMemorySize();

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
    Func = DeviceBuiltinInfo::getPrototype(*BitCodeLibrary, "__builtin_ocl_" #N, Fmt); \
    Engine->addGlobalMapping(Func, Addr);
  #include "InternalCalls.def"
  #undef INTERNAL_CALL

  // Save pointer.
  JIT.reset(Engine);
}

void CPUDevice::InitMultiprocessors(sys::HardwareNode &Node) {
  for(sys::HardwareNode::const_llc_iterator I = Node.llc_begin(),
                                            E = Node.llc_end();
                                            I != E;
                                            ++I)
    Multiprocessors.insert(new Multiprocessor(*this, *I));
}

void CPUDevice::DestroyJIT() {
  JIT->removeModule(&*BitCodeLibrary);
}

void CPUDevice::DestroyMultiprocessors() {
  llvm::DeleteContainerPointers(Multiprocessors);
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
                            0,
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
// LibLinker implementation
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
