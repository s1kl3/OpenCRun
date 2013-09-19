
#include "opencrun/Core/Command.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/Device.h"
#include "opencrun/Core/Kernel.h"
#include "opencrun/Core/MemoryObj.h"
#include "opencrun/Core/Event.h"
#include "opencrun/System/OS.h"

#include "llvm/Support/ErrorHandling.h"

using namespace opencrun;

//
// Command implementation.
//

Context &Command::GetContext() const {
  if(!NotifyEv)
    llvm_unreachable("Command without context");

  return NotifyEv->GetContext();
}

bool Command::CanRun() const {
  for(const_event_iterator I = WaitList.begin(), E = WaitList.end();
                           I != E;
                           ++I)
    if(!(*I)->HasCompleted())
      return false;

  return true;
}

bool Command::IsProfiled() const {
  return NotifyEv && NotifyEv->IsProfiled();
}

//
// EnqueueReadBuffer implementation.
//

EnqueueReadBuffer::EnqueueReadBuffer(void *Target,
                                     Buffer &Src,
                                     bool Blocking,
                                     size_t Offset,
                                     size_t Size,
                                     EventsContainer &WaitList)
  : Command(Command::ReadBuffer, WaitList, Blocking),
    Target(Target),
    Src(&Src),
    Offset(Offset),
    Size(Size) { }

//
// EnqueueWriteBuffer implementation.
//

EnqueueWriteBuffer::EnqueueWriteBuffer(Buffer &Target,
                                       const void *Src,
                                       bool Blocking,
                                       size_t Offset,
                                       size_t Size,
                                       EventsContainer &WaitList)
  : Command(Command::WriteBuffer, WaitList, Blocking),
    Target(&Target),
    Src(Src),
    Offset(Offset),
    Size(Size) { }

//
// EnqueueCopyBuffer implementation.
//

EnqueueCopyBuffer::EnqueueCopyBuffer(Buffer &Src,
																		 Buffer &Dst,
																		 size_t Src_Offset,
																		 size_t Dst_Offset,
																		 size_t Size,
																		 EventsContainer &WaitList)
	: Command(Command::CopyBuffer, WaitList, true),
		Target(&Dst),
		Source(&Src),
		Target_Offset(Dst_Offset),
		Source_Offset(Src_Offset),
		Size(Size) { }

//
// EnqueueMapBuffer implementation.
//

EnqueueMapBuffer::EnqueueMapBuffer(Buffer &Src,
																	 bool Blocking,
																	 cl_map_flags MapFlags,
																	 size_t Offset,
																	 size_t Size,
                                   void *MapBuf,
																	 EventsContainer &WaitList)
	:	Command(Command::MapBuffer, WaitList, Blocking),
		Source(&Src),
		MapFlags(MapFlags),
		Offset(Offset),
		Size(Size),
		MapBuf(MapBuf) { }

//
// EnqueueUnmapMemObject implementation.
//

EnqueueUnmapMemObject::EnqueueUnmapMemObject(MemoryObj &MemObj,
                                             void *MappedPtr,
                                             EventsContainer &WaitList)
	:	Command(Command::UnmapMemObject, WaitList),
		MemObj(&MemObj),
    MappedPtr(MappedPtr) { }    
    
//
// EnqueueNDRangeKernel implementation.
//

EnqueueNDRangeKernel::EnqueueNDRangeKernel(Kernel &Kern,
                                           DimensionInfo &DimInfo,
                                           EventsContainer &WaitList)
  : Command(Command::NDRangeKernel, WaitList),
    Kern(&Kern),
    DimInfo(DimInfo) { }

//
// EnqueueNativeKernel implementation.
//

