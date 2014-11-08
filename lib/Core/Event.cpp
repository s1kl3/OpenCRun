
#include "opencrun/Core/Event.h"
#include "opencrun/Core/CommandQueue.h"
#include "opencrun/Core/Context.h"

using namespace opencrun;

//
// EventCallbackClojure implementation.
//

void EventCallbackClojure::Notify(Event &Ev, int Status) const {
  if(Callback)
    Callback(&Ev, Status, UserData);
}

//
// Event implementation.
//

int Event::Wait() {
  sys::ScopedMonitor Mnt(Monitor);

  while(Status != CL_COMPLETE && Status >= 0)
    Mnt.Wait();

  return Status;
}

void Event::AddEventCallback(int CallbackType, EventCallbackClojure &Callback) {
  sys::ScopedMonitor Mnt(Monitor);

  // If the status for which the callback is being registered has
  // already been reached or passed we call the callback immediately,
  // otherwise we store the clojure and it will be triggered by a state
  // change to a value past or equal to CallbackType.
  if(Status <= CallbackType)
    Callback.Notify(*this, CallbackType);
  else
    Callbacks[CallbackType].push_back(Callback); 
}

void Event::Signal(int Status) {
  sys::ScopedMonitor Mnt(Monitor);

  // Sometimes can happen that a logically previous event is delayed. If so,
  // do not update status. OpenCL events are ordered from CL_QUEUED (3) to CL_COMPLETE (0).
  // Error events are lesser than 0, so the < operator can be used to filter out
  // delayed events.
  // This OpenCL event has not been delayed, update internal status.
  if(this->Status > Status)
    this->Status = Status;

  // If present, call the user-defined callback functions for status
  // change notification.
  if(Callbacks.count(Status))
    for(const_call_iterator I = Callbacks[Status].begin(),
                            E = Callbacks[Status].end();
                            I != E;
                            ++I)
      I->Notify(*this, Status);

  if(Status == CL_COMPLETE || Status < 0)
    Mnt.Broadcast();
}

//
// InternalEvent implementation.
//

#define RETURN_WITH_ERROR(MSG)               \
  {                                          \
  Queue->GetContext().ReportDiagnostic(MSG); \
  return;                                    \
  }

InternalEvent::InternalEvent(CommandQueue &Queue,
                             unsigned CmdType,
                             ProfileSample *Sample) :
  Event(Event::InternalEvent, CL_QUEUED),
  Queue(&Queue),
  CmdType(CmdType) {
  Profile.SetEnabled(Sample != NULL);
  Profile << Sample;
}

InternalEvent::~InternalEvent() {
  GetProfiler().DumpTrace(CmdType, Profile);
}

Context &InternalEvent::GetContext() const {
  return Queue->GetContext();
}

unsigned long InternalEvent::GetProfiledQueuedTime() const {
  return GetProfiledTime(ProfileSample::CommandEnqueued);
}

unsigned long InternalEvent::GetProfiledSubmittedTime() const {
  return GetProfiledTime(ProfileSample::CommandSubmitted);
}
 
unsigned long InternalEvent::GetProfiledRunningTime() const {
  return GetProfiledTime(ProfileSample::CommandRunning);
}

unsigned long InternalEvent::GetProfiledCompletedTime() const {
  return GetProfiledTime(ProfileSample::CommandCompleted);
}

unsigned long InternalEvent::GetProfiledTime(ProfileSample::Label SampleLabel) const {
  for(ProfileTrace::const_iterator I = Profile.begin(),
                                   E = Profile.end();
                                   I != E;
                                   ++I) {
    if((*I)->GetLabel() != SampleLabel)
      continue;

    return (*I)->GetTime().AsLong();
  }

  return 0;
}

void InternalEvent::MarkSubmitted(ProfileSample *Sample) {
  Profile << Sample;
  Signal(CL_SUBMITTED);
}

void InternalEvent::MarkRunning(ProfileSample *Sample) {
  Profile << Sample;
  Signal(CL_RUNNING);
}

void InternalEvent::MarkSubRunning(ProfileSample *Sample) {
  Profile << Sample;
}

void InternalEvent::MarkSubCompleted(ProfileSample *Sample) {
  Profile << Sample;
}

void InternalEvent::MarkCompleted(int Status, ProfileSample *Sample) {
  if(Status != CL_COMPLETE && Status >= 0)
    RETURN_WITH_ERROR("invalid event status");

  Profile << Sample;
  Signal(Status);
}

//
// UserEvent implementation.
//

UserEvent::UserEvent(Context &Ctx) :
  Event(Event::UserEvent, CL_SUBMITTED),
  Ctx(&Ctx) { }

bool UserEvent::SetStatus(int Status) {
  sys::ScopedMonitor Mnt(Monitor);

  // Status already changed
  if(GetStatus() != CL_SUBMITTED)
    return false;

  Signal(Status);
  return true;
}
