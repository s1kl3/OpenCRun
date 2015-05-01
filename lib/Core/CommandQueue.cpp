
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

  ThisLock.acquire();

  unsigned Cnts = EnableProfile ? Profiler::Time : Profiler::None;
  ProfileSample *Sample = GetProfilerSample(Dev,
                                            Cnts,
                                            ProfileSample::CommandEnqueued);
  llvm::IntrusiveRefCntPtr<InternalEvent> Ev;
  Ev = new InternalEvent(*this, Cmd.GetType(), Sample);

  Cmd.SetNotifyEvent(Ev.getPtr());
  Commands.push_back(std::unique_ptr<Command>(&Cmd));

  ThisLock.release();

  RunScheduler();

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  if(Cmd.IsBlocking())
    Ev->Wait();

  return Ev;
}

void CommandQueue::CommandDone(Command &Cmd) {
  RunScheduler();

  ThisLock.acquire();
  PendingCommands.erase(&Cmd);
  ThisLock.release();

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
  ThisLock.acquire();
  for (auto I = PendingCommands.begin(), E = PendingCommands.end(); I != E; ++I)
    WaitList.push_back(&(*I)->GetNotifyEvent());
  for (auto I = Commands.begin(), E = Commands.end(); I != E; ++I)
    WaitList.push_back(&(*I)->GetNotifyEvent());
  ThisLock.release();

  // Wait for all events. If an event was linked to a command that is terminated
  // after leaving the critical section, the reference counting mechanism had
  // prevented the runtime to delete the memory associated with the event: we
  // can safely use the event to wait for a already terminated command.
  for (auto I = WaitList.begin(), E = WaitList.end(); I != E; ++I)
    (*I)->Wait();
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

  if (!Dev.Submit(*Cmd)) {
    Commands.front() = std::move(Cmd);
    return true;
  }

  PendingCommands.insert(Cmd.release());
  Commands.pop_front();
  CommandOnFly = true;

  return Commands.size();
}

void InOrderQueue::CommandDone(Command &Cmd) {
  ThisLock.acquire();
  CommandOnFly = false;
  ThisLock.release();

  CommandQueue::CommandDone(Cmd);
}
