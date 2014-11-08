
#include "opencrun/Device/CPU/Multiprocessor.h"
#include "opencrun/Device/CPU/CPUDevice.h"
#include "opencrun/System/OS.h"

#include <float.h>

using namespace opencrun;
using namespace opencrun::cpu;

//
// Multiprocessor implementation.
//

Multiprocessor::Multiprocessor(CPUDevice &Dev, const sys::HardwareSocket &Socket)
  : Dev(Dev) {
  for(sys::HardwareSocket::const_cpu_iterator I = Socket.cpu_begin(),
                                              E = Socket.cpu_end();
                                              I != E;
                                              ++I)
    Threads.insert(new CPUThread(*this, *I));
}

Multiprocessor::Multiprocessor(CPUDevice &Dev, const HardwareCPUsContainer &CPUs)
  : Dev(Dev) {
  for(HardwareCPUsContainer::iterator I = CPUs.begin(),
                                      E = CPUs.end();
                                      I != E;
                                      ++I)
    Threads.insert(new CPUThread(*this, **I));
}

Multiprocessor::~Multiprocessor() {
  llvm::DeleteContainerPointers(Threads);
}

bool Multiprocessor::Submit(ReadBufferCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(WriteBufferCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(CopyBufferCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(ReadImageCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(WriteImageCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(CopyImageCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(CopyImageToBufferCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(CopyBufferToImageCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(MapBufferCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(MapImageCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(UnmapMemObjectCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(ReadBufferRectCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(WriteBufferRectCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(CopyBufferRectCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(FillBufferCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(FillImageCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(NDRangeKernelBlockCPUCommand *Cmd) {
  CPUThread &Thr = GetLesserLoadedThread();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(NativeKernelCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

bool Multiprocessor::Submit(NoOpCPUCommand *Cmd) {
  // TODO: implement a smarter selection policy.
  CPUThread &Thr = **Threads.begin();

  return Thr.Submit(static_cast<CPUCommand *>(Cmd));
}

void Multiprocessor::NotifyDone(CPUServiceCommand *Cmd) {
  Dev.NotifyDone(Cmd);
}

void Multiprocessor::NotifyDone(CPUExecCommand *Cmd, int ExitStatus) {
  Dev.NotifyDone(Cmd, ExitStatus);
}

void Multiprocessor::GetPinnedCPUs(CPUDevice::HardwareCPUsContainer &CPUs) const {
  for(CPUThreadsContainer::iterator I = Threads.begin(),
                                    E = Threads.end();
                                    I != E;
                                    ++I)
    CPUs.insert((*I)->GetPinnedToCPU());
}

CPUThread &Multiprocessor::GetLesserLoadedThread() {
  CPUThreadsContainer::iterator I = Threads.begin(),
                                E = Threads.end();
  CPUThread *Thr = *I;
  float CurMinLoad, MinLoad = FLT_MAX;

  for(; I != E; ++I) {
    CurMinLoad = (*I)->GetLoadIndicator();
    if(CurMinLoad < MinLoad) {
      Thr = *I;
      MinLoad = CurMinLoad;
    }
  }

  return *Thr;
}

//
// ProfilerTraits<Multiprocessor> implementation.
//

sys::Time ProfilerTraits<Multiprocessor>::ReadTime(Multiprocessor &Profilable) {
  return ProfilerTraits<CPUDevice>::ReadTime(Profilable.Dev);
}
