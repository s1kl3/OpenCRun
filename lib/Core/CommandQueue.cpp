
#include "opencrun/Core/CommandQueue.h"
#include "opencrun/Core/Device.h"
#include "opencrun/Core/Command.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/Event.h"

using namespace opencrun;

//
// CommandQueue implementation.
//

#define RETURN_WITH_ERROR(VAR, ERRCODE, MSG) \
  {                                          \
  Ctx->ReportDiagnostic(MSG);                \
  if(VAR)                                    \
    *VAR = ERRCODE;                          \
  return NULL;                               \
  }

CommandQueue::CommandQueue(Type Ty, Context &Ctx, Device &Dev, bool EnableProfile) :
  Ty(Ty),
  Ctx(&Ctx),
  Dev(Dev),
  EnableProfile(EnableProfile) { 
    // If the associated device is a sub-device do an implicit retain.
    if(Dev.IsSubDevice())
        Dev.Retain();
  }

CommandQueue::~CommandQueue() { 
  // If the associated device is a sub-device do an implicit release.
  if(Dev.IsSubDevice())
      Dev.Release();
}

llvm::IntrusiveRefCntPtr<Event>
CommandQueue::Enqueue(Command &Cmd, cl_int *ErrCode) {
  for(Command::event_iterator I = Cmd.wait_begin(), E = Cmd.wait_end();
                              I != E;
                              ++I)
    if((*I)->GetContext() != *Ctx)
      RETURN_WITH_ERROR(ErrCode,
                        CL_INVALID_CONTEXT,
                        "cannot wait for events of a different context");

  if(llvm::isa<EnqueueNativeKernel>(&Cmd) && !Dev.SupportsNativeKernels())
    RETURN_WITH_ERROR(ErrCode,
                      CL_INVALID_OPERATION,
                      "device does not support native kernels");

  llvm::IntrusiveRefCntPtr<InternalEvent> Ev;
  {
    llvm::sys::ScopedLock Lock(ThisLock);

    unsigned Cnts = EnableProfile ? Profiler::Time : Profiler::None;
    ProfileSample *Sample = GetProfilerSample(Cnts,
                                              ProfileSample::CommandEnqueued);
    Ev = new InternalEvent(*this, Cmd.GetType(), Sample);

    Cmd.SetNotifyEvent(Ev.get());
    Commands.push_back(std::unique_ptr<Command>(&Cmd));
  }

  // TODO: This should be avoided, the queue should notify the device that at
  // least one command is available so that it can start the processing.
  RunScheduler();

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  if(Cmd.IsBlocking())
    Ev->Wait();

  return Ev;
}

void CommandQueue::CommandDone(Command &Cmd) {
  RunScheduler();

  {
    llvm::sys::ScopedLock Lock(ThisLock);
    PendingCommands.erase(&Cmd);
  }

  delete &Cmd;
}

void CommandQueue::Flush() {
  while(RunScheduler()) { }
}

void CommandQueue::Finish() {
  Flush();

  typedef llvm::IntrusiveRefCntPtr<InternalEvent> InternalEventRefPtr;
  typedef llvm::SmallVector<InternalEventRefPtr, 32> EventsContainer;

  EventsContainer WaitList;

  // Safely copy events to wait in a new container. Reference counting is
  // incremented, because current thread can not be the same who has enqueued
  // the commands.
  {
    llvm::sys::ScopedLock Lock(ThisLock);
    for (auto Cmd : PendingCommands)
      WaitList.push_back(&Cmd->GetNotifyEvent());
    for (const auto &Cmd : Commands)
      WaitList.push_back(&Cmd->GetNotifyEvent());
  }

  // Wait for all events. If an event was linked to a command that is terminated
  // after leaving the critical section, the reference counting mechanism had
  // prevented the runtime to delete the memory associated with the event: we
  // can safely use the event to wait for a already terminated command.
  for (const auto &Ev : WaitList)
    Ev->Wait();
}

