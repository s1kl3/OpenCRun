
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
class Buffer;
class Device;
class Event;
class InternalEvent;
class MemoryObj;

class Command {
public:
  enum Type {
    ReadBuffer = CL_COMMAND_READ_BUFFER,
    WriteBuffer = CL_COMMAND_WRITE_BUFFER,
		CopyBuffer = CL_COMMAND_COPY_BUFFER,
		MapBuffer = CL_COMMAND_MAP_BUFFER,
		UnmapMemObject = CL_COMMAND_UNMAP_MEM_OBJECT,
    NDRangeKernel = CL_COMMAND_NDRANGE_KERNEL,
    NativeKernel = CL_COMMAND_NATIVE_KERNEL
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
                    Buffer &Src,
                    bool Blocking,
                    size_t Offset,
                    size_t Size,
                    EventsContainer &WaitList);

public:
  void *GetTarget() { return Target; }
  Buffer &GetSource() { return *Src; }
  size_t GetOffset() { return Offset; }
  size_t GetSize() { return Size; }

private:
  void *Target;
  llvm::IntrusiveRefCntPtr<Buffer> Src;
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
                     const void *Src,
                     bool Blocking,
                     size_t Offset,
                     size_t Size,
                     EventsContainer &WaitList);

public:
  Buffer &GetTarget() { return *Target; }
  const void *GetSource() { return Src; }
  size_t GetOffset() { return Offset; }
  size_t GetSize() { return Size; }

private:
  llvm::IntrusiveRefCntPtr<Buffer> Target;
  const void *Src;
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
	EnqueueCopyBuffer(Buffer &Src,
										Buffer &Dst,
										size_t Src_Offset,
										size_t Dst_Offset,
										size_t Size,
										EventsContainer &WaitList);
										
public:
	Buffer &GetTarget() { return *Target; }
	Buffer &GetSource() { return *Source; }
	size_t GetTargetOffset() { return Target_Offset; }
	size_t GetSourceOffset() { return Source_Offset; }
	size_t GetSize() { return Size; }
	
private:
	llvm::IntrusiveRefCntPtr<Buffer> Target;
	llvm::IntrusiveRefCntPtr<Buffer> Source;
	size_t Target_Offset;
	size_t Source_Offset;
	size_t Size;
	
	friend class EnqueueCopyBufferBuilder;
};

class EnqueueMapBuffer : public Command {
public:
	static bool classof(const Command *Cmd) {
		return Cmd->GetType() == Command::MapBuffer;
	}
	
private:
	EnqueueMapBuffer(Buffer &Src,
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
    EnqueueReadBufferBuilder,
    EnqueueWriteBufferBuilder,
		EnqueueCopyBufferBuilder,
		EnqueueMapBufferBuilder,
		EnqueueUnmapMemObjectBuilder,
    EnqueueNDRangeKernelBuilder,
    EnqueueNativeKernelBuilder
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
  Buffer *Src;
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
  EnqueueWriteBufferBuilder(Context &Ctx, cl_mem Buf, const void *Src);

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
  const void *Src;
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
  EnqueueCopyBufferBuilder(Context &Ctx, cl_mem DstBuf, cl_mem SrcBuf);

public:
  EnqueueCopyBufferBuilder &SetCopyArea(size_t DstOffset, size_t SrcOffset, size_t Size);
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
  size_t Target_Offset;
	size_t Source_Offset;
  size_t Size;
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
