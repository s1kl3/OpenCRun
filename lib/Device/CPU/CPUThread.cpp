
#include "CPUThread.h"
#include "CPUDevice.h"

#include "opencrun/Core/Event.h"
#include "opencrun/System/OS.h"

#include "llvm/Support/Format.h"

//
// Include low-level routines for current architecture related to stack
// management. All symbols should use the C linking convention.
//

#if defined(__x86_64__)
#include "StackX8664.inc"
#elif defined(__i386__)
#include "StackX8632.inc"
#else
#error "architecture not supported"
#endif

using namespace opencrun;
using namespace opencrun::cpu;

namespace { // Anonymous namespace.

//
// Routines to manage current thread pointer in the TLS.
//

static void SetCurrentThread(CPUThread *Thr);
static void ResetCurrentThread();

} // End anonymous namespace.

//
// ExecutionStack implementation.
//

ExecutionStack::ExecutionStack(const sys::HardwareCache &Cache) {
  // Ideally, all the stack should reside in the L1 cache, together with the
  // local memory.
  StackSize = 4 * Cache.GetSize();
  Stack = sys::PageAlignedAlloc(StackSize);
}

ExecutionStack::~ExecutionStack() {
  #ifndef NDEBUG
  sys::MarkPagesReadWrite(Stack, StackSize);
  #endif
  sys::Free(Stack);
}

size_t ExecutionStack::getRequiredStackSize(unsigned N) const {
  return N * getPrivateSize() + sys::Hardware::GetPageSize();
}

size_t ExecutionStack::getPrivateSize() const {
  #ifndef NDEBUG
  const unsigned K = 1 + 1;
  #else
  const unsigned K = 1;
  #endif
  return K * sys::Hardware::GetPageSize();
}

void ExecutionStack::Reset(EntryPoint Entry, void **Args, unsigned N) {
  size_t RequiredStackSize = getRequiredStackSize(N);
  size_t PrivateSize = getPrivateSize();

  // Stack too small, expand it.
  if(StackSize < RequiredStackSize) {
    #ifndef NDEBUG
    sys::MarkPagesReadWrite(Stack, StackSize);
    #endif
    sys::Free(Stack);
    Stack = sys::PageAlignedAlloc(RequiredStackSize);
    StackSize = RequiredStackSize;

    assert(Stack);
  }

  #ifndef NDEBUG
  sys::MarkPagesReadWrite(Stack, StackSize);

  // Zero the memory.
  std::memset(Stack, 0, StackSize);
  #endif

  uintptr_t BaseAddr = reinterpret_cast<uintptr_t>(Stack);
  uintptr_t Addr = BaseAddr;
  uintptr_t PrevAddr = 0;

  for (unsigned i = 0; i != N; ++i) {
    InitWorkItemStack(reinterpret_cast<void*>(Addr), PrivateSize, Entry, Args);

    if (PrevAddr)
      SetWorkItemStackLink(reinterpret_cast<void*>(Addr),
                           reinterpret_cast<void*>(PrevAddr),
                           PrivateSize);

    #ifndef NDEBUG
    // Mark first of each private memory as read-only.
    sys::MarkPageReadOnly(reinterpret_cast<void*>(Addr));
    #endif

    PrevAddr = Addr;
    Addr += PrivateSize;
  }

  SetWorkItemStackLink(reinterpret_cast<void*>(BaseAddr),
                       reinterpret_cast<void*>(PrevAddr),
                       PrivateSize);

  #ifndef NDEBUG
  // Mark page after last stack as read-only.
  sys::MarkPageReadOnly(reinterpret_cast<void*>(Addr));
  #endif
}

void ExecutionStack::Run() {
  uintptr_t StackAddr = reinterpret_cast<uintptr_t>(Stack);

  RunWorkItems(reinterpret_cast<void *>(StackAddr), getPrivateSize());
}

void ExecutionStack::SwitchToNextWorkItem() {
  SwitchWorkItem();
}

