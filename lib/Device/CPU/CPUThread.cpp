
#include "opencrun/Device/CPU/CPUThread.h"
#include "opencrun/Core/Event.h"
#include "opencrun/Device/CPU/Multiprocessor.h"
#include "opencrun/System/OS.h"

#include "llvm/Support/Format.h"
#include "llvm/Support/ThreadLocal.h"

using namespace opencrun;
using namespace opencrun::cpu;

namespace {

//
// Routines to manage current work-item stack pointer in the TLS.
//

extern "C" {

void SetCurrentWorkItemStack(void *Stack);
void *GetCurrentWorkItemStack();

}

//
// Routines to manage current thread pointer in the TLS.
//

void SetCurrentThread(CPUThread &Thr);
void ResetCurrentThread();

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
  sys::Free(Stack);
}

void ExecutionStack::Reset(EntryPoint Entry, void **Args, unsigned N) {
  size_t PageSize = sys::Hardware::GetPageSize(),
         PrivateSize = PageSize,
         RequiredStackSize = N * PrivateSize;

  // Stack too small, expand it.
  if(StackSize < RequiredStackSize) {
    sys::Free(Stack);
    Stack = sys::PageAlignedAlloc(RequiredStackSize);
    StackSize = RequiredStackSize;
  }

  #ifndef NDEBUG
  // Add guard pages between work-item stacks. Add a guard page at the end of
  // the stack.
  size_t DebugPrivateSize = PrivateSize + PageSize;
  size_t DebugStackSize = N * DebugPrivateSize + PageSize;

  // If needed, redo memory allocation, in order to get space for guard page.
  if(StackSize < DebugStackSize) {
    sys::Free(Stack);
    Stack = sys::PageAlignedAlloc(DebugStackSize);
    StackSize = DebugStackSize;
  }

  // Remove page protection, reset all pages to R/W.
  sys::MarkPagesReadWrite(Stack, StackSize);

  // Zero the memory.
  std::memset(Stack, 0, StackSize);
  #endif // NDEBUG

  uintptr_t StackAddr = reinterpret_cast<uintptr_t>(Stack),
            Addr = StackAddr,
            PrevAddr = 0;

  // Skip first page.
  #ifndef NDEBUG
  Addr += PageSize;
  #endif // NDEBUG

  // Initialize all work-item stacks.
  for(unsigned I = 0; I < N; ++I) {
    // Mark the guard page.
    #ifndef NDEBUG
    sys::MarkPageReadOnly(reinterpret_cast<void *>(Addr - PageSize));
    #endif // NDEBUG

    InitWorkItemStack(reinterpret_cast<void *>(Addr), PrivateSize, Entry, Args);

    if(PrevAddr)
      SetWorkItemStackLink(reinterpret_cast<void *>(Addr),
                           reinterpret_cast<void *>(PrevAddr),
                           PrivateSize);

    PrevAddr = Addr;

    #ifndef NDEBUG
    Addr += DebugPrivateSize;
    #else
    Addr += PrivateSize;
    #endif // NDEBUG
  }

  // Mark last guard page.
  #ifndef NDEBUG
  sys::MarkPageReadOnly(reinterpret_cast<void *>(Addr - PageSize));
  #endif // NDEBUG

  Addr = StackAddr;

  // Skip first page.
  #ifndef NDEBUG
  Addr += PageSize;
  #endif // NDEBUG

  // Link last stack with the first.
  SetWorkItemStackLink(reinterpret_cast<void *>(Addr),
                       reinterpret_cast<void *>(PrevAddr),
                       PrivateSize);
}

void ExecutionStack::Run() {
  uintptr_t StackAddr = reinterpret_cast<uintptr_t>(Stack);
  size_t PageSize = sys::Hardware::GetPageSize();

  // Skip first page.
  #ifndef NDEBUG
  StackAddr += PageSize;
  #endif // NDEBUG

  RunWorkItems(reinterpret_cast<void *>(StackAddr), PageSize);
}

void ExecutionStack::SwitchToNextWorkItem() {
  SwitchWorkItem();
}

