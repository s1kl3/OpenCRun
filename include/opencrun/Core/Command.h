
#ifndef OPENCRUN_CORE_COMMAND_H
#define OPENCRUN_CORE_COMMAND_H

#include "CL/opencl.h"

#include "opencrun/Core/Kernel.h"
#include "opencrun/Util/DimensionInfo.h"

#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/SmallVector.h"

#include <map>

namespace opencrun {

class Context;
class CommandQueue;
class Buffer;
class Image;
class Device;
class Event;
class InternalEvent;
class MemoryObj;

template<class ImgCmdBuilderType>
ImgCmdBuilderType &CheckDevImgSupport(
    ImgCmdBuilderType &,
    CommandQueue *,
    Image *);

template<class ImgCmdBuilderType>
bool IsValidImgRegion(
  ImgCmdBuilderType &,
  Image *,
  const size_t *,
  const size_t *);

class Command {
public:
  enum Type {
    NDRangeKernel = CL_COMMAND_NDRANGE_KERNEL,
    NativeKernel = CL_COMMAND_NATIVE_KERNEL,
    ReadBuffer = CL_COMMAND_READ_BUFFER,
    WriteBuffer = CL_COMMAND_WRITE_BUFFER,
    CopyBuffer = CL_COMMAND_COPY_BUFFER,
    ReadImage = CL_COMMAND_READ_IMAGE,
    WriteImage = CL_COMMAND_WRITE_IMAGE,
    CopyImage = CL_COMMAND_COPY_IMAGE,
    CopyImageToBuffer = CL_COMMAND_COPY_IMAGE_TO_BUFFER,
    CopyBufferToImage = CL_COMMAND_COPY_BUFFER_TO_IMAGE,
    MapBuffer = CL_COMMAND_MAP_BUFFER,
    UnmapMemObject = CL_COMMAND_UNMAP_MEM_OBJECT,
    ReadBufferRect = CL_COMMAND_READ_BUFFER_RECT,
    WriteBufferRect = CL_COMMAND_WRITE_BUFFER_RECT,
    CopyBufferRect = CL_COMMAND_COPY_BUFFER_RECT,
    FillBuffer = CL_COMMAND_FILL_BUFFER
  };

public:
  typedef llvm::SmallVector<Event *, 8> EventsContainer;

  typedef EventsContainer::iterator event_iterator;
  typedef EventsContainer::const_iterator const_event_iterator;

public:
  event_iterator wait_begin() { return WaitList.begin(); }
  event_iterator wait_end() { return WaitList.end(); }

  const_event_iterator wait_begin() const { return WaitList.begin(); }
  const_event_iterator wait_end() const { return WaitList.end(); }

public:
  virtual ~Command() { }

protected:
  Command(Type CommandTy,
          EventsContainer& WaitList,
          bool Blocking = false) : CommandTy(CommandTy),
                                   WaitList(WaitList),
                                   Blocking(Blocking) { }

public:
  void SetNotifyEvent(InternalEvent &Ev) { NotifyEv = &Ev; }
  InternalEvent &GetNotifyEvent() { return *NotifyEv; }

  Type GetType() const { return CommandTy; }
  Context &GetContext() const;

public:
  bool CanRun() const;
  bool IsProfiled() const;
  bool IsBlocking() const { return Blocking; }

protected:
  Type CommandTy;
  EventsContainer WaitList;
  bool Blocking;

  InternalEvent *NotifyEv;
};

class EnqueueReadBuffer : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::ReadBuffer;
  }

private:
  EnqueueReadBuffer(void *Target,
                    Buffer &Source,
                    bool Blocking,
                    size_t Offset,
                    size_t Size,
                    EventsContainer &WaitList);

public:
  void *GetTarget() { return Target; }
  Buffer &GetSource() { return *Source; }
  size_t GetOffset() { return Offset; }
  size_t GetSize() { return Size; }

private:
  void *Target;
  llvm::IntrusiveRefCntPtr<Buffer> Source;
  size_t Offset;
  size_t Size;

  friend class EnqueueReadBufferBuilder;
};

class EnqueueWriteBuffer : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::WriteBuffer;
  }

private:
  EnqueueWriteBuffer(Buffer &Target,
                     const void *Source,
                     bool Blocking,
                     size_t Offset,
                     size_t Size,
                     EventsContainer &WaitList);

public:
  Buffer &GetTarget() { return *Target; }
  const void *GetSource() { return Source; }
  size_t GetOffset() { return Offset; }
  size_t GetSize() { return Size; }

