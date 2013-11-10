
#ifndef OPENCRUN_DEVICE_CPU_CPUTHREAD_H
#define OPENCRUN_DEVICE_CPU_CPUTHREAD_H

#include "opencrun/Device/CPU/Command.h"
#include "opencrun/Device/CPU/Memory.h"
#include "opencrun/System/Hardware.h"
#include "opencrun/System/Monitor.h"
#include "opencrun/System/Thread.h"

#include <deque>

namespace opencrun {

namespace cpu {

class Multiprocessor;

class ExecutionStack {
public:
  typedef void (*EntryPoint)(void **);

public:
  ExecutionStack(const sys::HardwareCache &Cache);
  ~ExecutionStack();

public:
  void Reset(EntryPoint Entry, void **Args, unsigned N);
  void Run();
  void SwitchToNextWorkItem();

public:
  void Dump() const;
  void Dump(unsigned I) const;

private:
  void Dump(void *StartAddr, void *EndAddr) const;

private:
  void *Stack;
  size_t StackSize;
};

class CPUThread : public sys::Thread {
public:
  typedef std::deque<CPUCommand *> CPUCommands;

  enum WorkingMode {
    FullyOperational = 1 << 0,
    TearDown = 1 << 1,
    Stopped = 1 << 2,
    NoNewJobs = TearDown | Stopped,
    ExecJobs = FullyOperational | TearDown
  };

public:
  CPUThread(Multiprocessor &MP, const sys::HardwareCPU &CPU);
  virtual ~CPUThread();

public:
  bool Submit(CPUCommand *Cmd);

  virtual void Run();

public:
  float GetLoadIndicator();

  const DimensionInfo::iterator &GetCurrentIndex() { return Cur; }

  void SwitchToNextWorkItem();

private:
  bool Submit(CPUServiceCommand *Cmd);
  bool Submit(StopDeviceCPUCommand *Cmd) { Mode = TearDown; return true; }

  bool Submit(CPUExecCommand *Cmd);
  bool Submit(ReadBufferCPUCommand *Cmd) { return true; }
  bool Submit(WriteBufferCPUCommand *Cmd) { return true; }
  bool Submit(CopyBufferCPUCommand *Cmd) { return true; }
  bool Submit(ReadImageCPUCommand *Cmd) { return true; }
  bool Submit(WriteImageCPUCommand *Cmd) { return true; }
  bool Submit(MapBufferCPUCommand *Cmd) { return true; }
  bool Submit(UnmapMemObjectCPUCommand *Cmd) { return true; }
  bool Submit(ReadBufferRectCPUCommand *Cmd) { return true; }
  bool Submit(WriteBufferRectCPUCommand *Cmd) { return true; }
  bool Submit(CopyBufferRectCPUCommand *Cmd) { return true; }
  bool Submit(FillBufferCPUCommand *Cmd) { return true; }
  bool Submit(NDRangeKernelBlockCPUCommand *Cmd) { return true; }
  bool Submit(NativeKernelCPUCommand *Cmd) { return true; }

  void Execute(CPUCommand *Cmd);

  void Execute(CPUServiceCommand *Cmd);
  void Execute(StopDeviceCPUCommand *Cmd) { Mode = Stopped; }

  void Execute(CPUExecCommand *Cmd);
  
  int Execute(ReadBufferCPUCommand &Cmd);
  int Execute(WriteBufferCPUCommand &Cmd);
  int Execute(CopyBufferCPUCommand &Cmd);
  int Execute(ReadImageCPUCommand &Cmd);
  int Execute(WriteImageCPUCommand &Cmd);
  int Execute(MapBufferCPUCommand &Cmd);
  int Execute(UnmapMemObjectCPUCommand &Cmd);
  int Execute(ReadBufferRectCPUCommand &Cmd);
  int Execute(WriteBufferRectCPUCommand &Cmd);
  int Execute(CopyBufferRectCPUCommand &Cmd);
  int Execute(FillBufferCPUCommand &Cmd);
  int Execute(NDRangeKernelBlockCPUCommand &Cmd);
  int Execute(NativeKernelCPUCommand &Cmd);
  
  void MemRectCpy(void *Target, 
                  const void *Source,
                  const size_t *Region,
                  size_t TargetRowPitch,
                  size_t TargetSlicePitch,
                  size_t SourceRowPitch,
                  size_t SourceSlicePitch);
                  
  template<typename T>
  void MemFill(void *Target,
               const void *Source,
               size_t NumEls) {
    T *Target_T = reinterpret_cast<T *>(Target);
    const T Source_T = *(reinterpret_cast<const T *>(Source));

    for(size_t I = 0; I < NumEls; ++I)
      Target_T[I] = Source_T;
  }
                  
private:
  sys::Monitor ThisMnt;

  volatile WorkingMode Mode;
  CPUCommands Commands;
  Multiprocessor &MP;

  DimensionInfo::iterator Begin;
  DimensionInfo::iterator End;
  DimensionInfo::iterator Cur;

  ExecutionStack Stack;
  LocalMemory Local;
};

CPUThread &GetCurrentThread();

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_CPUTREAD_H
