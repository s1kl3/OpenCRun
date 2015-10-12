
#ifndef OPENCRUN_CORE_PROFILE
#define OPENCRUN_CORE_PROFILE

#include "opencrun/System/Time.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Mutex.h"

#include <cstdlib>
#include <vector>

namespace opencrun {

class Command;

class ProfileSample {
public:
  enum Label {
    CommandEnqueued,
    CommandSubmitted,
    CommandRunning,
    CommandCompleted,
    Unknown
  };

public:
  const sys::Time &getTime() const { return Time; }
  Label getLabel() const { return static_cast<Label>(SampleLabel); }
  size_t getSubId() const { return SubId; }

  bool isSubSample() const {
    return IsSubSample;
  }

  static ProfileSample getEnqueued() {
    return getSample(CommandEnqueued);
  }
  static ProfileSample getSubmitted() {
    return getSample(CommandSubmitted);
  }
  static ProfileSample getRunning() {
    return getSample(CommandRunning);
  }
  static ProfileSample getCompleted() {
    return getSample(CommandCompleted);
  }
  static ProfileSample getSubRunning(size_t Id) {
    return getSubSample(CommandRunning, Id);
  }
  static ProfileSample getSubCompleted(size_t Id) {
    return getSubSample(CommandCompleted, Id);
  }

private:
  ProfileSample(Label L, bool IsSubSample, size_t SubId, const sys::Time &Time)
   : SampleLabel(L), IsSubSample(IsSubSample), SubId(SubId), Time(Time) {}

  static ProfileSample getSample(Label L);
  static ProfileSample getSubSample(Label L, size_t Id);

private:
  unsigned SampleLabel : 3;
  unsigned IsSubSample : 1;
  size_t SubId;
  sys::Time Time;

  friend class Profiler;
};

class ProfileTrace {
public:
  using SamplesContainer = std::vector<ProfileSample>;
  using iterator = SamplesContainer::const_iterator;

public:
  explicit ProfileTrace(bool Trace);

  iterator begin() const { return Samples.begin(); }
  iterator end() const { return Samples.end(); }

  ProfileTrace &operator<<(const ProfileSample &Sample);

public:
  void setEnabled(bool Enabled = true) { this->Enabled = Enabled; }
  bool isEnabled() const  { return Enabled; }

  void print(llvm::raw_ostream &OS, unsigned CmdType) const;

private:
  mutable llvm::sys::Mutex ThisLock;
  SamplesContainer Samples;
  bool Enabled;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_PROFILE
