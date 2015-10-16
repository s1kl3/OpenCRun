
#include "Command.h"
#include "InternalCalls.h"
#include "Memory.h"
#include "opencrun/Core/CommandQueue.h"
#include "opencrun/System/OS.h"

using namespace opencrun;
using namespace opencrun::cpu;

void CPUSingleExecCommand::notifyStart() const {
  auto &Ev = GetQueueCommand().GetNotifyEvent();
  Ev.MarkRunning(ProfileSample::getRunning());
}

void CPUSingleExecCommand::notifyCompletion(bool Error) const {
  auto &Ev = GetQueueCommand().GetNotifyEvent();
  Ev.MarkCompleted(-(int)Error, ProfileSample::getCompleted());
  Ev.GetCommandQueue().CommandDone(GetQueueCommand());
}

CPUMultiExecContext::CPUMultiExecContext(size_t N)
 : StartedCount(0), WaitCount(N), Errors(false) {
}

bool CPUMultiExecContext::start() {
  return StartedCount++ == 0;
}

bool CPUMultiExecContext::finish(bool Error) {
  if (Error)
    Errors.store(true);
  return --WaitCount == 0;
}

bool CPUMultiExecContext::hasErrors() const {
  return Errors;
}

void CPUMultiExecCommand::notifyStart() const {
  auto &Ev = GetQueueCommand().GetNotifyEvent();
  if (CmdContext->start())
    Ev.MarkRunning(ProfileSample::getRunning());

  Ev.MarkSubRunning(ProfileSample::getSubRunning(Id));
}

void CPUMultiExecCommand::notifyCompletion(bool Error) const {
  auto &Ev = GetQueueCommand().GetNotifyEvent();
  Ev.MarkSubCompleted(ProfileSample::getSubCompleted(Id));
  if (CmdContext->finish(Error)) {
    auto ExitStatus = -(int)CmdContext->hasErrors();
    Ev.MarkCompleted(ExitStatus, ProfileSample::getCompleted());
    Ev.GetCommandQueue().CommandDone(GetQueueCommand());
  }
}

NDRangeKernelBlockCPUCommand::NDRangeKernelBlockCPUCommand(
    EnqueueNDRangeKernel &Cmd, void *Entry,
    DimensionInfo::iterator I, DimensionInfo::iterator E,
    llvm::IntrusiveRefCntPtr<NDRangeKernelBlockContext> CmdContext)
 : CPUMultiExecCommand(CPUCommand::NDRangeKernelBlock, Cmd, I.GetWorkGroup(),
                       std::move(CmdContext)),
   Entry(reinterpret_cast<Signature>(Entry)), Start(I), End(E) {}