private:
  llvm::IntrusiveRefCntPtr<Buffer> Target;
  const void *Source;
  size_t Offset;
  size_t Size;

  friend class EnqueueWriteBufferBuilder;
};

class EnqueueCopyBuffer : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::CopyBuffer;
  }
  
private:
  EnqueueCopyBuffer(Buffer &Target,
                    Buffer &Source,
                    size_t TargetOffset,
                    size_t SourceOffset,
                    size_t Size,
                    EventsContainer &WaitList);
                    
public:
  Buffer &GetTarget() { return *Target; }
  Buffer &GetSource() { return *Source; }
  size_t GetTargetOffset() { return TargetOffset; }
  size_t GetSourceOffset() { return SourceOffset; }
  size_t GetSize() { return Size; }
  
private:
  llvm::IntrusiveRefCntPtr<Buffer> Target;
  llvm::IntrusiveRefCntPtr<Buffer> Source;
  size_t TargetOffset;
  size_t SourceOffset;
  size_t Size;
  
  friend class EnqueueCopyBufferBuilder;
};

class EnqueueReadImage : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::ReadImage;
  }

private:
  EnqueueReadImage(void *Target,
                   Image &Source,
                   bool Blocking,
                   const size_t *Region,
                   size_t SourceOffset,
                   size_t *TargetPitches,
                   EventsContainer &WaitList);

public:
  void *GetTarget() { return Target; }
  Image &GetSource() { return *Source; }
  const size_t *GetRegion() { return Region; }
  size_t GetSourceOffset() { return SourceOffset; }
  size_t GetTargetRowPitch() { return TargetPitches[0]; }
  size_t GetTargetSlicePitch() { return TargetPitches[1]; }
  size_t GetSourceRowPitch() { return Source->GetRowPitch(); }
  size_t GetSourceSlicePitch() { return Source->GetSlicePitch(); }

private:
  void *Target;
  llvm::IntrusiveRefCntPtr<Image> Source;
  size_t SourceOffset;
  size_t Region[3];
  size_t TargetPitches[2];

  friend class EnqueueReadImageBuilder;
};

class EnqueueWriteImage : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::WriteImage;
  }

private:
  EnqueueWriteImage(Image &Target,
                    const void *Source,
                    bool Blocking,
                    const size_t *Region, 
                    size_t TargetOffset, 
                    size_t *SourcePitches, 
                    EventsContainer &WaitList);

public:
  Image &GetTarget() { return *Target; }
  const void *GetSource() { return Source; }
  const size_t *GetRegion() { return Region; }
  size_t GetTargetOffset() { return TargetOffset; }
  size_t GetTargetRowPitch() { return Target->GetRowPitch(); }
  size_t GetTargetSlicePitch() { return Target->GetSlicePitch(); }
  size_t GetSourceRowPitch() { return SourcePitches[0]; }
  size_t GetSourceSlicePitch() { return SourcePitches[1]; }

private:
  llvm::IntrusiveRefCntPtr<Image> Target;
  const void *Source;
  size_t TargetOffset;
  size_t Region[3];
  size_t SourcePitches[2];

  friend class EnqueueWriteImageBuilder;
};

class EnqueueCopyImage : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::CopyImage;
  }

private:
  EnqueueCopyImage(Image &Target,
                   Image &Source,
                   size_t TargetOffset,
                   size_t SourceOffset,
                   const size_t *Region,
                   EventsContainer &WaitList);

public:
  Image &GetTarget() { return *Target; }
  Image &GetSource() { return *Source; }
  size_t GetTargetOffset() { return TargetOffset; }
  size_t GetSourceOffset() { return SourceOffset; }
  size_t GetTargetRowPitch() { return Target->GetRowPitch(); }
  size_t GetTargetSlicePitch() { return Target->GetSlicePitch(); }
  size_t GetSourceRowPitch() { return Source->GetRowPitch(); }
  size_t GetSourceSlicePitch() { return Source->GetSlicePitch(); }
  const size_t *GetRegion() { return Region; }

private:
  llvm::IntrusiveRefCntPtr<Image> Target;
  llvm::IntrusiveRefCntPtr<Image> Source;
  size_t TargetOffset;
  size_t SourceOffset;
  size_t Region[3];

  friend class EnqueueCopyImageBuilder;
};

class EnqueueCopyImageToBuffer : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::CopyImageToBuffer;
  }

private:
  EnqueueCopyImageToBuffer(Buffer &Target,
                           Image &Source,
                           size_t TargetOffset,
                           size_t SourceOffset,
                           const size_t *Region,
                           EventsContainer &WaitList);

