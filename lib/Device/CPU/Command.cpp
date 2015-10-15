
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
    EnqueueNDRangeKernel &Cmd, Signature Entry,
    ArgsMappings &GlobalArgs, size_t StaticLocalSize,
    DimensionInfo::iterator I, DimensionInfo::iterator E,
    llvm::IntrusiveRefCntPtr<CPUMultiExecContext> CmdContext)
 : CPUMultiExecCommand(CPUCommand::NDRangeKernelBlock, Cmd, I.GetWorkGroup(),
                       std::move(CmdContext)),
  Entry(Entry),
  StaticLocalSize(StaticLocalSize),
  Start(I),
  End(E) {
  Kernel &Kern = GetKernel();

  unsigned ArgsCount = Kern.GetArgCount() + (StaticLocalSize > 0);

  // Hold arguments to be passed to stubs.
  Args = static_cast<void **>(sys::CAlloc(ArgsCount, sizeof(void *)));

  // We can start filling some arguments: global/constant buffers, images and arguments
  // passed by value.
  unsigned J = 0;
  for (auto I = Kern.arg_begin(), E = Kern.arg_end(); I != E; ++I, ++J) {
    Args[J] = nullptr;
    switch (I->getKind()) {
    default: break;
    case KernelArg::BufferArg:
    case KernelArg::ImageArg:
      Args[J] = GlobalArgs[J];
      break;
    case KernelArg::SamplerArg:
      if (auto *Smplr = I->getSampler()) {
        DeviceSampler *DevSmplr = new DeviceSampler(GetDeviceSampler(*Smplr));
        DevSmplrs.push_back(DevSmplr);

        // Store sampler address.
        Args[J] = DevSmplr;
      }
      break;
    case KernelArg::ByValueArg:
      Args[J] = I->getByValPtr();
      break;
    }
  }
}

NDRangeKernelBlockCPUCommand::~NDRangeKernelBlockCPUCommand() {
  // Sampler addresses can be freed.
  llvm::DeleteContainerPointers(DevSmplrs);
  sys::Free(Args);
}

void NDRangeKernelBlockCPUCommand::SetLocalParams(LocalMemory &Local) {
  Kernel &Kern = GetKernel();

  unsigned J = 0;
  for (auto I = Kern.arg_begin(), E = Kern.arg_end(); I != E; ++I, ++J)
    if (I->getKind() == KernelArg::LocalBufferArg)
      Args[J] = Local.Alloc(I->getLocalSize());

  if (StaticLocalSize)
    Args[J] = Local.GetStaticPtr();
}

cl_uint NDRangeKernelBlockCPUCommand::GetDeviceSampler(const Sampler &Smplr) {
  cl_uint flags = 0;

  if(Smplr.GetNormalizedCoords() == true)
    flags |= CLK_NORMALIZED_COORDS_TRUE;
  else if(Smplr.GetNormalizedCoords() == false)
    flags |= CLK_NORMALIZED_COORDS_FALSE;

  switch(Smplr.GetAddressingMode()) {
    case Sampler::AddressNone:
      flags |= CLK_ADDRESS_NONE;
      break;
    case Sampler::AddressClampToEdge:
      flags |= CLK_ADDRESS_CLAMP_TO_EDGE;
      break;
    case Sampler::AddressClamp:
      flags |= CLK_ADDRESS_CLAMP;
      break;
    case Sampler::AddressRepeat:
      flags |= CLK_ADDRESS_REPEAT;
      break;
    case Sampler::AddressMirroredRepeat:
      flags |= CLK_ADDRESS_MIRRORED_REPEAT;
      break;
    case Sampler::NoAddressing:
      break;
  }

  switch(Smplr.GetFilterMode()) {
    case Sampler::FilterNearest:
      flags |= CLK_FILTER_NEAREST;
      break;
    case Sampler::FilterLinear:
      flags |= CLK_FILTER_LINEAR;
      break;
    case Sampler::NoFilter:
      break;
  }

  return flags;
}