EnqueueNativeKernel::EnqueueNativeKernel(Signature &Func,
                                         Arguments &RawArgs,
                                         MappingsContainer &Mappings,
                                         EventsContainer &WaitList) :
  Command(Command::NativeKernel, WaitList),
  Func(Func) {
  void *ArgCopy = sys::Alloc(RawArgs.second);

  // Copy arguments.
  std::memcpy(ArgCopy, RawArgs.first, RawArgs.second);
  
  // Re-base pointers.
  uintptr_t Base = reinterpret_cast<uintptr_t>(ArgCopy);
  for(MappingsContainer::iterator I = Mappings.begin(),
                                  E = Mappings.end();
                                  I != E;
                                  ++I) {
    ptrdiff_t Offset = reinterpret_cast<uintptr_t>(I->second) -
                       reinterpret_cast<uintptr_t>(RawArgs.first);
    this->Mappings[I->first] = reinterpret_cast<void *>(Base + Offset);
  }

  this->RawArgs = std::make_pair(ArgCopy, RawArgs.second);
}

EnqueueNativeKernel::~EnqueueNativeKernel() {
  sys::Free(RawArgs.first);
}

void EnqueueNativeKernel::RemapMemoryObjAddresses(
                            const MappingsContainer &GlobalMappings) {
  MappingsContainer::const_iterator J, T = GlobalMappings.end();

  for(MappingsContainer::iterator I = Mappings.begin(),
                                  E = Mappings.end();
                                  I != E;
                                  ++I) {
    J = GlobalMappings.find(I->first);
    *static_cast<void **>(I->second) = J != T ? J->second : NULL;
  }
}

//
// CommandBuilder implementation.
//

#define RETURN_WITH_ERROR(VAR) \
  {                            \
  if(VAR)                      \
    *VAR = this->ErrCode;      \
  return NULL;                 \
  }

CommandBuilder &CommandBuilder::SetWaitList(unsigned N, const cl_event *Evs) {
  if(N && !Evs)
    return NotifyError(CL_INVALID_EVENT_WAIT_LIST, "expected events wait list");

  if(!N && Evs)
    return NotifyError(CL_INVALID_EVENT_WAIT_LIST,
                       "unexpected events wait list");

  for(unsigned I = 0; I < N; ++I) {
    if(!Evs[I])
      return NotifyError(CL_INVALID_EVENT_WAIT_LIST, "invalid event");

    WaitList.push_back(llvm::cast<Event>(Evs[I]));
  }

  return *this;
}

bool CommandBuilder::IsWaitListInconsistent() const {
  for(Command::const_event_iterator I = WaitList.begin(),
                                    E = WaitList.end();
                                    I != E;
                                    ++I)
    if((*I)->IsError())
      return true;

  return false;
}

CommandBuilder &CommandBuilder::NotifyError(cl_int ErrCode, const char *Msg) {
  Ctx.ReportDiagnostic(Msg);
  this->ErrCode = ErrCode;

  return *this;
}

//
// EnqueueReadBufferBuilder implementation.
//

EnqueueReadBufferBuilder::EnqueueReadBufferBuilder(
  Context &Ctx,
  cl_mem Buf,
  void *Target) : CommandBuilder(CommandBuilder::EnqueueReadBufferBuilder,
                                 Ctx),
                  Src(NULL),
                  Target(Target),
                  Blocking(false),
                  Offset(0),
                  Size(0) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is null");

  else if(!(Src = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(Buf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is not a buffer");

  if(!Target)
    NotifyError(CL_INVALID_VALUE, "pointer to data sink is null");
}

EnqueueReadBufferBuilder &EnqueueReadBufferBuilder::SetBlocking(
  bool Blocking) {
  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  this->Blocking = Blocking;

  return *this;
}

EnqueueReadBufferBuilder &EnqueueReadBufferBuilder::SetCopyArea(size_t Offset,
                                                                size_t Size) {
  if(!Src)
    return *this;

  if(Offset + Size > Src->GetSize())
    return NotifyError(CL_INVALID_VALUE, "out of bounds buffer read");

  // TODO: checking for sub-buffers.

  this->Offset = Offset;
  this->Size = Size;

  return *this;
}

EnqueueReadBufferBuilder &EnqueueReadBufferBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  return llvm::cast<EnqueueReadBufferBuilder>(Super);
}

EnqueueReadBuffer *EnqueueReadBufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueReadBuffer(Target, *Src, Blocking, Offset, Size, WaitList);
}

