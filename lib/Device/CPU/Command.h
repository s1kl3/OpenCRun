
#ifndef OPENCRUN_DEVICE_CPU_COMMAND_H
#define OPENCRUN_DEVICE_CPU_COMMAND_H

#include "opencrun/Core/Command.h"
#include "opencrun/Core/Context.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"

#include <utility>

namespace opencrun {
namespace cpu {

class LocalMemory;

class CPUCommand {
public:
  enum Type {
    StopDevice,
    LastCPUServiceCommand,
    ReadBuffer,
    WriteBuffer,
    CopyBuffer,
    ReadImage,
    WriteImage,
    CopyImage,
    CopyImageToBuffer,
    CopyBufferToImage,
    MapBuffer,
    MapImage,
    UnmapMemObject,
    ReadBufferRect,
    WriteBufferRect,
    CopyBufferRect,
    FillBuffer,
    FillImage,
    NativeKernel,
    NoOp,
    LastCPUSingleExecCommand,
    NDRangeKernelBlock
  };

  enum {
    NoError           = CL_COMPLETE,
    InvalidExecutable = CL_INVALID_PROGRAM_EXECUTABLE,
    Unsupported       = CL_INVALID_OPERATION
  };

public:
  class ResultRecorder : public MTRefCountedBase<ResultRecorder> {
  public:
    typedef llvm::SmallVector<int, 32> StatusContainer;

  public:
    ResultRecorder(unsigned N = 1) : Started(false),
                                     ToWait(N),
                                     ExitStatus(N, CPUCommand::NoError) { }

  public:
    bool SetStarted() {
      // Common case, command started -- skip atomic operations.
      if(Started)
        return false;

      // Infrequent/contented case -- use CAS.
      return !llvm::sys::CompareAndSwap(&Started, true, false);
    }

    bool SetExitStatus(unsigned ExitStatus) {
      // Decrement the number of acknowledgment to wait.
      unsigned I = llvm::sys::AtomicDecrement(&ToWait);

      // The returned number is the caller Id inside the exit status array.
      this->ExitStatus[I] = ExitStatus;

      // Return true if this is the last acknowledgment.
      return I == 0;
    }

    int GetExitStatus() {
      int Status = CPUCommand::NoError;

      for(StatusContainer::iterator I = ExitStatus.begin(),
                                    E = ExitStatus.end();
                                    I != E && Status == CPUCommand::NoError;
                                    ++I)
        Status = *I;

      return Status;
    }

  private:
    volatile llvm::sys::cas_flag Started;
    volatile llvm::sys::cas_flag ToWait;
    StatusContainer ExitStatus;
  };

protected:
  CPUCommand(Type CommandTy) : CommandTy(CommandTy),
                               Result(new CPUCommand::ResultRecorder()) { }

  CPUCommand(Type CommandTy, CPUCommand::ResultRecorder &Result) :
    CommandTy(CommandTy),
    Result(&Result) { }

public:
  virtual ~CPUCommand() { }

public:
  Type GetType() const { return CommandTy; }

  virtual bool IsProfiled() const = 0;

  int GetExitStatus() { return Result->GetExitStatus(); }

public:
  bool RegisterStarted() {
    return Result->SetStarted();
  }

  bool RegisterCompleted(int ExitStatus) {
    return Result->SetExitStatus(ExitStatus);
  }

private:
  Type CommandTy;
  llvm::IntrusiveRefCntPtr<ResultRecorder> Result;
};

class CPUServiceCommand : public CPUCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() < CPUCommand::LastCPUServiceCommand;
  }

protected:
  CPUServiceCommand(CPUCommand::Type CommandTy) : CPUCommand(CommandTy) { }

public:
  virtual bool IsProfiled() const { return false; }
};

class CPUExecCommand : public CPUCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() > CPUCommand::LastCPUServiceCommand;
  }

protected:
  CPUExecCommand(CPUCommand::Type CommandTy, Command &Cmd) :
    CPUCommand(CommandTy),
    Cmd(Cmd) { }

  CPUExecCommand(CPUCommand::Type CommandTy,
                 Command &Cmd,
                 CPUCommand::ResultRecorder &Result) :
    CPUCommand(CommandTy, Result),
    Cmd(Cmd) { }

