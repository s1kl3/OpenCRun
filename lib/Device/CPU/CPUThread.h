
#ifndef OPENCRUN_DEVICE_CPU_CPUTHREAD_H
#define OPENCRUN_DEVICE_CPU_CPUTHREAD_H

#include "Command.h"
#include "Memory.h"
#include "opencrun/System/Hardware.h"
#include "opencrun/System/Monitor.h"
#include "opencrun/System/Thread.h"

#include <deque>

namespace opencrun {
namespace cpu {

class CPUDevice;

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
  size_t getRequiredStackSize(unsigned N) const;
  size_t getPrivateSize() const;

private:
  void *Stack;
  size_t StackSize;
};

class CPUThread : public sys::Thread {
public:
  typedef std::deque<CPUCommand *> CPUCommands;
  typedef llvm::SmallVector<void *, 8> StaticLocalPointers;

  enum WorkingMode {
    FullyOperational = 1 << 0,
    TearDown = 1 << 1,
    Stopped = 1 << 2,
    NoNewJobs = TearDown | Stopped,
    ExecJobs = FullyOperational | TearDown
  };

public:
  CPUThread(CPUDevice &Dev, const sys::HardwareCPU &CPU);
  virtual ~CPUThread();

public:
  bool Submit(CPUCommand *Cmd);

  virtual void Run();

public:
  float GetLoadIndicator();

  const DimensionInfo::iterator &GetCurrentIndex() { return Cur; }

  void SwitchToNextWorkItem();

private:
  void Execute(CPUCommand *Cmd);

  void Execute(CPUServiceCommand &Cmd);
  void Execute(StopDeviceCPUCommand &Cmd);

  void Execute(CPUExecCommand &Cmd);
  
  int Execute(ReadBufferCPUCommand &Cmd);
  int Execute(WriteBufferCPUCommand &Cmd);
  int Execute(CopyBufferCPUCommand &Cmd);
  int Execute(ReadImageCPUCommand &Cmd);
  int Execute(WriteImageCPUCommand &Cmd);
  int Execute(CopyImageCPUCommand &Cmd);
  int Execute(CopyImageToBufferCPUCommand &Cmd);
  int Execute(CopyBufferToImageCPUCommand &Cmd);
  int Execute(MapBufferCPUCommand &Cmd);
  int Execute(MapImageCPUCommand &Cmd);
  int Execute(UnmapMemObjectCPUCommand &Cmd);
  int Execute(ReadBufferRectCPUCommand &Cmd);
  int Execute(WriteBufferRectCPUCommand &Cmd);
  int Execute(CopyBufferRectCPUCommand &Cmd);
  int Execute(FillBufferCPUCommand &Cmd);
  int Execute(FillImageCPUCommand &Cmd);
  int Execute(NDRangeKernelBlockCPUCommand &Cmd);
  int Execute(NativeKernelCPUCommand &Cmd);
  int Execute(NoOpCPUCommand &Cmd);

private:
  // Method used to copy data between rectangular regions.
  void MemRectCpy(void *Target, 
                  const void *Source,
                  const size_t *Region,
                  size_t TargetRowPitch,
                  size_t TargetSlicePitch,
                  size_t SourceRowPitch,
                  size_t SourceSlicePitch);

  // Method used to rewrite colour compoment data from one
  // of the RGBA channels in the input buffer to one channel
  // in the output buffer and to convert it to the appropriate
  // data type.
  void WriteChannel(void *DataOut,
                    const void *DataIn,
                    size_t ToChNum,
                    size_t FromChNum,
                    const cl_image_format &ImgFmt);
  
  // Template method used to fill a target buffer with NumEls
  // elements of type T pointed by Source.
  template<typename T>
  void MemFill(void *Target,
               const void *Source,
               size_t NumEls) {
    T *Target_T = reinterpret_cast<T *>(Target);
    const T Source_T = *(reinterpret_cast<const T *>(Source));

    for(size_t I = 0; I < NumEls; ++I)
      Target_T[I] = Source_T;
  }

  // Template method used to fill a rectangular region of a
  // target buffer with elements of type T.
  template<typename T>
  void MemRectFill(void *Target,
                   const void *Source,
                   const size_t *Region,
                   size_t TargetRowPitch,
                   size_t TargetSlicePitch) {
  // Region's first element is stored in bytes inside
  // EnqueueFillImage class.
  size_t RowEls = Region[0] / sizeof(T);

    for(size_t Z = 0; Z < Region[2]; ++Z)
      for(size_t Y = 0; Y < Region[1]; ++Y)
        MemFill<T>(reinterpret_cast<void *>(
                    reinterpret_cast<uintptr_t>(Target) +
                    TargetRowPitch * Y +
                    TargetSlicePitch * Z
                   ),
                   Source,
                   RowEls);
  }

  // Conversion from single-precision floating point to half-precision
  // floating point and its reversal.
  cl_half FloatToHalf(cl_float f);
  cl_float HalfToFloat(cl_half h);

private:
  sys::Monitor ThisMnt;

  volatile WorkingMode Mode;
  CPUCommands Commands;
  CPUDevice &Dev;

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