//
// EnqueueWriteBufferBuilder implementation.
//

EnqueueWriteBufferBuilder::EnqueueWriteBufferBuilder(
  Context &Ctx,
  cl_mem Buf,
  const void *Src) : CommandBuilder(CommandBuilder::EnqueueWriteBufferBuilder,
                                    Ctx),
                     Target(NULL),
                     Src(Src),
                     Blocking(false),
                     Offset(0),
                     Size(0) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is null");

  else if(!(Target = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(Buf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is not a buffer");

  if(!Src)
    NotifyError(CL_INVALID_VALUE, "pointer to data source is null");
}

EnqueueWriteBufferBuilder &EnqueueWriteBufferBuilder::SetBlocking(
  bool Blocking) {
  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  this->Blocking = Blocking;

  return *this;
}

EnqueueWriteBufferBuilder &EnqueueWriteBufferBuilder::SetCopyArea(size_t Offset,
                                                                  size_t Size) {
  if(!Target)
    return *this;

  if(Offset + Size > Target->GetSize())
    return NotifyError(CL_INVALID_VALUE, "data size exceed buffer capacity");

  // TODO: checking for sub-buffers.

  this->Offset = Offset;
  this->Size = Size;

  return *this;
}

EnqueueWriteBufferBuilder &EnqueueWriteBufferBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  return llvm::cast<EnqueueWriteBufferBuilder>(Super);
}

EnqueueWriteBuffer *EnqueueWriteBufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueWriteBuffer(*Target, Src, Blocking, Offset, Size, WaitList);
}

//
// EnqueueCopyBufferBuilder implementation.
//