public:
  Command &GetQueueCommand() const { return Cmd; }
  template <typename Ty>
  Ty &GetQueueCommandAs() const { return llvm::cast<Ty>(Cmd); }

  virtual bool IsProfiled() const { return Cmd.IsProfiled(); }

private:
  Command &Cmd;
};

class CPUSingleExecCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return CPUExecCommand::classof(Cmd) &&
           Cmd->GetType() < CPUCommand::LastCPUSingleExecCommand;
  }

protected:
  CPUSingleExecCommand(CPUCommand::Type CommandTy, Command &Cmd)
    : CPUExecCommand(CommandTy, Cmd) { }
};

class CPUMultiExecCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return CPUExecCommand::classof(Cmd) &&
           Cmd->GetType() > CPUCommand::LastCPUSingleExecCommand;
  }

protected:
  CPUMultiExecCommand(CPUCommand::Type CommandTy,
                      Command &Cmd,
                      CPUCommand::ResultRecorder &Result,
                      unsigned Id) :
    CPUExecCommand(CommandTy, Cmd, Result),
    Id(Id) { }

public:
  unsigned GetId() const { return Id; }

private:
  unsigned Id;
};

class StopDeviceCPUCommand : public CPUServiceCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::StopDevice;
  }

public:
  StopDeviceCPUCommand() : CPUServiceCommand(CPUCommand::StopDevice) { }
};

class NoOpCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::NoOp;
  }

public:
  NoOpCPUCommand(Command &Cmd) : CPUExecCommand(CPUCommand::NoOp, Cmd) {}
};

class ReadBufferCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::ReadBuffer;
  }

public:
  ReadBufferCPUCommand(EnqueueReadBuffer &Cmd, const void *Src)
    : CPUExecCommand(CPUCommand::ReadBuffer, Cmd),
      Src(Src) { }

  void *GetTarget() {
    return GetQueueCommandAs<EnqueueReadBuffer>().GetTarget();
  }

  const void *GetSource() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Src);
    EnqueueReadBuffer &Cmd = GetQueueCommandAs<EnqueueReadBuffer>();

    return reinterpret_cast<const void *>(Base + Cmd.GetOffset());
  }

  size_t GetSize() {
    return GetQueueCommandAs<EnqueueReadBuffer>().GetSize();
  }

private:
  const void *Src;
};

class WriteBufferCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::WriteBuffer;
  }

public:
  WriteBufferCPUCommand(EnqueueWriteBuffer &Cmd, void *Dst)
    : CPUExecCommand(CPUCommand::WriteBuffer, Cmd),
      Dst(Dst) { }

public:
  void *GetTarget() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Dst);
    EnqueueWriteBuffer &Cmd = GetQueueCommandAs<EnqueueWriteBuffer>();
    
    return reinterpret_cast<void *>(Base + Cmd.GetOffset());
  }

  const void *GetSource() {
    return GetQueueCommandAs<EnqueueWriteBuffer>().GetSource();
  }

  size_t GetSize() {
    return GetQueueCommandAs<EnqueueWriteBuffer>().GetSize();
  }

private:
  void *Dst;
};

class CopyBufferCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::CopyBuffer;
  }

public:
  CopyBufferCPUCommand(EnqueueCopyBuffer &Cmd, 
                       void *Dst,
                       const void *Src)
    :	CPUExecCommand(CPUCommand::CopyBuffer, Cmd),
      Dst(Dst),
      Src(Src) { }
    
public:
  void *GetTarget() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Dst);
    EnqueueCopyBuffer &Cmd = GetQueueCommandAs<EnqueueCopyBuffer>();
    
    return reinterpret_cast<void *>(Base + Cmd.GetTargetOffset());
  }
  
  const void *GetSource() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Src);
    EnqueueCopyBuffer &Cmd = GetQueueCommandAs<EnqueueCopyBuffer>();
    
    return reinterpret_cast<const void *>(Base + Cmd.GetSourceOffset());	
  }
  
  size_t GetSize() {
    return GetQueueCommandAs<EnqueueCopyBuffer>().GetSize();	
  }

private:
  void *Dst;
  const void *Src;
};

class ReadImageCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::ReadImage;
  }

