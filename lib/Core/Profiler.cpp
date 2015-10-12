
#include "opencrun/Core/Profiler.h"
#include "opencrun/Core/Command.h"
#include "opencrun/Core/Context.h"
#include "opencrun/System/Env.h"
#include "opencrun/System/Time.h"
#include "opencrun/Util/Table.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ManagedStatic.h"

#include <string>

#include <unistd.h>

using namespace opencrun;

ProfileSample ProfileSample::getSample(Label L) {
  return ProfileSample {L, false, 0, sys::Time::GetWallClock()};
}

ProfileSample ProfileSample::getSubSample(Label L, size_t Id) {
  return ProfileSample {L, true, Id, sys::Time::GetWallClock()};
}

ProfileTrace::ProfileTrace(bool Trace)
 : Enabled(Trace | sys::HasEnv("OPENCRUN_PROFILED_COUNTERS")) {}

ProfileTrace &ProfileTrace::operator<<(const ProfileSample &S) {
  if(Enabled) {
    llvm::sys::ScopedLock Lock(ThisLock);

    auto SampleCmp = [](const ProfileSample &S1, const ProfileSample &S2) {
      return S1.getLabel() < S2.getLabel();
    };
    auto IP = std::upper_bound(Samples.begin(), Samples.end(), S, SampleCmp);
    Samples.insert(IP, S);
  }

  return *this;
}

static std::string formatLabel(const ProfileSample &S) {
  std::string Text;

  switch (S.getLabel()) {
  case ProfileSample::Unknown:
    Text = "Unknown";
    break;

  case ProfileSample::CommandEnqueued:
    Text = "CommandEnqueued";
    break;

  case ProfileSample::CommandSubmitted:
    Text = "CommandSubmitted";
    break;

  case ProfileSample::CommandRunning:
    Text = "CommandRunning";
    break;

  case ProfileSample::CommandCompleted:
    Text = "CommandCompleted";
    break;

  default:
    llvm_unreachable("Unknown sample label");
  }

  if (S.isSubSample())
    Text += "-" + llvm::itostr(S.getSubId());

  return Text;
}

static llvm::raw_ostream &printPrefix(llvm::raw_ostream &OS,
                                      llvm::StringRef Prefix) {
  OS.changeColor(llvm::raw_ostream::GREEN) << Prefix << ":";
  return OS.resetColor();
}

void ProfileTrace::print(llvm::raw_ostream &OS, unsigned CmdType) const {
  llvm::sys::ScopedLock Lock(ThisLock);

  if (Samples.empty())
    return;

  util::Table Tab;

  Tab << "Label" << "Time" << "Delta" << util::Table::EOL;

  sys::Time Last;
  for (const auto &S : Samples) {
    // First columns must be always printed.
    Tab << formatLabel(S);

    Tab << S.getTime();

    // Emit delta with respect to previous event.
    if (S.isSubSample()) {
      Tab << S.getTime() - Last;
      Last = S.getTime();
    }

    // Do not emit delta for sub-commands.
    else
      Tab << util::Table::NA;

    // Terminate the line.
    Tab << util::Table::EOL;
  }

  printPrefix(OS, "profile") << " " << CmdType << "\n";
  Tab.Dump(OS, "profile");
  printPrefix(OS, "profile") << "\n";
}