void ExecutionStack::Dump() const {
  llvm::raw_ostream &OS = llvm::errs();

  uintptr_t StartAddr = reinterpret_cast<uintptr_t>(Stack),
            EndAddr = StartAddr + StackSize;
  size_t PageSize = sys::Hardware::GetPageSize();

  OS << llvm::format("Stack [%p, %p]", StartAddr, EndAddr)
     << llvm::format(" => %zd bytes, %u pages", StackSize, StackSize / PageSize)
     << ":";
  Dump(reinterpret_cast<void *>(StartAddr), reinterpret_cast<void *>(EndAddr));
}

void ExecutionStack::Dump(unsigned I) const {
  llvm::raw_ostream &OS = llvm::errs();

  uintptr_t StartAddr = reinterpret_cast<uintptr_t>(Stack),
            EndAddr;
  size_t PageSize = sys::Hardware::GetPageSize();

  #ifndef NDEBUG
  StartAddr += PageSize + I * 2 * PageSize;
  #else
  StartAddr += StartAddr + I * PageSize;
  #endif // NDEBUG
  
  EndAddr = StartAddr + PageSize;

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
// CurrentWorkItemStack implementation.
//

namespace {

extern "C" {

llvm::sys::ThreadLocal<const void> CurrentWorkItemStack;

void SetCurrentWorkItemStack(void *Stack) {
  CurrentWorkItemStack.set(Stack);
}

void *GetCurrentWorkItemStack() {
  return const_cast<void *>(CurrentWorkItemStack.get());
}

}

} // End anonymous namespace.

//
// CPUThread implementation.
//

CPUThread::CPUThread(Multiprocessor &MP, const sys::HardwareCPU &CPU) :
  Thread(CPU),
  Mode(FullyOperational),
  MP(MP),
  Stack(*CPU.GetFirstLevelCache()),
  Local(*CPU.GetFirstLevelCache()) {
  Start();
}

CPUThread::~CPUThread() {
  // Tell worker thread to stop activities.
  CPUCommand *Cmd = new StopDeviceCPUCommand();
  Submit(Cmd);

  // Use Thread::Join to suspend current thread until the joined thread exits.
  Join();
}

bool CPUThread::Submit(CPUCommand *Cmd) {
  bool Enqueued;

  sys::ScopedMonitor Mnt(ThisMnt);

  if(Mode & NoNewJobs)
    return false;

  if(CPUServiceCommand *Enq = llvm::dyn_cast<CPUServiceCommand>(Cmd))
    Enqueued = Submit(Enq);
  else if(CPUExecCommand *Enq = llvm::dyn_cast<CPUExecCommand>(Cmd))
    Enqueued = Submit(Enq);
  else
    Enqueued = false;

  if(Enqueued) {
    Commands.push_back(Cmd);
    Mnt.Signal();
  }

  return Enqueued;
}

void CPUThread::Run() {
  SetCurrentThread(*this);

  while(Mode & ExecJobs) {
    ThisMnt.Enter();

    while(Commands.empty())
      ThisMnt.Wait();

    CPUCommand *Cmd = Commands.front();
    Commands.pop_front();

    ThisMnt.Exit();

    Execute(Cmd);
  }

  // ResetCurrentThread();
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

bool CPUThread::Submit(CPUServiceCommand *Cmd) {
  if(StopDeviceCPUCommand *Enq = llvm::dyn_cast<StopDeviceCPUCommand>(Cmd))
    return Submit(Enq);

  else
    llvm::report_fatal_error("unknown command submitted");

  return false;
}

bool CPUThread::Submit(CPUExecCommand *Cmd) {
  if(ReadBufferCPUCommand *Read = llvm::dyn_cast<ReadBufferCPUCommand>(Cmd))
    return Submit(Read);

  else if(WriteBufferCPUCommand *Write =
            llvm::dyn_cast<WriteBufferCPUCommand>(Cmd))
    return Submit(Write);
  
  else if(CopyBufferCPUCommand *Copy =
            llvm::dyn_cast<CopyBufferCPUCommand>(Cmd))
    return Submit(Copy);

  else if(MapBufferCPUCommand *Map =
            llvm::dyn_cast<MapBufferCPUCommand>(Cmd))
    return Submit(Map);
  
  else if(UnmapMemObjectCPUCommand *Unmap =
            llvm::dyn_cast<UnmapMemObjectCPUCommand>(Cmd))
    return Submit(Unmap);
  
  else if(ReadBufferRectCPUCommand *ReadRect =
            llvm::dyn_cast<ReadBufferRectCPUCommand>(Cmd))
    return Submit(ReadRect);
  
  else if(WriteBufferRectCPUCommand *WriteRect =
            llvm::dyn_cast<WriteBufferRectCPUCommand>(Cmd))
    return Submit(WriteRect);
  
  else if(CopyBufferRectCPUCommand *CopyRect =
            llvm::dyn_cast<CopyBufferRectCPUCommand>(Cmd))
    return Submit(CopyRect);
  
  else if(NDRangeKernelBlockCPUCommand *NDBlock =
            llvm::dyn_cast<NDRangeKernelBlockCPUCommand>(Cmd))
    return Submit(NDBlock);

  else if(NativeKernelCPUCommand *Native =
            llvm::dyn_cast<NativeKernelCPUCommand>(Cmd))
    return Submit(Native);

  else
    llvm::report_fatal_error("unknown command submitted");

  return false;
}

void CPUThread::Execute(CPUCommand *Cmd) {
  if(CPUServiceCommand *OnFly = llvm::dyn_cast<CPUServiceCommand>(Cmd))
    Execute(OnFly);
  else if(CPUExecCommand *OnFly = llvm::dyn_cast<CPUExecCommand>(Cmd))
    Execute(OnFly);
}

void CPUThread::Execute(CPUServiceCommand *Cmd) {
  if(StopDeviceCPUCommand *OnFly =
            llvm::dyn_cast<StopDeviceCPUCommand>(Cmd))
    Execute(OnFly);

  MP.NotifyDone(Cmd);
}

void CPUThread::Execute(CPUExecCommand *Cmd) {
  InternalEvent &Ev = Cmd->GetQueueCommand().GetNotifyEvent();
  unsigned Counters = Cmd->IsProfiled() ? Profiler::Time : Profiler::None;
  int ExitStatus;

  // Command started.
  if(Cmd->RegisterStarted()) {
    ProfileSample *Sample = GetProfilerSample(MP,
                                              Counters,
                                              ProfileSample::CommandRunning);
    Ev.MarkRunning(Sample);
  }

  // This command is part of a large OpenCL command. Register partial execution.
  if(CPUMultiExecCommand *MultiCmd = llvm::dyn_cast<CPUMultiExecCommand>(Cmd)) {
    ProfileSample *Sample = GetProfilerSample(MP,
                                              Counters,
                                              ProfileSample::CommandRunning,
                                              MultiCmd->GetId());
    Ev.MarkSubRunning(Sample);
  }

  if(ReadBufferCPUCommand *OnFly = llvm::dyn_cast<ReadBufferCPUCommand>(Cmd))
    ExitStatus = Execute(*OnFly);

  else if(WriteBufferCPUCommand *OnFly =
            llvm::dyn_cast<WriteBufferCPUCommand>(Cmd))
    ExitStatus = Execute(*OnFly);

  else if(CopyBufferCPUCommand *OnFly =
            llvm::dyn_cast<CopyBufferCPUCommand>(Cmd))
    ExitStatus = Execute(*OnFly);
  
  else if(MapBufferCPUCommand *OnFly =
            llvm::dyn_cast<MapBufferCPUCommand>(Cmd))
    ExitStatus = Execute(*OnFly);
  
  else if(UnmapMemObjectCPUCommand *OnFly =
            llvm::dyn_cast<UnmapMemObjectCPUCommand>(Cmd))
    ExitStatus = Execute(*OnFly);
  
  else if(ReadBufferRectCPUCommand *OnFly =
            llvm::dyn_cast<ReadBufferRectCPUCommand>(Cmd))
    ExitStatus = Execute(*OnFly);

  else if(WriteBufferRectCPUCommand *OnFly =
            llvm::dyn_cast<WriteBufferRectCPUCommand>(Cmd))
    ExitStatus = Execute(*OnFly);
  
  else if(CopyBufferRectCPUCommand *OnFly =
            llvm::dyn_cast<CopyBufferRectCPUCommand>(Cmd))
    ExitStatus = Execute(*OnFly);    
    
  else if(NDRangeKernelBlockCPUCommand *OnFly =
            llvm::dyn_cast<NDRangeKernelBlockCPUCommand>(Cmd))
    ExitStatus = Execute(*OnFly);

  else if(NativeKernelCPUCommand *OnFly =
            llvm::dyn_cast<NativeKernelCPUCommand>(Cmd))
    ExitStatus = Execute(*OnFly);
  
  else
    ExitStatus = CPUCommand::Unsupported;
  
  MP.NotifyDone(Cmd, ExitStatus);
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

int CPUThread::Execute(MapBufferCPUCommand &Cmd) {
  EnqueueMapBuffer &CmdMap = Cmd.GetQueueCommandAs<EnqueueMapBuffer>();
  Buffer &Buf = CmdMap.GetSource();
  
  // When device and host buffer are distinct we need to copy the specified region to
  // the host buffer only for CL_MAP_READ and CL_MAP_WRITE mappings, because we have
  // to guarantee that the host buffer contains the latest bits from the region being
  // mapped; this is not necessary for CL_MAP_WRITE_INVALIDATE_REGION mappings.
  if((llvm::isa<DeviceBuffer>(&Buf) || llvm::isa<HostAccessibleBuffer>(&Buf)) &&
      (CmdMap.IsMapRead() || CmdMap.IsMapWrite()))
    std::memcpy(Cmd.GetTarget(), Cmd.GetSource(), Cmd.GetSize());

  return CPUCommand::NoError;
}

int CPUThread::Execute(UnmapMemObjectCPUCommand &Cmd) {
  EnqueueUnmapMemObject &CmdUnmap = Cmd.GetQueueCommandAs<EnqueueUnmapMemObject>();
  MemoryObj &MemObj = CmdUnmap.GetMemObj();
  
  if(llvm::isa<DeviceBuffer>(&MemObj) || llvm::isa<HostAccessibleBuffer>(&MemObj)) {
    // Copy data back to the memory object if it was mapped with
    // CL_MAP_WRITE_INVALIDATE_REGION
    MemoryObj::MappingInfo *Info = MemObj.GetMappingInfo(Cmd.GetMappedPtr());
    if(Info && (Info->MapFlags & CL_MAP_WRITE_INVALIDATE_REGION))
      memcpy(Cmd.GetMemObjAddr(), Cmd.GetMappedPtr(), Info->Size);
  
    free(Cmd.GetMappedPtr());
  }
  
  MemObj.RemoveMapping(Cmd.GetMappedPtr());
  
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

int CPUThread::Execute(NDRangeKernelBlockCPUCommand &Cmd) {
  // Reserve space for local buffers.
  // TODO: reserve space for local automatic buffers.
  Local.Reset(0);
  Cmd.SetLocalParams(Local);

  // Get function and arguments.
  NDRangeKernelBlockCPUCommand::Signature Func = Cmd.GetFunction();
  void **Args = Cmd.GetArgumentsPointer();

  // Entry point not available, error!
  if(!Func)
    return CPUCommand::InvalidExecutable;

  // Save first and last work item to execute.
  Begin = Cmd.index_begin();
  End = Cmd.index_end();

  // Get the iteration space.
  DimensionInfo &DimInfo = Cmd.GetDimensionInfo();

  // Reset the stack and set initial work-item.
  Stack.Reset(Func, Args, DimInfo.GetLocalWorkItems());
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

// Used by: EnqueueReadBufferRect
//          EnqueueWriteBufferRect
//          EnqueueCopyBufferRect
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

//
// GetCurrentThread implementation.
//

namespace {

llvm::sys::ThreadLocal<const CPUThread> CurThread;

void SetCurrentThread(CPUThread &Thr) { CurThread.set(&Thr); }
void ResetCurrentThread() { CurThread.erase(); }

} // End anonymous namespace.

CPUThread &opencrun::cpu::GetCurrentThread() {
  return *const_cast<CPUThread *>(CurThread.get());
}