public:
  ReadImageCPUCommand(EnqueueReadImage &Cmd, const void *Src)
    : CPUExecCommand(CPUCommand::ReadImage, Cmd),
      Src(Src) { }

  void *GetTarget() {
    return GetQueueCommandAs<EnqueueReadImage>().GetTarget();
  }

  const void *GetSource() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Src);
    EnqueueReadImage &Cmd = GetQueueCommandAs<EnqueueReadImage>();

    return reinterpret_cast<const void *>(Base + Cmd.GetSourceOffset());
  }

  size_t GetTargetRowPitch() {
    return GetQueueCommandAs<EnqueueReadImage>().GetTargetRowPitch();
  }
  
  size_t GetTargetSlicePitch() {
    return GetQueueCommandAs<EnqueueReadImage>().GetTargetSlicePitch();  
  }
  
  size_t GetSourceRowPitch() {
    return GetQueueCommandAs<EnqueueReadImage>().GetSourceRowPitch();
  }
  
  size_t GetSourceSlicePitch() {
    return GetQueueCommandAs<EnqueueReadImage>().GetSourceSlicePitch();
  }
  
  const size_t *GetRegion() {
    return GetQueueCommandAs<EnqueueReadImage>().GetRegion();
  }

private:
  const void *Src;
};

class WriteImageCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::WriteImage;
  }

public:
  WriteImageCPUCommand(EnqueueWriteImage &Cmd, void *Dst)
    : CPUExecCommand(CPUCommand::WriteImage, Cmd),
      Dst(Dst) { }

  void *GetTarget() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Dst);
    EnqueueWriteImage &Cmd = GetQueueCommandAs<EnqueueWriteImage>();

    return reinterpret_cast<void *>(Base + Cmd.GetTargetOffset());
  }

  const void *GetSource() {
    return GetQueueCommandAs<EnqueueWriteImage>().GetSource();
  }

  size_t GetTargetRowPitch() {
    return GetQueueCommandAs<EnqueueWriteImage>().GetTargetRowPitch();
  }
  
  size_t GetTargetSlicePitch() {
    return GetQueueCommandAs<EnqueueWriteImage>().GetTargetSlicePitch();  
  }
  
  size_t GetSourceRowPitch() {
    return GetQueueCommandAs<EnqueueWriteImage>().GetSourceRowPitch();
  }
  
  size_t GetSourceSlicePitch() {
    return GetQueueCommandAs<EnqueueWriteImage>().GetSourceSlicePitch();
  }
  
  const size_t *GetRegion() {
    return GetQueueCommandAs<EnqueueWriteImage>().GetRegion();
  }

private:
  void *Dst;
};

class CopyImageCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::CopyImage;
  }

public:
  CopyImageCPUCommand(EnqueueCopyImage &Cmd, void *Dst, const void *Src)
    : CPUExecCommand(CPUCommand::CopyImage, Cmd),
      Dst(Dst),
      Src(Src) { }

  void *GetTarget() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Dst);
    EnqueueCopyImage &Cmd = GetQueueCommandAs<EnqueueCopyImage>();

    return reinterpret_cast<void *>(Base + Cmd.GetTargetOffset());
  }

  const void *GetSource() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Src);
    EnqueueCopyImage &Cmd = GetQueueCommandAs<EnqueueCopyImage>();

    return reinterpret_cast<const void *>(Base + Cmd.GetSourceOffset());
  }

  size_t GetTargetRowPitch() {
    return GetQueueCommandAs<EnqueueCopyImage>().GetTargetRowPitch();
  }
  
  size_t GetTargetSlicePitch() {
    return GetQueueCommandAs<EnqueueCopyImage>().GetTargetSlicePitch();  
  }
  
  size_t GetSourceRowPitch() {
    return GetQueueCommandAs<EnqueueCopyImage>().GetSourceRowPitch();
  }
  
  size_t GetSourceSlicePitch() {
    return GetQueueCommandAs<EnqueueCopyImage>().GetSourceSlicePitch();
  }
  
  const size_t *GetRegion() {
    return GetQueueCommandAs<EnqueueCopyImage>().GetRegion();
  }

private:
  void *Dst;
  const void *Src;
};

class CopyImageToBufferCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::CopyImageToBuffer;
  }