void ExecutionStack::Dump() const {
  llvm::raw_ostream &OS = llvm::errs();

  uintptr_t StartAddr = reinterpret_cast<uintptr_t>(Stack);
  uintptr_t EndAddr = StartAddr + StackSize;
  size_t PageSize = sys::Hardware::GetPageSize();

  OS << llvm::format("Stack [%p, %p]", StartAddr, EndAddr)
     << llvm::format(" => %zd bytes, %u pages", StackSize, StackSize / PageSize)
     << ":";
  Dump(reinterpret_cast<void *>(StartAddr), reinterpret_cast<void *>(EndAddr));
}

void ExecutionStack::Dump(unsigned I) const {
  llvm::raw_ostream &OS = llvm::errs();

  uintptr_t StartAddr = reinterpret_cast<uintptr_t>(Stack) +
                        I * getPrivateSize();
  uintptr_t EndAddr = StartAddr + getPrivateSize();

  OS << llvm::format("Work-item stack [%p, %p]", StartAddr, EndAddr)
     << ":";
  Dump(reinterpret_cast<void *>(StartAddr), reinterpret_cast<void *>(EndAddr));
}

void ExecutionStack::Dump(void *StartAddr, void *EndAddr) const {
  llvm::raw_ostream &OS = llvm::errs();

  size_t PageSize = sys::Hardware::GetPageSize();

  for(uintptr_t Addr = reinterpret_cast<uintptr_t>(StartAddr);
                Addr < reinterpret_cast<uintptr_t>(EndAddr);
                Addr += 16) {
    if((Addr % PageSize) == 0)
      OS << "\n";

    OS.indent(2) << llvm::format("%p:", Addr);
    for(unsigned I = 0; I < 16; ++I) {
      uint8_t *Byte = reinterpret_cast<uint8_t *>(Addr) + I;
      OS << llvm::format(" %02x", *(Byte));
    }
    OS << "\n";
  }
}


//
// CPUThread implementation.
//

CPUThread::CPUThread(CPUDevice &Dev, const sys::HardwareCPU &CPU) :
  Thread(CPU),
  Mode(FullyOperational),
  Dev(Dev),
  Stack(*CPU.GetFirstLevelCache()),
  Local(*CPU.GetFirstLevelCache()) {
  Start();
}

CPUThread::~CPUThread() {
  // Tell worker thread to stop activities.
  submit<StopDeviceCPUCommand>();

  // Use Thread::Join to suspend current thread until the joined thread exits.
  Join();
}

bool CPUThread::Submit(std::unique_ptr<CPUCommand> Cmd) {
  sys::ScopedMonitor Mnt(ThisMnt);

  if (Mode & NoNewJobs)
    return false;

  switch (Cmd->GetType()) {
  default:
    llvm::report_fatal_error("unknown command submitted");
  case CPUCommand::StopDevice:
    Mode = TearDown;
    break;
  case CPUCommand::ReadBuffer:
  case CPUCommand::WriteBuffer:
  case CPUCommand::CopyBuffer:
  case CPUCommand::ReadImage:
  case CPUCommand::WriteImage:
  case CPUCommand::CopyImage:
  case CPUCommand::CopyImageToBuffer:
  case CPUCommand::CopyBufferToImage:
  case CPUCommand::MapBuffer:
  case CPUCommand::MapImage:
  case CPUCommand::UnmapMemObject:
  case CPUCommand::ReadBufferRect:
  case CPUCommand::WriteBufferRect:
  case CPUCommand::CopyBufferRect:
  case CPUCommand::FillBuffer:
  case CPUCommand::FillImage:
  case CPUCommand::NativeKernel:
  case CPUCommand::NoOp:
  case CPUCommand::NDRangeKernelBlock:
    break;
  }

  Commands.push_back(std::move(Cmd));
  Mnt.Signal();

  return true;
}

void CPUThread::Run() {
  SetCurrentThread(this);

  while(Mode & ExecJobs) {
    ThisMnt.Enter();

    while(Commands.empty())
      ThisMnt.Wait();

    auto Cmd = std::move(Commands.front());
    Commands.pop_front();

    ThisMnt.Exit();

    Execute(std::move(Cmd));
  }

  ResetCurrentThread();
}

float CPUThread::GetLoadIndicator() {
  sys::ScopedMonitor Mnt(ThisMnt);

  return Commands.size();
}

