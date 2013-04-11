
#ifndef OPENCRUN_DEVICE_CPU_WAITLIST_H
#define OPENCRUN_DEVICE_CPU_WAITLIST_H

#include "opencrun/Device/CPU/OCLTypes.h"
#include "opencrun/System/Monitor.h"

#include "llvm/ADT/SmallVector.h"

#include <map>
#include <utility>

namespace opencrun {
namespace cpu {

class WaitList {
public:
  typedef std::pair<bool, llvm::SmallVector<event_t, 4> > EventData;
  typedef std::map<event_t, EventData> EventsContainer;

public:
  WaitList() {}

public:
  event_t GetEvent();

  event_t GetEvent(const event_t evt);

  void RemoveEvent(const event_t evt);

  void WaitEventCompletion(const event_t evt);

public:
  bool GetEventCompletion(const event_t evt);

  void SetEventCompletion(const event_t evt, bool completed);

private:
  EventsContainer events;

  sys::Monitor mntr;
};

WaitList &GetWaitList();
event_t GetEvent();
event_t GetEvent(const event_t evt);
void RemoveEvent(const event_t evt);
void WaitEventCompletion(const event_t evt);
bool GetEventCompletion(const event_t evt);
void SetEventCompletion(const event_t evt, bool completed);

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_WAITLIST_H