EnqueueCopyBufferBuilder::EnqueueCopyBufferBuilder(
	Context &Ctx,
	cl_mem DstBuf,
	cl_mem SrcBuf) : CommandBuilder(CommandBuilder::EnqueueCopyBufferBuilder,
																	Ctx),
									 Target(NULL),
									 Source(NULL),
									 Target_Offset(0),
									 Source_Offset(0),
									 Size(0) {
  if(!DstBuf)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is null");
	
	else if(!SrcBuf)
		NotifyError(CL_INVALID_MEM_OBJECT, "copy source is null");

  else if(!(Target = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(DstBuf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is not a buffer");
		
	else if(!(Source = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(SrcBuf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source is not a buffer");				 
}

EnqueueCopyBufferBuilder &EnqueueCopyBufferBuilder::SetCopyArea(size_t DstOffset,
																																size_t SrcOffset,
																																size_t Size) {
  if(!Target || !Source)
    return *this;

  if(DstOffset + Size > Target->GetSize())
    return NotifyError(CL_INVALID_VALUE, "data size exceed destination buffer capacity");

	if(SrcOffset + Size > Source->GetSize())
		return NotifyError(CL_INVALID_VALUE, "data size exceed source buffer capacity");
		
  // TODO: checking for sub-buffers.

  this->Target_Offset = DstOffset;
	this->Source_Offset = SrcOffset;
  this->Size = Size;

  return *this;																																
}

EnqueueCopyBufferBuilder &EnqueueCopyBufferBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  return llvm::cast<EnqueueCopyBufferBuilder>(Super);
}

EnqueueCopyBuffer *EnqueueCopyBufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueCopyBuffer(*Target, *Source, Source_Offset, Target_Offset, Size, WaitList);
}

//
// EnqueueMapBufferBuilder implementation.
//

EnqueueMapBufferBuilder::EnqueueMapBufferBuilder(
	Context &Ctx,
	cl_mem Buf) : CommandBuilder(CommandBuilder::EnqueueMapBufferBuilder,
															 Ctx),
								Source(NULL),
								Blocking(false),
                MapFlags(0),
								Offset(0),
								Size(0),
								MapBuf(NULL) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "map source is null");

  else if(!(Source = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(Buf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "map source is not a buffer");	
    
  else if(Ctx != Source->GetContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and buffer have different context");
}

EnqueueMapBufferBuilder &EnqueueMapBufferBuilder::SetBlocking(bool Blocking) {
  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  this->Blocking = Blocking;

  return *this;
}

EnqueueMapBufferBuilder &EnqueueMapBufferBuilder::SetMapFlags(cl_map_flags MapFlags) {
	if(MapFlags & CL_MAP_READ) {
		if(MapFlags & CL_MAP_WRITE_INVALIDATE_REGION)
			return NotifyError(CL_INVALID_VALUE, "invalid flag combination");

		if(Source->GetHostAccessProtection() == MemoryObj::HostWriteOnly ||
		   Source->GetHostAccessProtection() == MemoryObj::HostNoAccess)
			return NotifyError(CL_INVALID_OPERATION, "invalid map operation");		
	}
	
	if(MapFlags & CL_MAP_WRITE) {
		if(MapFlags & CL_MAP_WRITE_INVALIDATE_REGION)
			return NotifyError(CL_INVALID_VALUE, "invalid flag combination");

		if(Source->GetHostAccessProtection() == MemoryObj::HostReadOnly ||
		   Source->GetHostAccessProtection() == MemoryObj::HostNoAccess)
			return NotifyError(CL_INVALID_OPERATION, "invalid map operation");
	}
	
	if(MapFlags & CL_MAP_WRITE_INVALIDATE_REGION) {
		if((MapFlags & CL_MAP_READ) || (MapFlags & CL_MAP_WRITE))
			return NotifyError(CL_INVALID_VALUE, "invalid flag combination");

		if(Source->GetHostAccessProtection() == MemoryObj::HostReadOnly ||
		   Source->GetHostAccessProtection() == MemoryObj::HostNoAccess)
			return NotifyError(CL_INVALID_OPERATION, "invalid map operation");
	}

	this->MapFlags = MapFlags;

	return *this;
}

EnqueueMapBufferBuilder &EnqueueMapBufferBuilder::SetMapArea(size_t Offset, 
																														 size_t Size) {
  if(!Source)
    return *this;

  if(Offset + Size > Source->GetSize())
    return NotifyError(CL_INVALID_VALUE, "out of bounds buffer mapping");

  // TODO: checking for sub-buffers.

  this->Offset = Offset;
  this->Size = Size;

  return *this;
}

EnqueueMapBufferBuilder &EnqueueMapBufferBuilder::SetWaitList(unsigned N, const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  return llvm::cast<EnqueueMapBufferBuilder>(Super);
}

EnqueueMapBufferBuilder &EnqueueMapBufferBuilder::SetMapBuffer(void *MapBuf) {
	if(!MapBuf)
		return *this;
		
	this->MapBuf = MapBuf;
	
	return *this;
}

EnqueueMapBuffer *EnqueueMapBufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueMapBuffer(*Source, Blocking, MapFlags, Offset, Size, MapBuf, WaitList);
}


//
// EnqueueUnmapMemObjectBuilder implementation.
//

EnqueueUnmapMemObjectBuilder::EnqueueUnmapMemObjectBuilder(
	Context &Ctx, 
	cl_mem MemObj, 
	void *MappedPtr) : CommandBuilder(CommandBuilder::EnqueueUnmapMemObjectBuilder,
																		Ctx),
										 MemObj(llvm::cast<MemoryObj>(MemObj)),
										 MappedPtr(MappedPtr) {
  if(!MemObj)
    NotifyError(CL_INVALID_MEM_OBJECT, "memory object is null");
  
  else if(!MappedPtr)
    NotifyError(CL_INVALID_VALUE, "pointer to mapped data is null");

  else if(Ctx != llvm::cast<MemoryObj>(MemObj)->GetContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and buffer have different context");
}

EnqueueUnmapMemObjectBuilder &
EnqueueUnmapMemObjectBuilder::SetWaitList(unsigned N, const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  return llvm::cast<EnqueueUnmapMemObjectBuilder>(Super);
}

EnqueueUnmapMemObject *EnqueueUnmapMemObjectBuilder::Create(cl_int *ErrCode) {
	if(this->ErrCode != CL_SUCCESS)
		RETURN_WITH_ERROR(ErrCode);
		
	if(ErrCode)
		*ErrCode = CL_SUCCESS;
		
	return new EnqueueUnmapMemObject(*MemObj, MappedPtr, WaitList);
}

//
// EnqueueNDRangeKernelBuilder implementation.
//

EnqueueNDRangeKernelBuilder::EnqueueNDRangeKernelBuilder(
  Context &Ctx,
  Device &Dev,
  cl_kernel Kern,
  unsigned WorkDimensions,
  const size_t *GlobalWorkSizes) :
  CommandBuilder(CommandBuilder::EnqueueNDRangeKernelBuilder, Ctx),
  Dev(Dev),
  WorkDimensions(0) {
  if(!Kern)
    NotifyError(CL_INVALID_KERNEL, "cannot enqueue an invalid kernel");
  else
    this->Kern = llvm::cast<Kernel>(Kern);

  if(!this->Kern->IsBuiltFor(Dev))
    NotifyError(CL_INVALID_PROGRAM_EXECUTABLE,
                "kernel not built for current device");

  if(this->Kern->GetContext() != Ctx)
    NotifyError(CL_INVALID_CONTEXT,
                "cannot enqueue a kernel into a command queue with "
                "a different context");

  if(!this->Kern->AreAllArgsSpecified())
    NotifyError(CL_INVALID_KERNEL_ARGS,
                "not all kernel arguments have been specified");

  if(WorkDimensions < 1 || WorkDimensions > Dev.GetMaxWorkItemDimensions())
    NotifyError(CL_INVALID_WORK_DIMENSION,
                "given work dimensions is out of range");
  else
    this->WorkDimensions = WorkDimensions;

  if(!GlobalWorkSizes)
    NotifyError(CL_INVALID_GLOBAL_WORK_SIZE, "invalid global work size given");

  else for(unsigned I = 0; I < WorkDimensions; ++I) {
    if(GlobalWorkSizes[I] == 0 || GlobalWorkSizes[I] > Dev.GetSizeTypeMax()) {
      NotifyError(CL_INVALID_GLOBAL_WORK_SIZE,
                  "out of range global work size given");
      break;
    }

    this->GlobalWorkSizes.push_back(GlobalWorkSizes[I]);
  }
}

EnqueueNDRangeKernelBuilder &
EnqueueNDRangeKernelBuilder::SetGlobalWorkOffset(
  const size_t *GlobalWorkOffsets) {
  if(!WorkDimensions || GlobalWorkSizes.empty())
    return *this;

  if(!GlobalWorkOffsets)
    this->GlobalWorkOffsets.assign(WorkDimensions, 0);

  else for(unsigned I = 0; I < WorkDimensions; ++I) {
    if(GlobalWorkOffsets[I] + GlobalWorkSizes[I] > Dev.GetSizeTypeMax()) {
      NotifyError(CL_INVALID_GLOBAL_OFFSET,
                  "given global offset shift work items out of range");
      break;
    }

    this->GlobalWorkOffsets.push_back(GlobalWorkOffsets[I]);
  }

  return *this;
}

EnqueueNDRangeKernelBuilder &
EnqueueNDRangeKernelBuilder::SetLocalWorkSize(const size_t *LocalWorkSizes) {
  if(!LocalWorkSizes) {
    if(Kern->RequireWorkGroupSizes())
      NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                  "kernel requires fixed local work size");

    if (!Dev.ComputeGlobalWorkPartition(GlobalWorkSizes, this->LocalWorkSizes))
      NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                  "unable to partition global work on device");
    return *this;
  }

  llvm::SmallVector<size_t, 4> &MaxWorkItemSizes = Dev.GetMaxWorkItemSizes();
  size_t WorkGroupSize = 1;

  for(unsigned I = 0; I < WorkDimensions; ++I) {
    if(LocalWorkSizes[I] > MaxWorkItemSizes[I])
      return NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                         "work group size exceed device limits");

    if(GlobalWorkSizes[I] % LocalWorkSizes[I])
      return NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                         "work group size does not divide "
                         "number of work items");

    if(Kern->RequireWorkGroupSizes() &&
       (I >= 3 || Kern->GetRequiredWorkGroupSizes()[I] != LocalWorkSizes[I]))
      return NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                         "work group size does not match "
                         "the one requested by the kernel");

    this->LocalWorkSizes.push_back(LocalWorkSizes[I]);
    WorkGroupSize *= LocalWorkSizes[I];
  }

  if(WorkGroupSize > Dev.GetMaxWorkGroupSize())
    return NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                       "work group size exceed device limits");

  return *this;
}