public:
  CopyImageToBufferCPUCommand(EnqueueCopyImageToBuffer &Cmd, 
                              void *Dst,
                              const void *Src)
    : CPUExecCommand(CPUCommand::CopyImageToBuffer, Cmd),
      Dst(Dst),
      Src(Src) { }

  void *GetTarget() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Dst);
    EnqueueCopyImageToBuffer &Cmd = 
      GetQueueCommandAs<EnqueueCopyImageToBuffer>();

    return reinterpret_cast<void *>(Base + Cmd.GetTargetOffset());
  }

  const void *GetSource() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Src);
    EnqueueCopyImageToBuffer &Cmd = 
      GetQueueCommandAs<EnqueueCopyImageToBuffer>();

    return reinterpret_cast<const void *>(Base + Cmd.GetSourceOffset());
  }

  size_t GetSourceRowPitch() {
    return GetQueueCommandAs<EnqueueCopyImageToBuffer>().GetSourceRowPitch();
  }
  
  size_t GetSourceSlicePitch() {
    return GetQueueCommandAs<EnqueueCopyImageToBuffer>().GetSourceSlicePitch();
  }
  
  const size_t *GetRegion() {
    return GetQueueCommandAs<EnqueueCopyImageToBuffer>().GetRegion();
  }

private:
  void *Dst;
  const void *Src;
};

class CopyBufferToImageCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::CopyBufferToImage;
  }

public:
  CopyBufferToImageCPUCommand(EnqueueCopyBufferToImage &Cmd, 
                              void *Dst,
                              const void *Src)
    : CPUExecCommand(CPUCommand::CopyBufferToImage, Cmd),
      Dst(Dst),
      Src(Src) { }

  void *GetTarget() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Dst);
    EnqueueCopyBufferToImage &Cmd = 
      GetQueueCommandAs<EnqueueCopyBufferToImage>();

    return reinterpret_cast<void *>(Base + Cmd.GetTargetOffset());
  }

  const void *GetSource() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Src);
    EnqueueCopyBufferToImage &Cmd = 
      GetQueueCommandAs<EnqueueCopyBufferToImage>();

    return reinterpret_cast<const void *>(Base + Cmd.GetSourceOffset());
  }

  size_t GetTargetRowPitch() {
    return GetQueueCommandAs<EnqueueCopyBufferToImage>().GetTargetRowPitch();
  }
  
  size_t GetTargetSlicePitch() {
    return GetQueueCommandAs<EnqueueCopyBufferToImage>().GetTargetSlicePitch();
  }
  
  const size_t *GetRegion() {
    return GetQueueCommandAs<EnqueueCopyBufferToImage>().GetRegion();
  }

private:
  void *Dst;
  const void *Src;
};

class MapBufferCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::MapBuffer;
  }
    
public:
  MapBufferCPUCommand(EnqueueMapBuffer &Cmd,
                      const void *Src)
    : CPUExecCommand(CPUCommand::MapBuffer, Cmd),
      Src(Src) { }

public:
  void *GetTarget() {
    return GetQueueCommandAs<EnqueueMapBuffer>().GetMapBuffer();		
  }
      
  const void *GetSource() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Src);
    EnqueueMapBuffer &Cmd = GetQueueCommandAs<EnqueueMapBuffer>();
    
    return reinterpret_cast<void *>(Base + Cmd.GetOffset());
  }
  
  size_t GetSize() {
    return GetQueueCommandAs<EnqueueMapBuffer>().GetSize();
  }
  
private:
  const void *Src;
};

class MapImageCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::MapImage;
  }
    
public:
  MapImageCPUCommand(EnqueueMapImage &Cmd,
                     const void *Src)
    : CPUExecCommand(CPUCommand::MapImage, Cmd),
      Src(Src) { }

