
#ifndef OPENCRUN_DEVICE_CPU_MULTIPROCESSOR_H
#define OPENCRUN_DEVICE_CPU_MULTIPROCESSOR_H

#include "Command.h"
#include "CPUThread.h"
#include "opencrun/Core/Profiler.h"
#include "opencrun/System/Hardware.h"

#include "llvm/ADT/SmallPtrSet.h"

namespace opencrun {
namespace cpu {

class CPUDevice;

class Multiprocessor {
public:
  typedef llvm::SmallPtrSet<CPUThread *, 4> CPUThreadsContainer;
  typedef llvm::SmallPtrSet<const sys::HardwareCPU *, 16> HardwareCPUsContainer;

public:
  Multiprocessor(CPUDevice &Dev, const sys::HardwareSocket &Socket);
  Multiprocessor(CPUDevice &Dev, const HardwareCPUsContainer &CPUs);
  ~Multiprocessor();

  Multiprocessor(const Multiprocessor &That); // Do not implement.
  void operator=(const Multiprocessor &That); // Do not implement.

public:
  bool Submit(ReadBufferCPUCommand *Cmd);
  bool Submit(WriteBufferCPUCommand *Cmd);
  bool Submit(CopyBufferCPUCommand *Cmd);
  bool Submit(ReadImageCPUCommand *Cmd);
  bool Submit(WriteImageCPUCommand *Cmd);
  bool Submit(CopyImageCPUCommand *Cmd);
  bool Submit(CopyImageToBufferCPUCommand *Cmd);
  bool Submit(CopyBufferToImageCPUCommand *Cmd);
  bool Submit(MapBufferCPUCommand *Cmd);
  bool Submit(MapImageCPUCommand *Cmd);
  bool Submit(UnmapMemObjectCPUCommand *Cmd);
  bool Submit(ReadBufferRectCPUCommand *Cmd);
  bool Submit(WriteBufferRectCPUCommand *Cmd);
  bool Submit(CopyBufferRectCPUCommand *Cmd);
  bool Submit(FillBufferCPUCommand *Cmd);
  bool Submit(FillImageCPUCommand *Cmd);
  bool Submit(NoOpCPUCommand *Cmd);
  bool Submit(NDRangeKernelBlockCPUCommand *Cmd);
  bool Submit(NativeKernelCPUCommand *Cmd);

  void NotifyDone(CPUServiceCommand *Cmd);
  void NotifyDone(CPUExecCommand *Cmd, int ExitStatus);

  void GetPinnedCPUs(HardwareCPUsContainer &CPUs) const;

private:
  CPUThread &GetLesserLoadedThread();

private:
  CPUDevice &Dev;
  CPUThreadsContainer Threads;

  friend class ProfilerTraits<Multiprocessor>;
};

} // End namespace cpu.

template <>
class ProfilerTraits<Multiprocessor> {
public:
  static sys::Time ReadTime(cpu::Multiprocessor &Profilable);
};

} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_MULTIPROCESSOR_H