public:
  Buffer &GetTarget() { return *Target; }
  Image &GetSource() { return *Source; }
  size_t GetTargetOffset() { return TargetOffset; }
  size_t GetSourceOffset() { return SourceOffset; }
  size_t GetSourceRowPitch() { return Source->GetRowPitch(); }
  size_t GetSourceSlicePitch() { return Source->GetSlicePitch(); }
  const size_t *GetRegion() { return Region; }

private:
  llvm::IntrusiveRefCntPtr<Buffer> Target;
  llvm::IntrusiveRefCntPtr<Image> Source;
  size_t TargetOffset;
  size_t SourceOffset;
  size_t Region[3];

  friend class EnqueueCopyImageToBufferBuilder;
};

class EnqueueCopyBufferToImage : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::CopyBufferToImage;
  }

private:
  EnqueueCopyBufferToImage(Image &Target,
                           Buffer &Source,
                           size_t TargetOffset,
                           const size_t *Region,
                           size_t SourceOffset,
                           EventsContainer &WaitList);

public:
  Image &GetTarget() { return *Target; }
  Buffer &GetSource() { return *Source; }
  size_t GetTargetOffset() { return TargetOffset; }
  size_t GetSourceOffset() { return SourceOffset; }
  size_t GetTargetRowPitch() { return Target->GetRowPitch(); }
  size_t GetTargetSlicePitch() { return Target->GetSlicePitch(); }
  const size_t *GetRegion() { return Region; }

private:
  llvm::IntrusiveRefCntPtr<Image> Target;
  llvm::IntrusiveRefCntPtr<Buffer> Source;
  size_t TargetOffset;
  size_t SourceOffset;
  size_t Region[3];

  friend class EnqueueCopyBufferToImageBuilder;
};

class EnqueueMapBuffer : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::MapBuffer;
  }
  
private:
  EnqueueMapBuffer(Buffer &Source,
                   bool Blocking,
                   cl_map_flags MapFlags,
                   size_t Offset,
                   size_t Size,
                   void *MapBuf,
                   EventsContainer &WaitList);

public:
  Buffer &GetSource() { return *Source; }
  cl_map_flags GetMapFlags() { return MapFlags; }
  size_t GetOffset() { return Offset; }
  size_t GetSize() { return Size; }
  void *GetMapBuffer() { return MapBuf; }

public:
  bool IsMapRead() const { return MapFlags & CL_MAP_READ; }
  bool IsMapWrite() const { return MapFlags & CL_MAP_WRITE; }
  bool IsMapInvalidate() const { return MapFlags & CL_MAP_WRITE_INVALIDATE_REGION; }
  
public:
  void SetMapBuffer(void *MapBuf) { this->MapBuf = MapBuf;}
  
private:
  llvm::IntrusiveRefCntPtr<Buffer> Source;
  cl_map_flags MapFlags;
  size_t Offset;
  size_t Size;

  void *MapBuf;
  
  friend class EnqueueMapBufferBuilder;	
};

class EnqueueUnmapMemObject : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::UnmapMemObject;
  }
  
private:
  EnqueueUnmapMemObject(MemoryObj &MemObj,
                        void *MappedPtr,
                        EventsContainer &WaitList);
  
public:
  MemoryObj &GetMemObj() { return *MemObj; }
  void *GetMappedPtr() { return MappedPtr; }

private:
  llvm::IntrusiveRefCntPtr<MemoryObj> MemObj;
  void *MappedPtr;
  
  friend class EnqueueUnmapMemObjectBuilder;
};

class EnqueueReadBufferRect : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::ReadBufferRect;
  }

private:
  EnqueueReadBufferRect(void *Target,
                        Buffer &Source,
                        bool Blocking,
                        const size_t *Region,
                        size_t TargetOffset,
                        size_t SourceOffset,
                        size_t *TargetPitches,
                        size_t *SourcePitches,
                        EventsContainer &WaitList);

public:
  void *GetTarget() { return Target; }
  Buffer &GetSource() { return *Source; }
  const size_t *GetRegion() { return Region; }
  size_t GetTargetOffset() { return TargetOffset; }
  size_t GetSourceOffset() { return SourceOffset; }
  size_t GetTargetRowPitch() { return TargetPitches[0]; }
  size_t GetTargetSlicePitch() { return TargetPitches[1]; }
  size_t GetSourceRowPitch() { return SourcePitches[0]; }
  size_t GetSourceSlicePitch() { return SourcePitches[1]; }