public:
  void *GetTarget() {
    return GetQueueCommandAs<EnqueueMapImage>().GetMapBuffer();		
  }
      
  const void *GetSource() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Src);
    EnqueueMapImage &Cmd = GetQueueCommandAs<EnqueueMapImage>();
    
    return reinterpret_cast<void *>(Base + Cmd.GetOffset());
  }

  size_t GetTargetRowPitch() {
    return GetQueueCommandAs<EnqueueMapImage>().GetMapRowPitch();
  }
  
  size_t GetTargetSlicePitch() {
    return GetQueueCommandAs<EnqueueMapImage>().GetMapSlicePitch();  
  }
  
  size_t GetSourceRowPitch() {
    return GetQueueCommandAs<EnqueueMapImage>().GetSourceRowPitch();
  }
  
  size_t GetSourceSlicePitch() {
    return GetQueueCommandAs<EnqueueMapImage>().GetSourceSlicePitch();
  }
  
  const size_t *GetRegion() {
    return GetQueueCommandAs<EnqueueMapImage>().GetRegion();
  }

private:
  const void *Src;
};

class UnmapMemObjectCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::UnmapMemObject;
  }
  
public:
  UnmapMemObjectCPUCommand(EnqueueUnmapMemObject &Cmd,
                           void *MemObjAddr,
                           void *MappedPtr)
    : CPUExecCommand(CPUCommand::UnmapMemObject, Cmd),
      MemObjAddr(MemObjAddr),
      MappedPtr(MappedPtr) { }
    
public:
  void *GetMemObjAddr() const { return MemObjAddr; }
  
  void *GetMappedPtr() const { return MappedPtr; }
  
private:
  void *MemObjAddr;
  void *MappedPtr;
};

class ReadBufferRectCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::ReadBufferRect;
  }
  
public:
  ReadBufferRectCPUCommand(EnqueueReadBufferRect &Cmd,
                           void *Dst,
                           void *Src)
    : CPUExecCommand(CPUCommand::ReadBufferRect, Cmd),
      Dst(Dst),
      Src(Src) { }

  void *GetTarget() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Dst);
    EnqueueReadBufferRect &Cmd = GetQueueCommandAs<EnqueueReadBufferRect>();
    
    return reinterpret_cast<void *>(Base + Cmd.GetTargetOffset());
  }

  void *GetSource() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Src);
    EnqueueReadBufferRect &Cmd = GetQueueCommandAs<EnqueueReadBufferRect>();

    return reinterpret_cast<void *>(Base + Cmd.GetSourceOffset());
  }
  
  size_t GetTargetRowPitch() {
    return GetQueueCommandAs<EnqueueReadBufferRect>().GetTargetRowPitch();
  }
  
  size_t GetTargetSlicePitch() {
  return GetQueueCommandAs<EnqueueReadBufferRect>().GetTargetSlicePitch();  
  }
  
  size_t GetSourceRowPitch() {
  return GetQueueCommandAs<EnqueueReadBufferRect>().GetSourceRowPitch();
  }
  
  size_t GetSourceSlicePitch() {
    return GetQueueCommandAs<EnqueueReadBufferRect>().GetSourceSlicePitch();
  }
  
  const size_t *GetRegion() {
    return GetQueueCommandAs<EnqueueReadBufferRect>().GetRegion();
  }

private:
  void *Dst;
  void *Src;
};

class WriteBufferRectCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::WriteBufferRect;
  }
  
public:
  WriteBufferRectCPUCommand(EnqueueWriteBufferRect &Cmd,
                            void *Dst,
                            const void *Src)
    : CPUExecCommand(CPUCommand::WriteBufferRect, Cmd),
      Dst(Dst),
      Src(Src) { }

  void *GetTarget() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Dst);
    EnqueueWriteBufferRect &Cmd = GetQueueCommandAs<EnqueueWriteBufferRect>();
    
    return reinterpret_cast<void *>(Base + Cmd.GetTargetOffset());
  }

  const void *GetSource() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Src);
    EnqueueWriteBufferRect &Cmd = GetQueueCommandAs<EnqueueWriteBufferRect>();

    return reinterpret_cast<const void *>(Base + Cmd.GetSourceOffset());
  }
    
  size_t GetTargetRowPitch() {
    return GetQueueCommandAs<EnqueueWriteBufferRect>().GetTargetRowPitch();
  }
  
  size_t GetTargetSlicePitch() {
  return GetQueueCommandAs<EnqueueWriteBufferRect>().GetTargetSlicePitch();  
  }
  
  size_t GetSourceRowPitch() {
  return GetQueueCommandAs<EnqueueWriteBufferRect>().GetSourceRowPitch();
  }
  
  size_t GetSourceSlicePitch() {
    return GetQueueCommandAs<EnqueueWriteBufferRect>().GetSourceSlicePitch();
  }
  
  const size_t *GetRegion() {
    return GetQueueCommandAs<EnqueueWriteBufferRect>().GetRegion();
  }
  
