
#ifndef OPENCRUN_CORE_EVENT_H
#define OPENCRUN_CORE_EVENT_H

#include "opencrun/Core/Command.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/Profiler.h"
#include "opencrun/System/Monitor.h"
#include "opencrun/Util/MTRefCounted.h"

#include <map>
#include <deque>

struct _cl_event { };

namespace opencrun {

class CommandQueue;
class Event;

class EventCallbackClojure {
public:
  typedef void (CL_CALLBACK *Signature)(cl_event, cl_int, void *);
  typedef void *UserDataSignature;

public:
  EventCallbackClojure(Signature Callback, UserDataSignature UserData) :
    Callback(Callback),
    UserData(UserData) { }

public:
  void Notify(Event &Ev, int Status) const;

private:
  const Signature Callback;
  const UserDataSignature UserData;
};

class Event : public _cl_event, public MTRefCountedBaseVPTR<Event> {
public:
  enum Type {
    InternalEvent,
    UserEvent
  };

public:
  typedef std::deque<EventCallbackClojure> CallList;
  typedef std::map<int, CallList> EventCallbacksContainer;

  typedef CallList::iterator call_iterator;
  typedef CallList::const_iterator const_call_iterator;

public:
  static bool classof(const _cl_event *Ev) { return true; }

protected:
  Event(Type EvTy, int Status) : EvTy(EvTy), Status(Status) { }
  virtual ~Event() { }

public:
  int Wait();

public:
  virtual Context &GetContext() const = 0;

  int GetStatus() const { return Status; }

  bool HasCompleted() const { return Status == CL_COMPLETE || Status < 0; }
  bool IsError() const { return Status < 0; }

  Type GetType() const { return EvTy; }

public:
  void AddEventCallback(int CallbackType,
                        EventCallbackClojure &Callback);

protected:
  void Signal(int Status);

protected:
  sys::Monitor Monitor;

private:
  Type EvTy;

  // Positive values are normal status. Negative values are platform/runtime
  // errors. See OpenCL specs, version 1.1, table 5.15.
  volatile int Status;

  // User-defined callback functions to be called at specific status changes.
  EventCallbacksContainer Callbacks;
};

class InternalEvent : public Event {
public:
  static bool classof(const Event *Ev) {
    return Ev->GetType() == Event::InternalEvent;
  }

public:
  InternalEvent(CommandQueue &Queue,
                Command &Cmd,
                ProfileSample *Sample = NULL);
  virtual ~InternalEvent();

  InternalEvent(const InternalEvent &That); // Do not implement.
  void operator=(const InternalEvent &That); // Do not implement.

public:
  Context &GetContext() const;
  CommandQueue &GetCommandQueue() const { return *Queue; }
  Command &GetCommand() const { return Cmd; }

  bool IsProfiled() const { return Profile.IsEnabled(); }

  const ProfileTrace &GetProfile() const { return Profile; }

public:
  void MarkSubmitted(ProfileSample *Sample = NULL);
  void MarkRunning(ProfileSample *Sample = NULL);
  void MarkSubRunning(ProfileSample *Sample = NULL);
  void MarkSubCompleted(ProfileSample *Sample = NULL);
  void MarkCompleted(int Status, ProfileSample *Sample = NULL);

private:
  llvm::IntrusiveRefCntPtr<CommandQueue> Queue;
  Command &Cmd;

  ProfileTrace Profile;
};

class UserEvent : public Event {
public:
  static bool classof(const Event *Ev) {
    return Ev->GetType() == Event::UserEvent;
  }

public:
  UserEvent(Context &Ctx);
  virtual ~UserEvent() {}

  UserEvent(const UserEvent &That); // Do not implement.
  void operator=(const UserEvent &That); // Do not implement.

public:
  Context &GetContext() const { return *Ctx; } 

public:
  bool SetStatus(int Status);

private:
  llvm::IntrusiveRefCntPtr<Context> Ctx;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_EVENT_H