EnqueueNDRangeKernelBuilder &
EnqueueNDRangeKernelBuilder::SetWaitList(unsigned N, const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  return llvm::cast<EnqueueNDRangeKernelBuilder>(Super);
}

EnqueueNDRangeKernel *EnqueueNDRangeKernelBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  DimensionInfo DimInfo(GlobalWorkOffsets, GlobalWorkSizes, LocalWorkSizes);

  return new EnqueueNDRangeKernel(*Kern, DimInfo, WaitList);
}

//
// EnqueueNativeKernelBuilder implementation.
//

EnqueueNativeKernelBuilder::EnqueueNativeKernelBuilder(
  Context &Ctx,
  EnqueueNativeKernel::Signature Func,
  EnqueueNativeKernel::Arguments &RawArgs) :
  CommandBuilder(CommandBuilder::EnqueueNativeKernelBuilder, Ctx),
  Func(Func),
  RawArgs(RawArgs) {
  if(!Func)
    NotifyError(CL_INVALID_VALUE,
                "cannot build a native kernel without a working function");

  if(!RawArgs.first && RawArgs.second)
    NotifyError(CL_INVALID_VALUE, "expected arguments pointer");
  if(RawArgs.first && !RawArgs.second)
    NotifyError(CL_INVALID_VALUE, "expected arguments size");
}

EnqueueNativeKernelBuilder &EnqueueNativeKernelBuilder::SetMemoryMappings(
  unsigned N,
  const cl_mem *MemObjs,
  const void **MemLocs) {
  if((N && !RawArgs.first) || (!N && (MemObjs || MemLocs)))
    return NotifyError(CL_INVALID_VALUE, "unexpected memory mappings");

  if(N && (!MemObjs || !MemLocs))
    return NotifyError(CL_INVALID_VALUE, "expected memory mappings");

  for(unsigned I = 0; I < N; ++I) {
    if(!MemObjs[I])
      return NotifyError(CL_INVALID_MEM_OBJECT, "invalid memory object");

    MemoryObj *MemObj = llvm::cast<MemoryObj>(MemObjs[I]);
    Mappings[MemObj] = const_cast<void *>(MemLocs[I]);
  }

  return *this;
}

EnqueueNativeKernelBuilder &EnqueueNativeKernelBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  return llvm::cast<EnqueueNativeKernelBuilder>(Super);
}

EnqueueNativeKernel *EnqueueNativeKernelBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueNativeKernel(Func, RawArgs, Mappings, WaitList);
}