private:
  void *Dst;
  const void *Src;
};

class CopyBufferRectCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::CopyBufferRect;
  }
  
public:
  CopyBufferRectCPUCommand(EnqueueCopyBufferRect &Cmd,
                            void *Dst,
                            void *Src)
    : CPUExecCommand(CPUCommand::CopyBufferRect, Cmd),
      Dst(Dst),
      Src(Src) { }

  void *GetTarget() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Dst);
    EnqueueCopyBufferRect &Cmd = GetQueueCommandAs<EnqueueCopyBufferRect>();
    
    return reinterpret_cast<void *>(Base + Cmd.GetTargetOffset());
  }

  void *GetSource() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Src);
    EnqueueCopyBufferRect &Cmd = GetQueueCommandAs<EnqueueCopyBufferRect>();

    return reinterpret_cast<void *>(Base + Cmd.GetSourceOffset());
  }
  
  size_t GetTargetRowPitch() {
    return GetQueueCommandAs<EnqueueCopyBufferRect>().GetTargetRowPitch();
  }
  
  size_t GetTargetSlicePitch() {
  return GetQueueCommandAs<EnqueueCopyBufferRect>().GetTargetSlicePitch();  
  }
  
  size_t GetSourceRowPitch() {
  return GetQueueCommandAs<EnqueueCopyBufferRect>().GetSourceRowPitch();
  }
  
  size_t GetSourceSlicePitch() {
    return GetQueueCommandAs<EnqueueCopyBufferRect>().GetSourceSlicePitch();
  }
  
  const size_t *GetRegion() {
    return GetQueueCommandAs<EnqueueCopyBufferRect>().GetRegion();
  }
  
private:
  void *Dst;
  void *Src;
};

class FillBufferCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::FillBuffer;
  }

public:
  FillBufferCPUCommand(EnqueueFillBuffer &Cmd,
                       void *Dst)
    : CPUExecCommand(CPUCommand::FillBuffer, Cmd),
      Dst(Dst) { }
      
  void *GetTarget() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Dst);
    EnqueueFillBuffer &Cmd = GetQueueCommandAs<EnqueueFillBuffer>();
    
    return reinterpret_cast<void *>(Base + Cmd.GetTargetOffset());  
  }
  
  const void *GetSource() {
    return GetQueueCommandAs<EnqueueFillBuffer>().GetSource();
  }

  size_t GetTargetSize() {
    return GetQueueCommandAs<EnqueueFillBuffer>().GetTargetSize();
  }

  size_t GetSourceSize() {
    return GetQueueCommandAs<EnqueueFillBuffer>().GetSourceSize();
  }
  
private:
  void *Dst;
};

class FillImageCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::FillImage;
  }

public:
  FillImageCPUCommand(EnqueueFillImage &Cmd,
                      void *Dst)
    : CPUExecCommand(CPUCommand::FillImage, Cmd),
      Dst(Dst) { }
      
  void *GetTarget() {
    uintptr_t Base = reinterpret_cast<uintptr_t>(Dst);
    EnqueueFillImage &Cmd = GetQueueCommandAs<EnqueueFillImage>();
    
    return reinterpret_cast<void *>(Base + Cmd.GetTargetOffset());  
  }
  
  const void *GetSource() {
    return GetQueueCommandAs<EnqueueFillImage>().GetSource();
  }

  size_t *GetTargetRegion() {
    return GetQueueCommandAs<EnqueueFillImage>().GetTargetRegion();
  }

  size_t GetTargetRowPitch() {
    return GetQueueCommandAs<EnqueueFillImage>().GetTargetRowPitch();
  }

  size_t GetTargetSlicePitch() {
    return GetQueueCommandAs<EnqueueFillImage>().GetTargetSlicePitch();
  }

  size_t GetTargetElementSize() {
    return GetQueueCommandAs<EnqueueFillImage>().GetTarget().GetElementSize();
  }

  cl_image_format GetTargetImageFormat() {
    return GetQueueCommandAs<EnqueueFillImage>().GetTarget().GetImageFormat();
  }
  