private:
  void *Target;
  llvm::IntrusiveRefCntPtr<Buffer> Source;
  size_t Region[3];
  size_t TargetOffset;
  size_t SourceOffset;
  size_t TargetPitches[2];
  size_t SourcePitches[2];
  
  friend class EnqueueReadBufferRectBuilder;
};

class EnqueueWriteBufferRect : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::WriteBufferRect;
  }

private:
  EnqueueWriteBufferRect(Buffer &Target,
                         const void *Source,
                         bool Blocking,
                         const size_t *Region,
                         size_t TargetOffset,
                         size_t SourceOffset,
                         size_t *TargetPitches,
                         size_t *SourcePitches,
                         EventsContainer &WaitList);

public:
  Buffer &GetTarget() { return *Target; }
  const void *GetSource() { return Source; }
  const size_t *GetRegion() { return Region; }
  size_t GetTargetOffset() { return TargetOffset; }
  size_t GetSourceOffset() { return SourceOffset; }
  size_t GetTargetRowPitch() { return TargetPitches[0]; }
  size_t GetTargetSlicePitch() { return TargetPitches[1]; }
  size_t GetSourceRowPitch() { return SourcePitches[0]; }
  size_t GetSourceSlicePitch() { return SourcePitches[1]; }

private:
  llvm::IntrusiveRefCntPtr<Buffer> Target;
  const void *Source;
  size_t Region[3];
  size_t TargetOffset;
  size_t SourceOffset;
  size_t TargetPitches[2];
  size_t SourcePitches[2];
  
  friend class EnqueueWriteBufferRectBuilder;
};

class EnqueueCopyBufferRect : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::CopyBufferRect;
  }

public:
  EnqueueCopyBufferRect(Buffer &Target,
                        Buffer &Source,
                        const size_t *Region,
                        size_t TargetOffset,
                        size_t SourceOffset,
                        size_t *TargetPitches,
                        size_t *SourcePitches,
                        EventsContainer &WaitList);
                        
public:
  Buffer &GetTarget() { return *Target; }
  Buffer &GetSource() { return *Source; }
  const size_t *GetRegion() { return Region; }
  size_t GetTargetOffset() { return TargetOffset; }
  size_t GetSourceOffset() { return SourceOffset; }
  size_t GetTargetRowPitch() { return TargetPitches[0]; }
  size_t GetTargetSlicePitch() { return TargetPitches[1]; }
  size_t GetSourceRowPitch() { return SourcePitches[0]; }
  size_t GetSourceSlicePitch() { return SourcePitches[1]; }

private:
  llvm::IntrusiveRefCntPtr<Buffer> Target;
  llvm::IntrusiveRefCntPtr<Buffer> Source;
  size_t Region[3];
  size_t TargetOffset;
  size_t SourceOffset;
  size_t TargetPitches[2];
  size_t SourcePitches[2];
  
  friend class EnqueueCopyBufferRectBuilder;
};

class EnqueueFillBuffer : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::FillBuffer;
  }
  
private:
  EnqueueFillBuffer(Buffer &Target,
                    const void *Source,
                    size_t SourceSize,
                    size_t TargetOffset,
                    size_t TargetSize,
                    EventsContainer &WaitList);
public:
  ~EnqueueFillBuffer();
                    
public:
  Buffer &GetTarget() { return *Target; }
  const void *GetSource() { return Source; }
  size_t GetSourceSize() { return SourceSize; }
  size_t GetTargetOffset() { return TargetOffset; }
  size_t GetTargetSize() { return TargetSize; }
  
private:
  llvm::IntrusiveRefCntPtr<Buffer> Target;
  const void *Source;
  size_t SourceSize;
  size_t TargetOffset;
  size_t TargetSize;
  
  friend class EnqueueFillBufferBuilder;
};

class EnqueueNDRangeKernel : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::NDRangeKernel;
  }

private:
  EnqueueNDRangeKernel(Kernel &Kern,
                       DimensionInfo &DimInfo,
                       EventsContainer &WaitList);

public:
  Kernel &GetKernel() { return *Kern; }
  DimensionInfo &GetDimensionInfo() { return DimInfo; }

  unsigned GetWorkGroupsCount() const { return DimInfo.GetWorkGroupsCount(); }

public:
  bool IsLocalWorkGroupSizeSpecified() const {
    return DimInfo.IsLocalWorkGroupSizeSpecified();
  }

private:
  llvm::IntrusiveRefCntPtr<Kernel> Kern;
  DimensionInfo DimInfo;

  friend class EnqueueNDRangeKernelBuilder;
};

