
#ifndef OPENCRUN_CORE_COMMANDQUEUE_H
#define OPENCRUN_CORE_COMMANDQUEUE_H

#include "opencrun/Core/Command.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/Event.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/Mutex.h"

#include <deque>

struct _cl_command_queue { };

namespace opencrun {

class Command;
class Device;

class CommandQueue : public _cl_command_queue,
                     public MTRefCountedBaseVPTR<CommandQueue> {
public:
  enum Type {
    OutOfOrder,
    InOrder
  };

public:
  static bool classof(const _cl_command_queue *Cmd) { return true; }

public:
  typedef std::deque<std::unique_ptr<Command> > CommandsContainer;
  typedef std::set<Command *> PendingCommandsContainer;

protected:                     
  CommandQueue(Type Ty, Context &Ctx, Device &Dev, bool EnableProfile);
  virtual ~CommandQueue();

public:
  llvm::IntrusiveRefCntPtr<Event> Enqueue(Command &Cmd, cl_int *ErrCode = NULL);

  void Flush();
  void Finish();

  virtual bool RunScheduler() = 0;
  virtual void CommandDone(Command &Cmd);

public:
  Type GetType() const { return Ty; }

  Context &GetContext() const { return *Ctx; }
  Device &GetDevice() const { return Dev; }

  bool ProfilingEnabled() const { return EnableProfile; }

protected:
  bool submit(Command &Cmd);
  bool ensureMemoryCoherency(Command &Cmd);

protected:
  Type Ty;

  llvm::IntrusiveRefCntPtr<Context> Ctx;
  Device &Dev;

  bool EnableProfile;

  llvm::sys::Mutex ThisLock;
  CommandsContainer Commands;
  PendingCommandsContainer PendingCommands;
};

class OutOfOrderQueue : public CommandQueue {
public:
  static bool classof(const CommandQueue *Queue) {
    return Queue->GetType() == CommandQueue::OutOfOrder;
  }

public:
  OutOfOrderQueue(Context &Ctx, Device &Dev, bool EnableProfile) :
    CommandQueue(CommandQueue::OutOfOrder, Ctx, Dev, EnableProfile) { }
  virtual ~OutOfOrderQueue();

private:
  OutOfOrderQueue(const OutOfOrderQueue &That); // Do not implement.
  void operator=(const OutOfOrderQueue &That); // Do not implement.

public:
  virtual bool RunScheduler();
  virtual void CommandDone(Command &Cmd);
};

class InOrderQueue : public CommandQueue {
public:
  static bool classof(const CommandQueue *Queue) {
    return Queue->GetType() == CommandQueue::InOrder;
  }

public:
  InOrderQueue(Context &Ctx, Device &Dev, bool EnableProfile) :
    CommandQueue(CommandQueue::InOrder, Ctx, Dev, EnableProfile),
    CommandOnFly(false) { }
  virtual ~InOrderQueue();

private:
  InOrderQueue(const InOrderQueue &That); // Do not implement.
  void operator=(const InOrderQueue &That); // Do not implement.

public:
  virtual bool RunScheduler();
  virtual void CommandDone(Command &Cmd);

private:
  bool CommandOnFly;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_COMMANDQUEUE_H