bool CommandQueue::ensureMemoryCoherency(Command &Cmd) {
  using SM = MemoryObject::SynchronizeMode;

  switch (Cmd.GetType()) {
  default: return true;
  case Command::ReadBuffer: {
    auto &TheCmd = *llvm::cast<EnqueueReadBuffer>(&Cmd);
    TheCmd.GetSource().synchronizeFor(GetDevice(), SM::SyncRead);
    break;
  }
  case Command::WriteBuffer: {
    auto &TheCmd = *llvm::cast<EnqueueWriteBuffer>(&Cmd);
    TheCmd.GetTarget().synchronizeFor(GetDevice(), SM::SyncReadWrite);
    break;
  }
  case Command::CopyBuffer: {
    auto &TheCmd = *llvm::cast<EnqueueCopyBuffer>(&Cmd);
    TheCmd.GetSource().synchronizeFor(GetDevice(), SM::SyncRead);
    TheCmd.GetTarget().synchronizeFor(GetDevice(), SM::SyncReadWrite);
    break;
  }
  case Command::ReadImage: {
    auto &TheCmd = *llvm::cast<EnqueueReadImage>(&Cmd);
    TheCmd.GetSource().synchronizeFor(GetDevice(), SM::SyncRead);
    break;
  }
  case Command::WriteImage: {
    auto &TheCmd = *llvm::cast<EnqueueWriteImage>(&Cmd);
    TheCmd.GetTarget().synchronizeFor(GetDevice(), SM::SyncReadWrite);
    break;
  }
  case Command::CopyImage: {
    auto &TheCmd = *llvm::cast<EnqueueCopyImage>(&Cmd);
    TheCmd.GetSource().synchronizeFor(GetDevice(), SM::SyncRead);
    TheCmd.GetTarget().synchronizeFor(GetDevice(), SM::SyncReadWrite);
    break;
  }
  case Command::CopyImageToBuffer: {
    auto &TheCmd = *llvm::cast<EnqueueCopyImageToBuffer>(&Cmd);
    TheCmd.GetSource().synchronizeFor(GetDevice(), SM::SyncRead);
    TheCmd.GetTarget().synchronizeFor(GetDevice(), SM::SyncReadWrite);
    break;
  }
  case Command::CopyBufferToImage: {
    auto &TheCmd = *llvm::cast<EnqueueCopyBufferToImage>(&Cmd);
    TheCmd.GetSource().synchronizeFor(GetDevice(), SM::SyncRead);
    TheCmd.GetTarget().synchronizeFor(GetDevice(), SM::SyncReadWrite);
    break;
  }
  case Command::MapBuffer: {
    auto &TheCmd = *llvm::cast<EnqueueMapBuffer>(&Cmd);
    unsigned Mode = SM::SyncMap;
    if (TheCmd.GetMapFlags() & CL_MAP_READ)
      Mode |= SM::SyncRead;
    if (TheCmd.GetMapFlags() & CL_MAP_WRITE)
      Mode |= SM::SyncWrite;
    TheCmd.GetSource().synchronizeFor(GetDevice(), Mode);
    break;
  }
  case Command::MapImage: {
    auto &TheCmd = *llvm::cast<EnqueueMapImage>(&Cmd);
    unsigned Mode = SM::SyncMap;
    if (TheCmd.GetMapFlags() & CL_MAP_READ)
      Mode |= SM::SyncRead;
    if (TheCmd.GetMapFlags() & CL_MAP_WRITE)
      Mode |= SM::SyncWrite;
    TheCmd.GetSource().synchronizeFor(GetDevice(), Mode);
    break;
  }
  case Command::UnmapMemObject: {
    auto &TheCmd = *llvm::cast<EnqueueUnmapMemObject>(&Cmd);
    TheCmd.GetMemObj().synchronizeFor(GetDevice(), SM::SyncRead);
    break;
  }
  case Command::ReadBufferRect: {
    auto &TheCmd = *llvm::cast<EnqueueReadBufferRect>(&Cmd);
    TheCmd.GetSource().synchronizeFor(GetDevice(), SM::SyncRead);
    break;
  }
  case Command::WriteBufferRect: {
    auto &TheCmd = *llvm::cast<EnqueueWriteBufferRect>(&Cmd);
    TheCmd.GetTarget().synchronizeFor(GetDevice(), SM::SyncReadWrite);
    break;
  }
  case Command::CopyBufferRect: {
    auto &TheCmd = *llvm::cast<EnqueueCopyBufferRect>(&Cmd);
    TheCmd.GetSource().synchronizeFor(GetDevice(), SM::SyncRead);
    TheCmd.GetTarget().synchronizeFor(GetDevice(), SM::SyncReadWrite);
    break;
  }
  case Command::FillBuffer: {
    auto &TheCmd = *llvm::cast<EnqueueFillBuffer>(&Cmd);
    TheCmd.GetTarget().synchronizeFor(GetDevice(), SM::SyncReadWrite);
    break;
  }
  case Command::FillImage: {
    auto &TheCmd = *llvm::cast<EnqueueFillImage>(&Cmd);
    TheCmd.GetTarget().synchronizeFor(GetDevice(), SM::SyncReadWrite);
    break;
  }
  case Command::NDRangeKernel: {
    auto &TheCmd = *llvm::cast<EnqueueNDRangeKernel>(&Cmd);
    auto &Kern = TheCmd.GetKernel();
    for (auto I = Kern.arg_begin(), E = Kern.arg_end(); I != E; ++I) {
      MemoryObject *Obj = nullptr;
      if (I->getKind() == KernelArg::BufferArg)
        Obj = I->getBuffer();
      else if (I->getKind() == KernelArg::ImageArg)
        Obj = I->getImage();

      if (Obj) {
        unsigned Mode = 0;
        if (Obj->getFlags() & MemoryObject::ReadWrite)
          Mode = SM::SyncReadWrite;
        else if (Obj->getFlags() & MemoryObject::ReadOnly)
          Mode = SM::SyncRead;
        else if (Obj->getFlags() & MemoryObject::WriteOnly)
          Mode = SM::SyncWrite;

        Obj->synchronizeFor(GetDevice(), Mode);
      }
    }
    break;
  }
  case Command::NativeKernel: {
    auto &TheCmd = *llvm::cast<EnqueueNativeKernel>(&Cmd);
    for (const auto &Loc : TheCmd.GetMemoryLocations()) {
      unsigned Mode = 0;
      if (Loc.second->getFlags() & MemoryObject::ReadWrite)
        Mode = SM::SyncReadWrite;
      else if (Loc.second->getFlags() & MemoryObject::ReadOnly)
        Mode = SM::SyncRead;
      else if (Loc.second->getFlags() & MemoryObject::WriteOnly)
        Mode = SM::SyncWrite;

      Loc.second->synchronizeFor(GetDevice(), Mode);
    }
    break;
  }
  }
  return true;
}