void CPUThread::SwitchToNextWorkItem() {
  if(++Cur == End)
    Cur = Begin;

  Stack.SwitchToNextWorkItem();
}

void CPUThread::Execute(std::unique_ptr<CPUCommand> Cmd) {
  if (auto *OnFly = llvm::dyn_cast<CPUExecCommand>(Cmd.get()))
    Execute(*OnFly);

  if (auto *OnFly = llvm::dyn_cast<CPUServiceCommand>(Cmd.get()))
    Execute(*OnFly);
}

void CPUThread::Execute(CPUServiceCommand &Cmd) {
#define DISPATCH(CmdType)                              \
  case CPUCommand::CmdType:                            \
    Execute(*llvm::cast<CmdType ## CPUCommand>(&Cmd)); \
    break;

  switch (Cmd.GetType()) {
  default: llvm_unreachable("Unknown command type!");
  DISPATCH(StopDevice)
  }

#undef DISPATCH
}

void CPUThread::Execute(StopDeviceCPUCommand &Cmd) {
  Mode = Stopped;
}

void CPUThread::Execute(CPUExecCommand &Cmd) {
  int ExitStatus;

  // Command started.
  Cmd.notifyStart();

#define DISPATCH(CmdType)                                           \
  case CPUCommand::CmdType:                                         \
    ExitStatus = Execute(*llvm::cast<CmdType ## CPUCommand>(&Cmd)); \
    break;

  switch (Cmd.GetType()) {
  default: ExitStatus = CPUCommand::Unsupported; break;
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
  DISPATCH(NDRangeKernelBlock)
  DISPATCH(NativeKernel)
  DISPATCH(NoOp)
  }

#undef DISPATCH

  Cmd.notifyCompletion(ExitStatus != CPUCommand::NoError);
}

int CPUThread::Execute(ReadBufferCPUCommand &Cmd) {
  std::memcpy(Cmd.GetTarget(), Cmd.GetSource(), Cmd.GetSize());

  return CPUCommand::NoError;
}

int CPUThread::Execute(WriteBufferCPUCommand &Cmd) {
  std::memcpy(Cmd.GetTarget(), Cmd.GetSource(), Cmd.GetSize());

  return CPUCommand::NoError;
}

int CPUThread::Execute(CopyBufferCPUCommand &Cmd) {
  std::memcpy(Cmd.GetTarget(), Cmd.GetSource(), Cmd.GetSize());
  
  return CPUCommand::NoError;
}

int CPUThread::Execute(ReadImageCPUCommand &Cmd) {
  MemRectCpy(Cmd.GetTarget(),
             Cmd.GetSource(),
             Cmd.GetRegion(),
             Cmd.GetTargetRowPitch(),
             Cmd.GetTargetSlicePitch(),
             Cmd.GetSourceRowPitch(),
             Cmd.GetSourceSlicePitch());

  return CPUCommand::NoError;
}

int CPUThread::Execute(WriteImageCPUCommand &Cmd) {
  MemRectCpy(Cmd.GetTarget(),
             Cmd.GetSource(),
             Cmd.GetRegion(),
             Cmd.GetTargetRowPitch(),
             Cmd.GetTargetSlicePitch(),
             Cmd.GetSourceRowPitch(),
             Cmd.GetSourceSlicePitch());

  return CPUCommand::NoError;
}

int CPUThread::Execute(CopyImageCPUCommand &Cmd) {
  MemRectCpy(Cmd.GetTarget(),
             Cmd.GetSource(),
             Cmd.GetRegion(),
             Cmd.GetTargetRowPitch(),
             Cmd.GetTargetSlicePitch(),
             Cmd.GetSourceRowPitch(),
             Cmd.GetSourceSlicePitch());

  return CPUCommand::NoError;
}

int CPUThread::Execute(CopyImageToBufferCPUCommand &Cmd) {
  MemRectCpy(Cmd.GetTarget(),
             Cmd.GetSource(),
             Cmd.GetRegion(),
             Cmd.GetRegion()[0],
             Cmd.GetRegion()[0] * Cmd.GetRegion()[1],
             Cmd.GetSourceRowPitch(),
             Cmd.GetSourceSlicePitch());

  return CPUCommand::NoError;
}

int CPUThread::Execute(CopyBufferToImageCPUCommand &Cmd) {
  MemRectCpy(Cmd.GetTarget(),
             Cmd.GetSource(),
             Cmd.GetRegion(),
             Cmd.GetTargetRowPitch(),
             Cmd.GetTargetSlicePitch(),
             Cmd.GetRegion()[0],
             Cmd.GetRegion()[0] * Cmd.GetRegion()[1]);

  return CPUCommand::NoError;
}

int CPUThread::Execute(MapBufferCPUCommand &Cmd) {
  // The CPU device doesn't require any operation; the memory object iself is
  // always located in the same physical address space of device memory and
  // so it can be accessed directly.
   return CPUCommand::NoError;
}

int CPUThread::Execute(MapImageCPUCommand &Cmd) {
  // The same considerations as in the previous method.
  return CPUCommand::NoError;
}

int CPUThread::Execute(UnmapMemObjectCPUCommand &Cmd) {
  EnqueueUnmapMemObject &CmdUnmap = Cmd.GetQueueCommandAs<EnqueueUnmapMemObject>();
  MemoryObject &MemObj = CmdUnmap.GetMemObj();

  // FIXME: this should be device-independent!
  MemObj.removeMapping(Cmd.GetMappedPtr());
  
  return CPUCommand::NoError;
}

int CPUThread::Execute(ReadBufferRectCPUCommand &Cmd) {
  MemRectCpy(Cmd.GetTarget(),
             Cmd.GetSource(),
             Cmd.GetRegion(),
             Cmd.GetTargetRowPitch(),
             Cmd.GetTargetSlicePitch(),
             Cmd.GetSourceRowPitch(),
             Cmd.GetSourceSlicePitch());
             
  return CPUCommand::NoError;
}

int CPUThread::Execute(WriteBufferRectCPUCommand &Cmd) {
  MemRectCpy(Cmd.GetTarget(),
             Cmd.GetSource(),
             Cmd.GetRegion(),
             Cmd.GetTargetRowPitch(),
             Cmd.GetTargetSlicePitch(),
             Cmd.GetSourceRowPitch(),
             Cmd.GetSourceSlicePitch());
             
  return CPUCommand::NoError;
}

int CPUThread::Execute(CopyBufferRectCPUCommand &Cmd) {
  MemRectCpy(Cmd.GetTarget(),
             Cmd.GetSource(),
             Cmd.GetRegion(),
             Cmd.GetTargetRowPitch(),
             Cmd.GetTargetSlicePitch(),
             Cmd.GetSourceRowPitch(),
             Cmd.GetSourceSlicePitch());
             
  return CPUCommand::NoError;
}

int CPUThread::Execute(FillBufferCPUCommand &Cmd) {
  size_t NumEls = Cmd.GetTargetSize() / Cmd.GetSourceSize();
  
  switch(Cmd.GetSourceSize()) {
  case 1: 
    MemFill<cl_uchar>(Cmd.GetTarget(), Cmd.GetSource(), NumEls);
    break;
  case 2:
    MemFill<cl_ushort>(Cmd.GetTarget(), Cmd.GetSource(), NumEls);
    break;
  case 4:
    MemFill<cl_uint>(Cmd.GetTarget(), Cmd.GetSource(), NumEls);
    break;
  case 8:
    MemFill<cl_ulong>(Cmd.GetTarget(), Cmd.GetSource(), NumEls);
    break;
  case 16:
    MemFill<cl_float4>(Cmd.GetTarget(), Cmd.GetSource(), NumEls); 
    break;
  case 32:
    MemFill<cl_float8>(Cmd.GetTarget(), Cmd.GetSource(), NumEls); 
    break;
  case 64:
    MemFill<cl_float16>(Cmd.GetTarget(), Cmd.GetSource(), NumEls); 
    break;
  case 128:
    MemFill<cl_double16>(Cmd.GetTarget(), Cmd.GetSource(), NumEls); 
    break;
  }
  
  return CPUCommand::NoError;
}

int CPUThread::Execute(FillImageCPUCommand &Cmd) {
  // Maximum fill data size is 4 channels with 4 bytes
  // per channel.
  union {
    cl_uchar    uc[16];
    cl_char     c[16];
    cl_ushort   us[8];
    cl_short    s[8];
    cl_uint     ui[4];
    cl_int      i[4];
    cl_half     h[8];
    cl_float    f[4];
  } fill_data;

  cl_image_format ImgFmt = Cmd.GetTargetImageFormat();
  const size_t *Region = Cmd.GetTargetRegion(); 

  std::memset(&fill_data, 0, 16 * sizeof(cl_uchar));

  float r, g, b;

  switch(ImgFmt.image_channel_order) {
  case CL_R:
    WriteChannel(&fill_data, Cmd.GetSource(), 0, 0, ImgFmt);
    break;
  case CL_A:
    WriteChannel(&fill_data, Cmd.GetSource(), 0, 3, ImgFmt);
    break;
  case CL_Rx:
    WriteChannel(&fill_data, Cmd.GetSource(), 0, 0, ImgFmt);
    break;
  case CL_RG:
  case CL_RGx:
    WriteChannel(&fill_data, Cmd.GetSource(), 0, 0, ImgFmt);
    WriteChannel(&fill_data, Cmd.GetSource(), 1, 1, ImgFmt);
    break;
  case CL_RA:
    WriteChannel(&fill_data, Cmd.GetSource(), 0, 0, ImgFmt);
    WriteChannel(&fill_data, Cmd.GetSource(), 1, 3, ImgFmt);
    break;
  case CL_RGB:
  case CL_RGBx:
    r = std::min(1.0f, std::max(0.0f, ((const float*)Cmd.GetSource())[0]));
    g = std::min(1.0f, std::max(0.0f, ((const float*)Cmd.GetSource())[1]));
    b = std::min(1.0f, std::max(0.0f, ((const float*)Cmd.GetSource())[2]));

    switch(ImgFmt.image_channel_data_type) {
    case CL_UNORM_SHORT_565:
      *(fill_data.us) =
        ((cl_uint)(r * 31.0f) << 11) | ((cl_uint)(g * 63.0f) << 5) | (cl_uint)(b * 31.0f);
      break;
    case CL_UNORM_SHORT_555:
      *(fill_data.us) =
        ((cl_uint)(r * 31.0f) << 10) | ((cl_uint)(g * 31.0f) << 5) | (cl_uint)(b * 31.0f);
      break;
    case CL_UNORM_INT_101010:
      *(fill_data.ui) =
        ((cl_uint)(r * 1023.0f) << 20) | ((cl_uint)(g * 1023.0f) << 10) | (cl_uint)(b * 1023.0f);
      break;
    }
    break;
  case CL_RGBA:
    WriteChannel(&fill_data, Cmd.GetSource(), 0, 0, ImgFmt);
    WriteChannel(&fill_data, Cmd.GetSource(), 1, 1, ImgFmt);
    WriteChannel(&fill_data, Cmd.GetSource(), 2, 2, ImgFmt);
    WriteChannel(&fill_data, Cmd.GetSource(), 3, 3, ImgFmt);
    break;
  case CL_ARGB:
    WriteChannel(&fill_data, Cmd.GetSource(), 0, 3, ImgFmt);
    WriteChannel(&fill_data, Cmd.GetSource(), 1, 0, ImgFmt);
    WriteChannel(&fill_data, Cmd.GetSource(), 2, 1, ImgFmt);
    WriteChannel(&fill_data, Cmd.GetSource(), 3, 2, ImgFmt);
    break;
  case CL_BGRA:
    WriteChannel(&fill_data, Cmd.GetSource(), 0, 2, ImgFmt);
    WriteChannel(&fill_data, Cmd.GetSource(), 1, 1, ImgFmt);
    WriteChannel(&fill_data, Cmd.GetSource(), 2, 0, ImgFmt);
    WriteChannel(&fill_data, Cmd.GetSource(), 3, 3, ImgFmt);
    break;
  case CL_INTENSITY:
  case CL_LUMINANCE:
    // Only normalized data types are possible for these two
    // image channel layouts, so data source is composed by
    // 4 floating point values.
    cl_float Y = ((const float*)Cmd.GetSource())[0] * 0.299f +
                 ((const float*)Cmd.GetSource())[1] * 0.587f +
                 ((const float*)Cmd.GetSource())[2] * 0.114f;

    switch(ImgFmt.image_channel_data_type) {
    case CL_UNORM_INT8:
      *(fill_data.uc) = (cl_uchar)(Y * 255.0f);
      break;
    case CL_UNORM_INT16:
      *(fill_data.us) = (cl_ushort)(Y * 65535.0f);
      break;
    case CL_SNORM_INT8:
      *(fill_data.c) = (cl_char)(Y * 127.0f);
      break;
    case CL_SNORM_INT16:
      *(fill_data.s) = (cl_short)(Y * 32767.0f);
      break;
    case CL_HALF_FLOAT:
      *(fill_data.h) = FloatToHalf(Y);
      break;
    case CL_FLOAT:
      *(fill_data.f) = Y;
      break;
    }

    break;
  }

  switch(Cmd.GetTargetElementSize()) {
  case 1:
    MemRectFill<cl_uchar>(Cmd.GetTarget(),
                          &fill_data,
                          Region,
                          Cmd.GetTargetRowPitch(),
                          Cmd.GetTargetSlicePitch());
    break;
  case 2:
     MemRectFill<cl_ushort>(Cmd.GetTarget(),
                            &fill_data,
                            Region,
                            Cmd.GetTargetRowPitch(),
                            Cmd.GetTargetSlicePitch());
     break;
  case 4:
     MemRectFill<cl_uint>(Cmd.GetTarget(),
                          &fill_data,
                          Region,
                          Cmd.GetTargetRowPitch(),
                          Cmd.GetTargetSlicePitch());
     break;
  case 8:
     MemRectFill<cl_ulong>(Cmd.GetTarget(),
                           &fill_data,
                           Region,
                           Cmd.GetTargetRowPitch(),
                           Cmd.GetTargetSlicePitch());
     break;
  case 16:
     MemRectFill<cl_uint4>(Cmd.GetTarget(),
                           &fill_data,
                           Region,
                           Cmd.GetTargetRowPitch(),
                           Cmd.GetTargetSlicePitch());
     break;
  }

  return CPUCommand::NoError;
}

std::unique_ptr<void*[]>
CPUKernelArguments::prepareArgumentsBuffer(LocalMemory &LM) const {
  LM.Reset(AutoLocalsSize);

  bool WithAutoLocals = AutoLocalsSize > 0;
  auto ArgsBuffer = llvm::make_unique<void*[]>(Args.size() + WithAutoLocals);

  memcpy(ArgsBuffer.get(), Args.data(), Args.size() * sizeof(void*));

  for (const auto &P : LocalBuffers)
    ArgsBuffer[P.first] = LM.Alloc(P.second);

  if (WithAutoLocals)
    ArgsBuffer[Args.size()] = LM.GetStaticPtr();

  return ArgsBuffer;
}

int CPUThread::Execute(NDRangeKernelBlockCPUCommand &Cmd) {
  // Prepare arguments allocating local buffers.
  auto Args = Cmd.prepareArgumentsBuffer(Local);

  // Get function and arguments.
  NDRangeKernelBlockCPUCommand::Signature Func = Cmd.GetFunction();

  // Entry point not available, error!
  if(!Func)
    return CPUCommand::InvalidExecutable;

  // Save first and last work item to execute.
  Begin = Cmd.index_begin();
  End = Cmd.index_end();

  // Get the iteration space.
  DimensionInfo &DimInfo = Cmd.GetDimensionInfo();

  // Reset the stack and set initial work-item.
  Stack.Reset(Func, Args.get(), DimInfo.GetLocalWorkItems());
  Cur = Begin;

  // Start the first work-item.
  Stack.Run();

  return CPUCommand::NoError;
}

int CPUThread::Execute(NativeKernelCPUCommand &Cmd) {
  NativeKernelCPUCommand::Signature Func = Cmd.GetFunction();
  void *Args = Cmd.GetArgumentsPointer();

  Func(Args);

  return CPUCommand::NoError;
}

int CPUThread::Execute(NoOpCPUCommand &Cmd) {
  return CPUCommand::NoError;
}

void CPUThread::MemRectCpy(void *Target, 
                           const void *Source,
                           const size_t *Region,
                           size_t TargetRowPitch,
                           size_t TargetSlicePitch,
                           size_t SourceRowPitch,
                           size_t SourceSlicePitch) {
  for(size_t Z = 0; Z < Region[2]; ++Z)
    for(size_t Y = 0; Y < Region[1]; ++Y)
      std::memcpy(reinterpret_cast<void *>(
                    reinterpret_cast<uintptr_t>(Target) + TargetRowPitch * Y + TargetSlicePitch * Z
                  ),
                  reinterpret_cast<const void *>(
                    reinterpret_cast<uintptr_t>(Source) + SourceRowPitch * Y + SourceSlicePitch * Z
                  ),
                  Region[0]);                               
}

void CPUThread::WriteChannel(void *DataOut,
                             const void *DataIn,
                             size_t ToCh,
                             size_t FromCh,
                             const cl_image_format &ImgFmt) {
  switch(ImgFmt.image_channel_data_type) {
  case CL_SNORM_INT8:
    ((cl_char *)DataOut)[ToCh] =
      (cl_char)(std::min(1.0f, std::max(0.0f, ((const cl_float *)DataIn)[FromCh])) * 127.0f);
    break;
  case CL_SNORM_INT16:
    ((cl_short *)DataOut)[ToCh] =
      (cl_short)(std::min(1.0f, std::max(0.0f, ((const cl_float *)DataIn)[FromCh])) * 32767.0f);
    break;    
  case CL_UNORM_INT8:
    ((cl_uchar *)DataOut)[ToCh] =
      (cl_uchar)(std::min(1.0f, std::max(0.0f, ((const cl_float *)DataIn)[FromCh])) * 255.0f);
    break;
  case CL_UNORM_INT16:
    ((cl_ushort *)DataOut)[ToCh] =
      (cl_ushort)(std::min(1.0f, std::max(0.0f, ((const cl_float *)DataIn)[FromCh])) * 65535.0f);
    break;
  case CL_SIGNED_INT8:
    ((cl_char *)DataOut)[ToCh] = (cl_char)(((const cl_int *)DataIn)[FromCh]);
    break;
  case CL_SIGNED_INT16:
    ((cl_short *)DataOut)[ToCh] = (cl_short)(((const cl_int *)DataIn)[FromCh]);
    break;
  case CL_SIGNED_INT32:
    ((cl_int *)DataOut)[ToCh] = ((const cl_int *)DataIn)[FromCh];
    break;
  case CL_UNSIGNED_INT8:
    ((cl_uchar *)DataOut)[ToCh] = (cl_uchar)(((const cl_uint *)DataIn)[FromCh]);
    break;
  case CL_UNSIGNED_INT16:
    ((cl_ushort*)DataOut)[ToCh] = (cl_ushort)(((const cl_uint *)DataIn)[FromCh]);
    break;
  case CL_UNSIGNED_INT32:
    ((cl_uint *)DataOut)[ToCh] = ((const cl_uint *)DataIn)[FromCh];
    break;
  case CL_HALF_FLOAT:
    ((cl_half *)DataOut)[ToCh] = FloatToHalf(((const cl_float *)DataIn)[FromCh]); 
    break;
  case CL_FLOAT:
    ((cl_float *)DataOut)[ToCh] = ((const cl_float *)DataIn)[FromCh];
    break;
  }
}

cl_half CPUThread::FloatToHalf(cl_float f) {
  // Single-precision float as a 32 bit unsigned integer.
  cl_uint fp32 = *((cl_float *)&f);

  // Half-precision float as a 16 bit unsigned integer.
  cl_ushort fp16 = 0;

  // Round or not.
  bool round = false;

  // Single-precision float components (Sign, Exponent, Significand)
  // as 32 bit unsigned integers.
  cl_uint fp32_s = fp32 & 0x80000000U;
  cl_uint fp32_e = fp32 & 0x7f800000U;
  cl_uint fp32_m = fp32 & 0x007fffffU;

  // Half-precision float components (Sign, Exponent, Significand)
  // as 16 bit unsigned integers.
  cl_ushort fp16_s = fp32_s >> 16;
  cl_ushort fp16_e = 0;
  cl_ushort fp16_m = fp32_m >> 13;

  // Re-bias the exponent for the excess-15 binary representation.
  cl_int exp = (fp32_e >> 23) - 127 + 15;

  if(exp < -9) { // {-inf .. -10} : Excessive underflow.
    // Return +/- zero keeping only the original
    // sign.
    fp16_m = 0;  
  } else if(exp < 1) { // {-9 .. 0} : Underflow.
    // Denormalization.
    cl_uint m = fp32_m | 0x00800000U; 
    fp16_m = m >> (14 - exp);
    // If the MSB lost by denormalization was 1 we round up
    // the result.
    if((m >> (13 - exp)) & 1)
      round = true;
  } if(exp >= 31) { // {31 .. +inf} : Inf, NaN or overflow.
    // Nan -> QNaN
    // +/-Inf -> +/- Inf
    fp16_e = 0x001fU << 10;
    fp16_m = fp32_m ? 0x0200U : 0; 
  } else { // {1 .. 30} : Normal.
    fp16_e = (cl_ushort)(exp << 10);
    // If the MSB lost by shifting was 1 we round up.
    if(fp32_m & 0x00001000U)
      round = true;
  }

  fp16 = (cl_half)(fp16_s | fp16_e | fp16_m);

  if(round) fp16++; // May overflow into exponent.

  return fp16;
}

cl_float CPUThread::HalfToFloat(cl_half h) {
  // Half-precision float as a 16 bit unsigned integer.
  cl_ushort fp16 = *((cl_ushort *)&h);

  // Single-precision float as a 32 bit unsigned integer.
  cl_uint fp32 = 0;

  // Half-precision float components (Sign, Exponent, Significand)
  // as 16 bit unsigned integers.
  cl_ushort fp16_s = fp16 & 0x8000U;
  cl_ushort fp16_e = fp16 & 0x7c00U;
  cl_ushort fp16_m = fp16 & 0x03ffU;

  // Single-precision float components (Sign, Exponent, Significand)
  // as 32 bit unsigned integers.
  cl_uint fp32_s = fp16_s << 16;
  cl_uint fp32_e = 0;
  cl_uint fp32_m = fp16_m << 13;

  // Unsigned value for f16_e.
  cl_ushort exp = fp16_e >> 10;

  if(exp == 31) // Inf, NaN
    fp32_e = 255 << 23;
  else if(exp == 0) {
    if(fp16_m != 0) { // Denormal.
      cl_int e = -1;
      cl_uint m = fp16_m;

      do {
        ++e;
        m <<= 1;
      } while((m & 0x400U) == 0);

      fp32_m = (m & 0x3ffU) << 13;
      fp32_e = (127 - 15 - e) << 23;
    }
  } else // Normal.
    // Apply new bias and shift to correct position.
    fp32_e = ((cl_uint)exp - 15 + 127) << 23;

  // +/- 0 are handled here.
  fp32 = (cl_float)(fp32_s | fp32_e | fp32_m);

  return fp32;
}

//
// GetCurrentThread implementation.
//

namespace { // Anonymous namespace.

// Very architecture-dependent code. Use __thread instead of
// llvm::sys::ThreadLocal in order to generate more compact code.
// This thread-local variable is declared as static, thus it's
// used within this module only and the compiler generated code
// should use the local-dynamic TLS access model.
// 
// See <https://www.akkadia.org/drepper/tls.pdf>
static __thread const CPUThread *CurThread;

static void SetCurrentThread(CPUThread *Thr) { CurThread = Thr; }
static void ResetCurrentThread() { CurThread = nullptr; }

} // End anonymous namespace.

CPUThread &opencrun::cpu::GetCurrentThread() {
  return *const_cast<CPUThread *>(CurThread);
}