class EnqueueNativeKernel : public Command {
public:
  static bool classof(const Command *Cmd) {
    return Cmd->GetType() == Command::NativeKernel;
  }

public:
  typedef void (*Signature)(void*);
  typedef std::pair<void *, size_t> Arguments;
  typedef std::map<MemoryObj *, void *> MappingsContainer;

private:
  EnqueueNativeKernel(Signature &Func,
                      Arguments &RawArgs,
                      MappingsContainer &Mappings,
                      EventsContainer &WaitList);
  ~EnqueueNativeKernel();

public:
  void RemapMemoryObjAddresses(const MappingsContainer &GlobalMappings);

public:
  Signature &GetFunction() { return Func; }
  void *GetArgumentsPointer() { return RawArgs.first; }

private:
  Signature Func;
  Arguments RawArgs;
  MappingsContainer Mappings;

  friend class EnqueueNativeKernelBuilder;
};

class CommandBuilder {
public:
  enum Type {
    EnqueueNDRangeKernelBuilder,
    EnqueueNativeKernelBuilder,
    EnqueueReadBufferBuilder,
    EnqueueWriteBufferBuilder,
    EnqueueCopyBufferBuilder,
    EnqueueReadImageBuilder,
    EnqueueCopyImageBuilder,
    EnqueueCopyImageToBufferBuilder,
    EnqueueCopyBufferToImageBuilder,
    EnqueueWriteImageBuilder,
    EnqueueMapBufferBuilder,
    EnqueueUnmapMemObjectBuilder,
    EnqueueReadBufferRectBuilder,
    EnqueueWriteBufferRectBuilder,
    EnqueueCopyBufferRectBuilder,
    EnqueueFillBufferBuilder
  };

protected:
  CommandBuilder(Type BldTy, Context &Ctx) : Ctx(Ctx),
                                             ErrCode(CL_SUCCESS),
                                             BldTy(BldTy) { }

public:
  CommandBuilder &SetWaitList(unsigned N, const cl_event *Evs);

public:
  Type GetType() const { return BldTy; }

  bool IsWaitListInconsistent() const;

protected:
  CommandBuilder &NotifyError(cl_int ErrCode, const char *Msg = "");

protected:
  Context &Ctx;
  Command::EventsContainer WaitList;

  cl_int ErrCode;

private:
  Type BldTy;
};

class EnqueueReadBufferBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueReadBufferBuilder;
  }

public:
  EnqueueReadBufferBuilder(Context &Ctx, cl_mem Buf, void *Target);

public:
  EnqueueReadBufferBuilder &SetBlocking(bool Blocking = true);
  EnqueueReadBufferBuilder &SetCopyArea(size_t Offset, size_t Size);
  EnqueueReadBufferBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueReadBuffer *Create(cl_int *ErrCode);

private:
  EnqueueReadBufferBuilder &NotifyError(cl_int ErrCode,
                                        const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  Buffer *Source;
  void *Target;
  bool Blocking;
  size_t Offset;
  size_t Size;
};

class EnqueueWriteBufferBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueWriteBufferBuilder;
  }

public:
  EnqueueWriteBufferBuilder(Context &Ctx, cl_mem Buf, const void *Source);

public:
  EnqueueWriteBufferBuilder &SetBlocking(bool Blocking = true);
  EnqueueWriteBufferBuilder &SetCopyArea(size_t Offset, size_t Size);
  EnqueueWriteBufferBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueWriteBuffer *Create(cl_int *ErrCode);

private:
  EnqueueWriteBufferBuilder &NotifyError(cl_int ErrCode,
                                         const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  Buffer *Target;
  const void *Source;
  bool Blocking;
  size_t Offset;
  size_t Size;
};

class EnqueueCopyBufferBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueCopyBufferBuilder;
  }

public:
  EnqueueCopyBufferBuilder(Context &Ctx, cl_mem TargetBuf, cl_mem SourceBuf);

public:
  EnqueueCopyBufferBuilder &SetCopyArea(size_t TargetOffset, size_t SourceOffset, size_t Size);
  EnqueueCopyBufferBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueCopyBuffer *Create(cl_int *ErrCode);

private:
  EnqueueCopyBufferBuilder &NotifyError(cl_int ErrCode,
                                         const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  Buffer *Target;
  Buffer *Source;
  size_t TargetOffset;
  size_t SourceOffset;
  size_t Size;
};

class EnqueueReadImageBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueReadImageBuilder;
  }

public:
  EnqueueReadImageBuilder(CommandQueue *Queue, cl_mem Img, void *Target);

