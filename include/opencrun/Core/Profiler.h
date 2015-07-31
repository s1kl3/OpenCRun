
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
  ProfileSample(ProfileSample::Label Label = ProfileSample::Unknown,
                int SubLabelId = -1) :
    SampleLabel(Label),
    SampleSubLabelId(SubLabelId) { }

public:
  const sys::Time &GetTime() const { return Time; }
  ProfileSample::Label GetLabel() const { return SampleLabel; }
  int GetSubLabelId() const { return SampleSubLabelId; }

private:
  void SetTime(sys::Time &Time) { this->Time = Time; }

private:
  Label SampleLabel;
  int SampleSubLabelId;
  sys::Time Time;

  friend class Profiler;
};

class ProfileTrace {
public:
  typedef std::vector<ProfileSample *> SamplesContainer;

  typedef SamplesContainer::iterator iterator;
  typedef SamplesContainer::const_iterator const_iterator;

public:
  const_iterator begin() const { return Samples.begin(); }
  const_iterator end() const { return Samples.end(); }

public:
  ~ProfileTrace() { llvm::DeleteContainerPointers(Samples); }

public:
  ProfileTrace &operator<<(ProfileSample *Sample) {
    if(Enabled && Sample) {
      // TODO: non-blocking implementation.
      llvm::sys::ScopedLock Lock(SampleLock);

      Samples.insert(FindInsertPoint(Sample->GetLabel()), Sample);
    }

    return *this;
  }

public:
  void SetEnabled(bool Enabled = true) { this->Enabled = Enabled; }
  bool IsEnabled() const  { return Enabled; }

private:
  iterator FindInsertPoint(ProfileSample::Label Label) {
    iterator I = Samples.begin(),
             E = Samples.end();

    while(I != E && (*I)->GetLabel() <= Label)
      ++I;

    return I;
  }

private:
  bool Enabled;

  llvm::sys::Mutex SampleLock;
  SamplesContainer Samples;
};

class Profiler {
public:
  enum Counter {
    None = 0,
    Time = 1 << 0
  };

public:
  Profiler();
  Profiler(const Profiler &That); // Do not implement.
  void operator=(const Profiler &That); // Do not implement.

public:
  ProfileSample *GetSample(unsigned Counters,
                           ProfileSample::Label Label = ProfileSample::Unknown,
                           int SubLabelId = -1);

  bool IsProfilingForcedFromEnvironment() const { return ToProfile; }

public:
  void DumpTrace(unsigned CmdType,
                 const ProfileTrace &Trace,
                 bool Force = false);

private:
  std::string FormatLabel(ProfileSample::Label Label, int SubId);

  llvm::raw_ostream &DumpPrefix();
  llvm::raw_ostream &DumpCommandType(unsigned CmdType);

private:
  llvm::sys::Mutex ThisLock;

  llvm::raw_fd_ostream ProfileStream;
  unsigned ToProfile;
};

Profiler &GetProfiler();

static inline
ProfileSample *GetProfilerSample(
                 unsigned Counters,
                 ProfileSample::Label Label = ProfileSample::Unknown,
                 int SubLabelId = -1) {
  return GetProfiler().GetSample(Counters, Label, SubLabelId);
}

} // End namespace opencrun.

#endif // OPENCRUN_CORE_PROFILE