private:
  void *Dst;
};

class NDRangeKernelBlockCPUCommand : public CPUMultiExecCommand {
public:
  class DeviceImage {
  public:
    DeviceImage(const Image &Img, void *ImgData)
      : image_channel_order(Img.GetImageFormat().image_channel_order),
        image_channel_data_type(Img.GetImageFormat().image_channel_data_type),
        num_channels(Img.GetNumChannels()),
        element_size(Img.GetElementSize()),
        width(Img.GetWidth()),
        height(Img.GetHeight()),
        depth(Img.GetDepth()),
        row_pitch(Img.GetRowPitch()),
        slice_pitch(Img.GetSlicePitch()),
        array_size(Img.GetArraySize()),
        num_mip_levels(Img.GetNumMipLevels()),
        num_samples(Img.GetNumSamples()),
        data(ImgData) { }

  private:
    cl_channel_order image_channel_order;
    cl_channel_type image_channel_data_type;
    cl_uint num_channels;
    cl_uint element_size;

    cl_uint width;
    cl_uint height;
    cl_uint depth;

    cl_uint row_pitch;
    cl_uint slice_pitch;

    cl_uint array_size;
   
    cl_uint num_mip_levels;
    cl_uint num_samples;

    void *data; // Pointer to actual image data in __global AS.
  };

  typedef cl_uint DeviceSampler;

public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::NDRangeKernelBlock;
  }

public:
  typedef void (*Signature) (void **);

  typedef llvm::DenseMap<unsigned, void *> ArgsMappings;
  typedef llvm::SmallVector<DeviceImage *, 4> DeviceImagesContainer;
  typedef llvm::SmallVector<DeviceSampler *, 4> DeviceSamplersContainer;
  typedef llvm::SmallVector<std::pair<unsigned, size_t>, 4> IndexOffsetVector;
  typedef llvm::SmallVector<void *, 8> StaticLocalPointers;

public:
  DimensionInfo::iterator index_begin() { return Start; }
  DimensionInfo::iterator index_end() { return End; }

public:
  NDRangeKernelBlockCPUCommand(EnqueueNDRangeKernel &Cmd,
                               Signature Entry,
                               ArgsMappings &GlobalArgs,
                               DimensionInfo::iterator I,
                               DimensionInfo::iterator E,
                               size_t StaticLocalSize,
                               IndexOffsetVector &StaticLocalInfos,
                               CPUCommand::ResultRecorder &Result);

  ~NDRangeKernelBlockCPUCommand();

public:
  void SetLocalParams(LocalMemory &Local, StaticLocalPointers &StaticLocalPtrs);

public:
  Signature &GetFunction() { return Entry; }
  void **GetArgumentsPointer() { return Args; }
  unsigned GetStaticLocalSize() const { return StaticLocalSize; }

  Kernel &GetKernel() {
    return GetQueueCommandAs<EnqueueNDRangeKernel>().GetKernel();
  }

  DimensionInfo &GetDimensionInfo() {
    return GetQueueCommandAs<EnqueueNDRangeKernel>().GetDimensionInfo();
  }

private:
  cl_uint GetDeviceSampler(const Sampler &Smplr);

private:
  Signature Entry;
  void **Args;
  size_t StaticLocalSize;
  IndexOffsetVector StaticLocalInfos;

  DeviceImagesContainer DevImgs;
  DeviceSamplersContainer DevSmplrs;

  DimensionInfo::iterator Start;
  DimensionInfo::iterator End;
};

class NativeKernelCPUCommand : public CPUExecCommand {
public:
  static bool classof(const CPUCommand *Cmd) {
    return Cmd->GetType() == CPUCommand::NativeKernel;
  }

public:
  typedef EnqueueNativeKernel::Signature Signature;

public:
  NativeKernelCPUCommand(EnqueueNativeKernel &Cmd)
    : CPUExecCommand(CPUCommand::NativeKernel, Cmd) { }

public:
  Signature GetFunction() const {
    return GetQueueCommandAs<EnqueueNativeKernel>().GetFunction();
  }

  void *GetArgumentsPointer() const {
    return GetQueueCommandAs<EnqueueNativeKernel>().GetArgumentsPointer();
  }
};

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_COMMAND_H