public:
  EnqueueReadImageBuilder &SetBlocking(bool Blocking = true);
  EnqueueReadImageBuilder &SetCopyArea(const size_t *Origin,
                                       const size_t *Region,
                                       size_t RowPitch,
                                       size_t SlicePitch);
  EnqueueReadImageBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueReadImage *Create(cl_int *ErrCode);

private:
  EnqueueReadImageBuilder &NotifyError(cl_int ErrCode,
                                       const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  Image *Source;
  void *Target;
  bool Blocking;
  const size_t *Region;
  size_t SourceOffset;
  size_t TargetPitches[2];

  friend EnqueueReadImageBuilder &CheckDevImgSupport<>(
      EnqueueReadImageBuilder &,
      CommandQueue *,
      Image *);

  friend bool IsValidImgRegion<>(
      EnqueueReadImageBuilder &,
      Image *,
      const size_t *,
      const size_t *);
};

class EnqueueWriteImageBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueWriteImageBuilder;
  }

public:
  EnqueueWriteImageBuilder(CommandQueue *Queue, cl_mem Img, const void *Source);

public:
  EnqueueWriteImageBuilder &SetBlocking(bool Blocking = true);
  EnqueueWriteImageBuilder &SetCopyArea(const size_t *Origin, 
                                        const size_t *Region, 
                                        size_t RowPitch, 
                                        size_t SlicePitch);
  EnqueueWriteImageBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueWriteImage *Create(cl_int *ErrCode);

private:
  EnqueueWriteImageBuilder &NotifyError(cl_int ErrCode,
                                       const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  Image *Target;
  const void *Source;
  bool Blocking;
  const size_t *Region;
  size_t TargetOffset;
  size_t SourcePitches[2];

  friend EnqueueWriteImageBuilder &CheckDevImgSupport<>(
      EnqueueWriteImageBuilder &,
      CommandQueue *,
      Image *);

  friend bool IsValidImgRegion<>(
      EnqueueWriteImageBuilder &,
      Image *,
      const size_t *,
      const size_t *);
};

class EnqueueCopyImageBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueCopyImageBuilder;
  }

public:
  EnqueueCopyImageBuilder(CommandQueue *Cmd, cl_mem TargetImg, cl_mem SourceImg);

public:
  EnqueueCopyImageBuilder &SetCopyArea(const size_t *TargetOrigin,
                                       const size_t *SourceOrigin,
                                       const size_t *Region);
  EnqueueCopyImageBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueCopyImage *Create(cl_int *ErrCode);

private:
  EnqueueCopyImageBuilder &NotifyError(cl_int ErrCode,
                                       const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  Image *Target;
  Image *Source;
  size_t TargetOffset;
  size_t SourceOffset;
  const size_t *Region;

  friend EnqueueCopyImageBuilder &CheckDevImgSupport<>(
      EnqueueCopyImageBuilder &,
      CommandQueue *,
      Image *);

  friend bool IsValidImgRegion<>(
      EnqueueCopyImageBuilder &,
      Image *,
      const size_t *,
      const size_t *);
};

class EnqueueCopyImageToBufferBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueCopyImageToBufferBuilder;
  }

public:
  EnqueueCopyImageToBufferBuilder(CommandQueue *Queue, cl_mem TargetBuf, cl_mem SourceImg);

public:
  EnqueueCopyImageToBufferBuilder &SetCopyArea(size_t TargetOffset,
                                               const size_t *SourceOrigin,
                                               const size_t *Region);
  EnqueueCopyImageToBufferBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueCopyImageToBuffer *Create(cl_int *ErrCode);

private:
  EnqueueCopyImageToBufferBuilder &NotifyError(cl_int ErrCode,
                                               const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  Buffer *Target;
  Image *Source;
  size_t TargetOffset;
  size_t SourceOffset;
  const size_t *Region;

  friend EnqueueCopyImageToBufferBuilder &CheckDevImgSupport<>(
      EnqueueCopyImageToBufferBuilder &,
      CommandQueue *,
      Image *);

  friend bool IsValidImgRegion<>(
      EnqueueCopyImageToBufferBuilder &,
      Image *,
      const size_t *,
      const size_t *);
};

class EnqueueCopyBufferToImageBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueCopyBufferToImageBuilder;
  }

public:
  EnqueueCopyBufferToImageBuilder(CommandQueue *Queue, cl_mem TargetImg, cl_mem SourceBuf);

public:
  EnqueueCopyBufferToImageBuilder &SetCopyArea(const size_t *TargetOrigin,
                                               const size_t *Region,
                                               size_t SourceOffset);
  EnqueueCopyBufferToImageBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueCopyBufferToImage *Create(cl_int *ErrCode);