bool CommandQueue::submit(Command &Cmd) {
  if (!ensureMemoryCoherency(Cmd))
    return false;

  return Dev.Submit(Cmd);
}

//
// OutOfOrderQueue implementation.
//

OutOfOrderQueue::~OutOfOrderQueue() {
  Finish(); // Performs virtual calls, must be called here.
}

bool OutOfOrderQueue::RunScheduler() {
  return false;
}

void OutOfOrderQueue::CommandDone(Command &Cmd) { }

//
// InOrderQueue implementation.
//

InOrderQueue::~InOrderQueue() {
  Finish(); // Performs virtual calls, must be called here.
}

bool InOrderQueue::RunScheduler() {
  llvm::sys::ScopedLock Lock(ThisLock);

  if (Commands.empty() || CommandOnFly)
    return false;

  if (!Commands.front()->CanRun())
    return true;

  std::unique_ptr<Command> Cmd = std::move(Commands.front());

  if (!submit(*Cmd)) {
    Commands.front() = std::move(Cmd);
    return true;
  }

  PendingCommands.insert(Cmd.release());
  Commands.pop_front();
  CommandOnFly = true;

  return Commands.size();
}

void InOrderQueue::CommandDone(Command &Cmd) {
  {
    llvm::sys::ScopedLock Lock(ThisLock);
    CommandOnFly = false;
  }

  CommandQueue::CommandDone(Cmd);
}
