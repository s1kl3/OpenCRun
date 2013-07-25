
#include "opencrun/Device/CPU/WaitList.h"

#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/ErrorHandling.h"

using namespace opencrun;
using namespace opencrun::cpu;

event_t WaitList::GetEvent() {

  sys::ScopedMonitor lock(mntr);

  event_t new_evt = new _event_t;
  EventData new_evt_data(false, llvm::SmallVector<event_t, 4>());
  events[new_evt] = new_evt_data;

  return new_evt;

}

event_t WaitList::GetEvent(const event_t evt) {

  sys::ScopedMonitor lock(mntr);

  event_t new_evt = new _event_t;
  EventData new_evt_data(false, llvm::SmallVector<event_t, 4>());
  events[new_evt] = new_evt_data;

  // The pre-existing event evt must store a reference
  // to the newly inserted event.
  if(events.count(evt))
    llvm_unreachable("Non existant event");
  
  events[evt].second.push_back(new_evt);

  return new_evt;

}

void WaitList::RemoveEvent(const event_t evt) {

  sys::ScopedMonitor lock(mntr);

  events.erase(evt);

  delete evt;
  
}

void WaitList::WaitEventCompletion(const event_t evt) {

  sys::ScopedMonitor lock(mntr);

  if(events.count(evt) == 0)
    return;

  llvm::SmallVector<event_t, 4> &linked_evts =
    events[evt].second;

  for(llvm::SmallVector<event_t, 4>::iterator I = linked_evts.begin(),
                                              E = linked_evts.end();
                                              I != E;
                                              I++) {
    lock.Exit();
    // Call recursively to wait for attached events
    // completion.
    WaitEventCompletion(*I);

    lock.Enter();
    
    // Remove completed event from attached events' list.
    linked_evts.erase(I);
  }

  while(events[evt].first != true)
    lock.Wait();

  RemoveEvent(evt);

}

bool WaitList::GetEventCompletion(const event_t evt) {
  
  sys::ScopedMonitor lock(mntr);

  if(events.count(evt) == 0)
    return false;

  return events[evt].first;

}

void WaitList::SetEventCompletion(const event_t evt, bool completed) {

  sys::ScopedMonitor lock(mntr);

  if(events.count(evt) == 0)
    return;

  events[evt].first = completed;

  // Signal one thread waiting inside WaitEventCompletion .
  lock.Signal();

}

/*
 * Global event_t list. 
 */
llvm::ManagedStatic<WaitList> wait_list;

WaitList &opencrun::cpu::GetWaitList() {
  return *wait_list;
}

event_t opencrun::cpu::GetEvent() {
  return wait_list->GetEvent();
}

event_t opencrun::cpu::GetEvent(const event_t evt) {
  return wait_list->GetEvent(evt);
}

void opencrun::cpu::WaitEventCompletion(const event_t evt) {
  wait_list->WaitEventCompletion(evt);
}

bool opencrun::cpu::GetEventCompletion(const event_t evt) {
  return wait_list->GetEventCompletion(evt);
}

void opencrun::cpu::SetEventCompletion(const event_t evt, bool completed) {
  wait_list->SetEventCompletion(evt, completed);
}