private:
  EnqueueCopyBufferToImageBuilder &NotifyError(cl_int ErrCode,
                                               const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  Image *Target;
  Buffer *Source;
  size_t TargetOffset;
  const size_t *Region;
  size_t SourceOffset;

  friend EnqueueCopyBufferToImageBuilder &CheckDevImgSupport<>(
      EnqueueCopyBufferToImageBuilder &,
      CommandQueue *,
      Image *);

  friend bool IsValidImgRegion<>(
      EnqueueCopyBufferToImageBuilder &,
      Image *,
      const size_t *,
      const size_t *);
};

class EnqueueMapBufferBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueMapBufferBuilder;
  }

public:
  EnqueueMapBufferBuilder(Context &Ctx, cl_mem Buf);
  
public:
  EnqueueMapBufferBuilder &SetBlocking(bool Blocking = true);
  EnqueueMapBufferBuilder &SetMapFlags(cl_map_flags MapFlags);
  EnqueueMapBufferBuilder &SetMapArea(size_t Offset, size_t Size);
  EnqueueMapBufferBuilder &SetWaitList(unsigned N, const cl_event *Evs);
  EnqueueMapBufferBuilder &SetMapBuffer(void *MapBuf);
  
  EnqueueMapBuffer *Create(cl_int *ErrCode);

private:
  EnqueueMapBufferBuilder &NotifyError(cl_int ErrCode,
                                       const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }
  
private:
  Buffer *Source;
  bool Blocking;
  cl_map_flags MapFlags;
  size_t Offset;
  size_t Size;
  void *MapBuf;
};

class EnqueueUnmapMemObjectBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueUnmapMemObjectBuilder;
  }
  
public:
  EnqueueUnmapMemObjectBuilder(Context &Ctx, cl_mem MemObj, void *MappedPtr);
  
public:
  EnqueueUnmapMemObjectBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueUnmapMemObject *Create(cl_int *ErrCode);

private:
  EnqueueUnmapMemObjectBuilder &NotifyError(cl_int ErrCode,
                                            const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  MemoryObj *MemObj;
  void *MappedPtr;
};

class EnqueueReadBufferRectBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueReadBufferRectBuilder;
  }

public:
  EnqueueReadBufferRectBuilder(Context &Ctx, cl_mem Buf, void *Ptr);
  
public:
  EnqueueReadBufferRectBuilder &SetBlocking(bool Blocking = true);
  EnqueueReadBufferRectBuilder &SetRegion(const size_t *Region);
  EnqueueReadBufferRectBuilder &SetTargetOffset(const size_t *TargetOrigin,
                                                size_t TargetRowPitch,
                                                size_t TargetSlicePitch);
  EnqueueReadBufferRectBuilder &SetSourceOffset(const size_t *SourceOrigin,
                                                size_t SourceRowPitch,
                                                size_t SourceSlicePitch);
  EnqueueReadBufferRectBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueReadBufferRect *Create(cl_int *ErrCode);

private:
  EnqueueReadBufferRectBuilder &NotifyError(cl_int ErrCode,
                                            const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  void *Target;
  Buffer *Source;
  bool Blocking;
  const size_t *Region;
  const size_t *TargetOrigin;
  const size_t *SourceOrigin;
  size_t TargetPitches[2];
  size_t SourcePitches[2];
  size_t TargetOffset;
  size_t SourceOffset;
};

class EnqueueWriteBufferRectBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueWriteBufferRectBuilder;
  }

public:
  EnqueueWriteBufferRectBuilder(Context &Ctx, cl_mem Buf, const void *Ptr);

public:
  EnqueueWriteBufferRectBuilder &SetBlocking(bool Blocking = true);
  EnqueueWriteBufferRectBuilder &SetRegion(const size_t *Region);
  EnqueueWriteBufferRectBuilder &SetTargetOffset(const size_t *TargetOrigin,
                                                 size_t TargetRowPitch,
                                                 size_t TargetSlicePitch);
  EnqueueWriteBufferRectBuilder &SetSourceOffset(const size_t *SourceOrigin,
                                                 size_t SourceRowPitch,
                                                 size_t SourceSlicePitch);  
  EnqueueWriteBufferRectBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueWriteBufferRect *Create(cl_int *ErrCode);

private:
  EnqueueWriteBufferRectBuilder &NotifyError(cl_int ErrCode,
                                             const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  Buffer *Target;
  const void *Source;
  bool Blocking;
  const size_t *Region;
  const size_t *TargetOrigin;
  const size_t *SourceOrigin;
  size_t TargetPitches[2];
  size_t SourcePitches[2];
  size_t TargetOffset;
  size_t SourceOffset;  
};

class EnqueueCopyBufferRectBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueCopyBufferRectBuilder;
  }
  
public:
  EnqueueCopyBufferRectBuilder(Context &Ctx, cl_mem TargetBuf, cl_mem SourceBuf);
  
public:
  EnqueueCopyBufferRectBuilder &SetRegion(const size_t *Region);
  EnqueueCopyBufferRectBuilder &SetTargetOffset(const size_t *TargetOrigin,
                                                size_t TargetRowPitch,
                                                size_t TargetSlicePitch);
  EnqueueCopyBufferRectBuilder &SetSourceOffset(const size_t *SourceOrigin,
                                                size_t SourceRowPitch,
                                                size_t SourceSlicePitch);
  EnqueueCopyBufferRectBuilder &CheckCopyOverlap();
  EnqueueCopyBufferRectBuilder &SetWaitList(unsigned N, const cl_event *Evs);                                                
  
  EnqueueCopyBufferRect *Create(cl_int *ErrCode);

private:
  EnqueueCopyBufferRectBuilder &NotifyError(cl_int ErrCode,
                                            const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }
  
private:
  Buffer *Target;
  Buffer *Source;
  const size_t *Region;
  const size_t *TargetOrigin;
  const size_t *SourceOrigin;
  size_t TargetPitches[2];
  size_t SourcePitches[2];
  size_t TargetOffset;
  size_t SourceOffset;
};

class EnqueueFillBufferBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueFillBufferBuilder;
  }
  
public:
  EnqueueFillBufferBuilder(Context &Ctx, cl_mem Buf, const void *Pattern);
  
public:
  EnqueueFillBufferBuilder &SetPatternSize(size_t PatternSize);
  EnqueueFillBufferBuilder &SetFillRegion(size_t Offset, size_t Size);
  EnqueueFillBufferBuilder &SetWaitList(unsigned N, const cl_event *Evs);
  
  EnqueueFillBuffer *Create(cl_int *ErrCode);
  
private:
  EnqueueFillBufferBuilder &NotifyError(cl_int ErrCode,
                                            const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }  
  
private:
  Buffer *Target;
  const void *Source;
  size_t SourceSize;
  size_t TargetOffset;
  size_t TargetSize;
};

class EnqueueNDRangeKernelBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueNDRangeKernelBuilder;
  }

public:
  EnqueueNDRangeKernelBuilder(Context &Ctx,
                              Device &Dev,
                              cl_kernel Kern,
                              unsigned WorkDimensions,
                              const size_t *GlobalWorkSizes);

public:
  EnqueueNDRangeKernelBuilder &
  SetGlobalWorkOffset(const size_t *GlobalWorkOffsets);

  EnqueueNDRangeKernelBuilder &SetLocalWorkSize(const size_t *LocalWorkSizes);
  EnqueueNDRangeKernelBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueNDRangeKernel *Create(cl_int *ErrCode);

private:
  EnqueueNDRangeKernelBuilder &NotifyError(cl_int ErrCode,
                                           const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  Device &Dev;
  Kernel *Kern;

  unsigned WorkDimensions;
  llvm::SmallVector<size_t, 4> GlobalWorkSizes;
  llvm::SmallVector<size_t, 4> GlobalWorkOffsets;
  llvm::SmallVector<size_t, 4> LocalWorkSizes;
};

class EnqueueNativeKernelBuilder : public CommandBuilder {
public:
  static bool classof(const CommandBuilder *Bld) {
    return Bld->GetType() == CommandBuilder::EnqueueNativeKernelBuilder;
  }

public:
  EnqueueNativeKernelBuilder(Context &Ctx,
                             EnqueueNativeKernel::Signature Func,
                             EnqueueNativeKernel::Arguments &RawArgs);

public:
  EnqueueNativeKernelBuilder &SetMemoryMappings(unsigned N,
                                                const cl_mem *MemObjs,
                                                const void **MemLocs);
  EnqueueNativeKernelBuilder &SetWaitList(unsigned N, const cl_event *Evs);

  EnqueueNativeKernel *Create(cl_int *ErrCode);

private:
  EnqueueNativeKernelBuilder &NotifyError(cl_int ErrCode,
                                          const char *Msg = "") {
    CommandBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  EnqueueNativeKernel::Signature Func;
  EnqueueNativeKernel::Arguments RawArgs;
  EnqueueNativeKernel::MappingsContainer Mappings;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_COMMAND_H
