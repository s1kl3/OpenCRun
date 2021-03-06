
#include "opencrun/Core/Command.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/CommandQueue.h"
#include "opencrun/Core/Device.h"
#include "opencrun/Core/Kernel.h"
#include "opencrun/Core/MemoryObject.h"
#include "opencrun/Core/Event.h"
#include "opencrun/System/OS.h"

#include "llvm/Support/ErrorHandling.h"

#include <algorithm>

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
                                     Buffer &Source,
                                     bool Blocking,
                                     size_t Offset,
                                     size_t Size,
                                     EventsContainer &WaitList)
  : Command(Command::ReadBuffer, WaitList, Blocking),
    Target(Target),
    Source(&Source),
    Offset(Offset),
    Size(Size) { }

//
// EnqueueWriteBuffer implementation.
//

EnqueueWriteBuffer::EnqueueWriteBuffer(Buffer &Target,
                                       const void *Source,
                                       bool Blocking,
                                       size_t Offset,
                                       size_t Size,
                                       EventsContainer &WaitList)
  : Command(Command::WriteBuffer, WaitList, Blocking),
    Target(&Target),
    Source(Source),
    Offset(Offset),
    Size(Size) { }

//
// EnqueueCopyBuffer implementation.
//

EnqueueCopyBuffer::EnqueueCopyBuffer(Buffer &Target,
                                     Buffer &Source,
                                     size_t TargetOffset,
                                     size_t SourceOffset,
                                     size_t Size,
                                     EventsContainer &WaitList)
  : Command(Command::CopyBuffer, WaitList),
    Target(&Target),
    Source(&Source),
    TargetOffset(TargetOffset),
    SourceOffset(SourceOffset),
    Size(Size) { }

//
// EnqueueReadImage implementation.
//

EnqueueReadImage::EnqueueReadImage(void *Target,
                                   Image &Source,
                                   bool Blocking,
                                   const size_t *SourceOrigin,
                                   const size_t *Region,
                                   size_t *TargetPitches,
                                   EventsContainer &WaitList)
  : Command(Command::ReadImage, WaitList, Blocking),
    Target(Target),
    Source(&Source),
    SourceOffset(0) {
  std::memcpy(this->SourceOrigin, SourceOrigin, 3 * sizeof(size_t));
  std::memcpy(this->TargetPitches, TargetPitches, 2 * sizeof(size_t));

  // Convert the region width in bytes.
  this->Region[0] = Region[0] * Source.getElementSize();
  this->Region[1] = Region[1];
  this->Region[2] = Region[2];

  // Calculate offset inside the image object.
  SourceOffset = SourceOrigin[0] * Source.getElementSize() +
                 SourceOrigin[1] * Source.getRowPitch() +
                 SourceOrigin[2] * Source.getSlicePitch();
}

//
// EnqueueWriteImage implementation.
//

EnqueueWriteImage::EnqueueWriteImage(Image &Target, 
                                     const void *Source, 
                                     bool Blocking, 
                                     const size_t *TargetOrigin,
                                     const size_t *Region, 
                                     size_t *SourcePitches, 
                                     EventsContainer &WaitList)
  : Command(Command::WriteImage, WaitList, Blocking),
    Target(&Target),
    Source(Source),
    TargetOffset(0) { 
  std::memcpy(this->TargetOrigin, TargetOrigin, 3 * sizeof(size_t));
  std::memcpy(this->SourcePitches, SourcePitches, 2 * sizeof(size_t));

  // Convert the region width in bytes.
  this->Region[0] = Region[0] * Target.getElementSize();
  this->Region[1] = Region[1];
  this->Region[2] = Region[2];
  
  // Calculate offset inside the image object.
  TargetOffset = TargetOrigin[0] * Target.getElementSize() +
                 TargetOrigin[1] * Target.getRowPitch() +
                 TargetOrigin[2] * Target.getSlicePitch();
}


//
// EnqueueCopyImage implementation.
//

EnqueueCopyImage::EnqueueCopyImage(Image &Target,
                                   Image &Source,
                                   const size_t *TargetOrigin,
                                   const size_t *SourceOrigin,
                                   const size_t *Region,
                                   EventsContainer &WaitList)
  : Command(Command::CopyImage, WaitList),
    Target(&Target),
    Source(&Source),
    TargetOffset(0),
    SourceOffset(0) {
  std::memcpy(this->TargetOrigin, TargetOrigin, 3 * sizeof(size_t));
  std::memcpy(this->SourceOrigin, SourceOrigin, 3 * sizeof(size_t));

  // Convert the region width in bytes. Target and source images have
  // the same element size because they have the same image format.
  this->Region[0] = Region[0] * Target.getElementSize();
  this->Region[1] = Region[1];
  this->Region[2] = Region[2];
  
  // Calculate offset inside the target image object.
  TargetOffset = TargetOrigin[0] * Target.getElementSize() +
                 TargetOrigin[1] * Target.getRowPitch() +
                 TargetOrigin[2] * Target.getSlicePitch();
  
  // Calculate offset inside the source image object.
  SourceOffset = SourceOrigin[0] * Source.getElementSize() +
                 SourceOrigin[1] * Source.getRowPitch() +
                 SourceOrigin[2] * Source.getSlicePitch();
}

//
// EnqueueCopyImageToBuffer implementation.
//
EnqueueCopyImageToBuffer::EnqueueCopyImageToBuffer(
    Buffer &Target,
    Image &Source,
    size_t TargetOffset,
    const size_t *SourceOrigin,
    const size_t *Region,
    EventsContainer &WaitList)
  : Command(Command::CopyImageToBuffer, WaitList),
    Target(&Target),
    Source(&Source),
    TargetOffset(TargetOffset),
    SourceOffset(0) {
  std::memcpy(this->SourceOrigin, SourceOrigin, 3 * sizeof(size_t));

  // Convert the source region width in bytes.
  this->Region[0] = Region[0] * Source.getElementSize();
  this->Region[1] = Region[1];
  this->Region[2] = Region[2];
  
  // Calculate offset inside the source image object.
  SourceOffset = SourceOrigin[0] * Source.getElementSize() +
                 SourceOrigin[1] * Source.getRowPitch() +
                 SourceOrigin[2] * Source.getSlicePitch();
}

//
// EnqueueCopyBufferToImage implementation.
//
EnqueueCopyBufferToImage::EnqueueCopyBufferToImage(
    Image &Target,
    Buffer &Source,
    const size_t *TargetOrigin,
    const size_t *Region,
    size_t SourceOffset,
    EventsContainer &WaitList)
  : Command(Command::CopyBufferToImage, WaitList),
    Target(&Target),
    Source(&Source),
    TargetOffset(0),
    SourceOffset(SourceOffset) {
  std::memcpy(this->TargetOrigin, TargetOrigin, 3 * sizeof(size_t));
  
  // Convert the target region width in bytes.
  this->Region[0] = Region[0] * Target.getElementSize();
  this->Region[1] = Region[1];
  this->Region[2] = Region[2];
  
  // Calculate offset inside the source image object.
  TargetOffset = TargetOrigin[0] * Target.getElementSize() +
                 TargetOrigin[1] * Target.getRowPitch() +
                 TargetOrigin[2] * Target.getSlicePitch();
}

//
// EnqueueMapBuffer implementation.
//

EnqueueMapBuffer::EnqueueMapBuffer(Buffer &Source,
                                   bool Blocking,
                                   cl_map_flags MapFlags,
                                   size_t Offset,
                                   size_t Size,
                                   void *MapBuf,
                                   EventsContainer &WaitList)
  :	Command(Command::MapBuffer, WaitList, Blocking),
    Source(&Source),
    MapFlags(MapFlags),
    Offset(Offset),
    Size(Size),
    MapBuf(MapBuf) { }

//
// EnqueueMapImage implementation.
//

EnqueueMapImage::EnqueueMapImage(Image &Source,
                                 bool Blocking,
                                 cl_map_flags MapFlags,
                                 const size_t *Origin,
                                 const size_t *Region,
                                 void *MapBuf,
                                 size_t *MapPitches,
                                 EventsContainer &WaitList)
  : Command(Command::MapImage, WaitList, Blocking),
    Source(&Source),
    MapFlags(MapFlags),
    MapBuf(MapBuf) {
  std::memcpy(this->Origin, Origin, 3 * sizeof(size_t));
  // Convert the region width in bytes.
  this->Region[0] = Region[0] * Source.getElementSize();
  this->Region[1] = Region[1];
  this->Region[2] = Region[2];
  std::memcpy(this->MapPitches, MapPitches, 2 * sizeof(size_t));
}

//
// EnqueueUnmapMemObject implementation.
//

EnqueueUnmapMemObject::EnqueueUnmapMemObject(MemoryObject &MemObj,
                                             void *MappedPtr,
                                             EventsContainer &WaitList)
  :	Command(Command::UnmapMemObject, WaitList),
    MemObj(&MemObj),
    MappedPtr(MappedPtr) { }

//
// EnqueueMarker implementation.
//

EnqueueMarker::EnqueueMarker(EventsContainer &WaitList)
  : Command(Command::Marker, WaitList) { }

//
// EnqueueReadBufferRect implementation.
//

EnqueueReadBufferRect::EnqueueReadBufferRect(void *Target,
                                             Buffer &Source,
                                             bool Blocking,
                                             const size_t *Region,
                                             size_t TargetOffset,
                                             size_t SourceOffset,
                                             size_t *TargetPitches,
                                             size_t *SourcePitches,
                                             EventsContainer &WaitList)
  : Command(Command::ReadBufferRect, WaitList, Blocking),
  Target(Target),
  Source(&Source),
  TargetOffset(TargetOffset),
  SourceOffset(SourceOffset) {
  std::memcpy(this->Region, Region, 3 * sizeof(size_t));
  std::memcpy(this->TargetPitches, TargetPitches, 2 * sizeof(size_t));
  std::memcpy(this->SourcePitches, SourcePitches, 2 * sizeof(size_t));
}

//
// EnqueueWriteBufferRect implementation.
//

EnqueueWriteBufferRect::EnqueueWriteBufferRect(Buffer &Target,
                                               const void *Source,
                                               bool Blocking,
                                               const size_t *Region,
                                               size_t TargetOffset,
                                               size_t SourceOffset,
                                               size_t *TargetPitches,
                                               size_t *SourcePitches,
                                               EventsContainer &WaitList)
  : Command(Command::WriteBufferRect, WaitList, Blocking),
  Target(&Target),
  Source(Source),
  TargetOffset(TargetOffset),
  SourceOffset(SourceOffset) { 
  std::memcpy(this->Region, Region, 3 * sizeof(size_t));
  std::memcpy(this->TargetPitches, TargetPitches, 2 * sizeof(size_t));
  std::memcpy(this->SourcePitches, SourcePitches, 2 * sizeof(size_t));
}

//
// EnqueueCopyBufferRect implementation.
//

EnqueueCopyBufferRect::EnqueueCopyBufferRect(Buffer &Target,
                                             Buffer &Source,
                                             const size_t *Region,
                                             size_t TargetOffset,
                                             size_t SourceOffset,
                                             size_t *TargetPitches,
                                             size_t *SourcePitches,
                                             EventsContainer &WaitList)
  : Command(Command::CopyBufferRect, WaitList),
  Target(&Target),
  Source(&Source),
  TargetOffset(TargetOffset),
  SourceOffset(SourceOffset) {
  std::memcpy(this->Region, Region, 3 * sizeof(size_t));
  std::memcpy(this->TargetPitches, TargetPitches, 2 * sizeof(size_t));
  std::memcpy(this->SourcePitches, SourcePitches, 2 * sizeof(size_t));
}
    
//
// EnqueueBarrier implementation.
//

EnqueueBarrier::EnqueueBarrier(EventsContainer &WaitList)
  : Command(Command::Barrier, WaitList) { }

//
// EnqueueFillBuffer implementation.
//

EnqueueFillBuffer::EnqueueFillBuffer(Buffer &Target,
                                     const void *Source,
                                     size_t SourceSize,
                                     size_t TargetOffset,
                                     size_t TargetSize,
                                     EventsContainer &WaitList)
  : Command(Command::FillBuffer, WaitList),
    Target(&Target),
    Source(NULL),
    SourceSize(SourceSize),
    TargetOffset(TargetOffset),
    TargetSize(TargetSize) { 
  // The memory associated with pattern can be reused or 
  // freed after the API clEnqueueFillBuffer returns. 
  this->Source = std::malloc(SourceSize);
  std::memcpy(const_cast<void *>(this->Source), Source, SourceSize); 
}                                     

EnqueueFillBuffer::~EnqueueFillBuffer() {
  std::free(const_cast<void *>(Source));
}
    
//
// EnqueueFillImage implementation.
//

EnqueueFillImage ::EnqueueFillImage(Image &Target,
                                    const void *Source,
                                    const size_t *TargetOrigin,
                                    const size_t *TargetRegion,
                                    EventsContainer &WaitList)
  : Command(Command::FillImage, WaitList),
    Target(&Target),
    Source(Source),
    TargetOffset(0) {
  // Source is expected to be a four component RGBA floating-point
  // or integer color value (see section 5.3.4 of the OpenCL 1.2
  // specifications). Thus, its size will be that of a 16 bytes
  // (i.e. sizeof(cl_uint4) or sizeof(cl_float4)).
  this->Source = std::malloc(sizeof(cl_uint4));
  std::memcpy(const_cast<void *>(this->Source), Source, sizeof(cl_uint4));

  std::memcpy(this->TargetOrigin, TargetOrigin, 3 * sizeof(size_t));

  // Convert the region width in bytes.
  this->TargetRegion[0] = TargetRegion[0] * Target.getElementSize();
  this->TargetRegion[1] = TargetRegion[1];
  this->TargetRegion[2] = TargetRegion[2];

  TargetOffset = TargetOrigin[0] * Target.getElementSize() +
                 TargetOrigin[1] * Target.getRowPitch() +
                 TargetOrigin[2] * Target.getSlicePitch();
}                                     

EnqueueFillImage::~EnqueueFillImage() {
  std::free(const_cast<void *>(Source));
}

//
// EnqueueNDRangeKernel implementation.
//

EnqueueNDRangeKernel::EnqueueNDRangeKernel(const Kernel &Kern,
                                           DimensionInfo &DimInfo,
                                           EventsContainer &WaitList)
  : Command(Command::NDRangeKernel, WaitList),
    Kern(Kern),
    DimInfo(DimInfo) {}

//
// EnqueueNativeKernel implementation.
//

EnqueueNativeKernel::EnqueueNativeKernel(Signature Func,
                                         Arguments RawArgs,
                                         const MemoryLocations &Locs,
                                         EventsContainer &WaitList) :
  Command(Command::NativeKernel, WaitList),
  Func(Func), Locs(Locs) {
  void *ArgCopy = sys::Alloc(RawArgs.second);

  // Copy arguments.
  std::memcpy(ArgCopy, RawArgs.first, RawArgs.second);
  
  this->RawArgs = std::make_pair(ArgCopy, RawArgs.second);
}

EnqueueNativeKernel::~EnqueueNativeKernel() {
  sys::Free(RawArgs.first);
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
  CommandQueue &Queue,
  cl_mem Buf,
  void *Target) : CommandBuilder(CommandBuilder::EnqueueReadBufferBuilder,
                                 Queue.GetContext()),
                  Source(NULL),
                  Target(Target),
                  Blocking(false),
                  Offset(0),
                  Size(0) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is null");

  else if(!(Source = llvm::dyn_cast<Buffer>(Buf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is not a buffer");

  else if(Source->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Source->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "source sub-buffer offset is not aligned with device");
  }
  
  else if((Source->getFlags() & MemoryObject::HostWriteOnly) ||
          (Source->getFlags() & MemoryObject::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid read buffer operation");

  else if(Queue.GetContext() != Source->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and buffer have different context");
    
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
  if(!Source)
    return *this;

  if(Offset + Size > Source->getSize())
    return NotifyError(CL_INVALID_VALUE, "out of bounds buffer read");

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

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueReadBufferBuilder>(Super);
}

EnqueueReadBuffer *EnqueueReadBufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueReadBuffer(Target, *Source, Blocking, Offset, Size, WaitList);
}

//
// EnqueueWriteBufferBuilder implementation.
//

EnqueueWriteBufferBuilder::EnqueueWriteBufferBuilder(
  CommandQueue &Queue,
  cl_mem Buf,
  const void *Source) : CommandBuilder(CommandBuilder::EnqueueWriteBufferBuilder,
                                       Queue.GetContext()),
                        Target(NULL),
                        Source(Source),
                        Blocking(false),
                        Offset(0),
                        Size(0) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is null");

  else if(!(Target = llvm::dyn_cast<Buffer>(Buf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is not a buffer");

  else if(Target->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Target->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "target sub-buffer offset is not aligned with device");
  }
 
  else if((Target->getFlags() & MemoryObject::HostReadOnly) ||
          (Target->getFlags() & MemoryObject::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid read buffer operation");

  else if(Queue.GetContext() != Target->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and buffer have different context");
    
  if(!Source)
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

  if(Offset + Size > Target->getSize())
    return NotifyError(CL_INVALID_VALUE, "data size exceeds buffer capacity");

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

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueWriteBufferBuilder>(Super);
}

EnqueueWriteBuffer *EnqueueWriteBufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueWriteBuffer(*Target, Source, Blocking, Offset, Size, WaitList);
}

//
// EnqueueCopyBufferBuilder implementation.
//

EnqueueCopyBufferBuilder::EnqueueCopyBufferBuilder(
  CommandQueue &Queue,
  cl_mem TargetBuf,
  cl_mem SourceBuf) : CommandBuilder(CommandBuilder::EnqueueCopyBufferBuilder,
                                     Queue.GetContext()),
                      Target(NULL),
                      Source(NULL),
                      TargetOffset(0),
                      SourceOffset(0),
                      Size(0) {
  if(!TargetBuf)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is null");
  
  else if(!(Target = llvm::dyn_cast<Buffer>(TargetBuf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is not a buffer");
 
  else if(Target->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Target->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "target sub-buffer offset is not aligned with device");
  }
   
  else if(Queue.GetContext() != Target->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and target buffer have different context");
  
  if(!SourceBuf)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source is null");

  else if(!(Source = llvm::dyn_cast<Buffer>(SourceBuf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source is not a buffer");				 
 
  else if(Source->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Source->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "source sub-buffer offset is not aligned with device");
  }
   
  else if(Queue.GetContext() != Source->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and source buffer have different context"); 
}

EnqueueCopyBufferBuilder &EnqueueCopyBufferBuilder::SetCopyArea(size_t TargetOffset,
                                                                size_t SourceOffset,
                                                                size_t Size) {
  if(!Target || !Source)
    return *this;

  if(TargetOffset + Size > Target->getSize())
    return NotifyError(CL_INVALID_VALUE, "data size exceeds destination buffer capacity");
    
  if(SourceOffset + Size > Source->getSize())
    return NotifyError(CL_INVALID_VALUE, "data size exceeds source buffer capacity");

  // Target and Source are the same buffer or sub-buffer object and the source and
  // destination regions overlap  if((Target == Source) 
  if((Target == Source) &&
     (std::max(SourceOffset, TargetOffset) - std::min(SourceOffset, TargetOffset) < Size))
    return NotifyError(CL_MEM_COPY_OVERLAP, "source and destination regions overlap");

  // Target and Source are different sub-buffers of the same associated buffer object
  // and they overlap.
  if((Target != Source) && (Target->isSubBuffer() && Source->isSubBuffer()) &&
     (Target->getParent() == Source->getParent()) &&
     (std::max(SourceOffset, TargetOffset) - std::min(SourceOffset, TargetOffset) < Size))
    return NotifyError(CL_MEM_COPY_OVERLAP, "source and destination regions overlap");

  this->TargetOffset = TargetOffset;
  this->SourceOffset = SourceOffset;
  this->Size = Size;

  return *this;
}

EnqueueCopyBufferBuilder &EnqueueCopyBufferBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueCopyBufferBuilder>(Super);
}

EnqueueCopyBuffer *EnqueueCopyBufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueCopyBuffer(*Target, *Source, TargetOffset, SourceOffset, Size, WaitList);
}

//
// EnqueueReadImageBuilder implementation.
//

EnqueueReadImageBuilder::EnqueueReadImageBuilder(
  CommandQueue &Queue,
  cl_mem Img,
  void *Target) : CommandBuilder(CommandBuilder::EnqueueReadImageBuilder,
                                 Queue.GetContext()),
                  Source(NULL),
                  Target(Target),
                  Blocking(false),
                  SourceOrigin(NULL),
                  Region(NULL) {
  if(!Img)
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is null");

  else if(!(Source = llvm::dyn_cast<Image>(Img)))
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is not an image");
  
  else if((Source->getFlags() & MemoryObject::HostWriteOnly) ||
          (Source->getFlags() & MemoryObject::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid read image operation");

  else if(Queue.GetContext() != Source->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and image have different context");
    
  if(!Target)
    NotifyError(CL_INVALID_VALUE, "pointer to data sink is null");
  
  if(Source)
    CheckDevImgSupport<>(*this, Queue, Source);
}

EnqueueReadImageBuilder &EnqueueReadImageBuilder::SetBlocking(
  bool Blocking) {
  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  this->Blocking = Blocking;

  return *this;
}

EnqueueReadImageBuilder &EnqueueReadImageBuilder::SetCopyArea(
    const size_t *SourceOrigin,
    const size_t *Region,
    size_t TargetRowPitch,
    size_t TargetSlicePitch) {
  if(!Source)
    return *this;

  if(!SourceOrigin)
    return NotifyError(CL_INVALID_VALUE, "origin is null");

  if(!Region)
    return NotifyError(CL_INVALID_VALUE, "region is null");

  if(Region[0] == 0 || Region[1] == 0 || Region [2] == 0)
    return NotifyError(CL_INVALID_VALUE, "invalid region");

  if(!IsValidImgRegion<>(*this, Source, SourceOrigin, Region))
    return *this;
  
  // Region is valid.
  this->Region = Region;
  this->SourceOrigin = SourceOrigin;

  // Check row and slice picthes.
  if(TargetRowPitch == 0) 
    TargetPitches[0] = Region[0] * Source->getElementSize();
  else {
    if(TargetRowPitch < Region[0] * Source->getElementSize())
      return NotifyError(CL_INVALID_VALUE, "invalid row pitch");
    TargetPitches[0] = TargetRowPitch;
  }

  if(TargetSlicePitch == 0) 
    TargetPitches[1] = TargetPitches[0] * Region[1];
  else {
    if(TargetSlicePitch < TargetPitches[0] * Region[1])
      return NotifyError(CL_INVALID_VALUE, "invalid slice pitch");
    TargetPitches[1] = TargetSlicePitch;
  }

  return *this;
}

EnqueueReadImageBuilder &EnqueueReadImageBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueReadImageBuilder>(Super);
}

EnqueueReadImage *EnqueueReadImageBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueReadImage(Target,
                              *Source,
                              Blocking,
                              SourceOrigin,
                              Region,
                              TargetPitches,
                              WaitList);
}

//
// EnqueueWriteImageBuilder implementation.
//

EnqueueWriteImageBuilder::EnqueueWriteImageBuilder(
  CommandQueue &Queue,
  cl_mem Img,
  const void *Source) : CommandBuilder(CommandBuilder::EnqueueWriteImageBuilder,
                                       Queue.GetContext()),
                        Target(NULL),
                        Source(Source),
                        Blocking(false),
                        TargetOrigin(NULL),
                        Region(NULL) {
  if(!Img)
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is null");

  else if(!(Target = llvm::dyn_cast<Image>(Img)))
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is not an image");
  
  else if((Target->getFlags() & MemoryObject::HostReadOnly) ||
          (Target->getFlags() & MemoryObject::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid write image operation");

  else if(Ctx != Target->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and image have different context");
    
  if(!Target)
    NotifyError(CL_INVALID_VALUE, "pointer to data source is null");

  if(Target)
    CheckDevImgSupport<>(*this, Queue, Target);
}

EnqueueWriteImageBuilder &EnqueueWriteImageBuilder::SetBlocking(
  bool Blocking) {
  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  this->Blocking = Blocking;

  return *this;
}

EnqueueWriteImageBuilder &EnqueueWriteImageBuilder::SetCopyArea(
    const size_t *TargetOrigin,
    const size_t *Region,
    size_t SourceRowPitch,
    size_t SourceSlicePitch) {
  if(!Target)
    return *this;

  if(!TargetOrigin)
    return NotifyError(CL_INVALID_VALUE, "origin is null");

  if(!Region)
    return NotifyError(CL_INVALID_VALUE, "region is null");

  if(Region[0] == 0 || Region[1] == 0 || Region [2] == 0)
    return NotifyError(CL_INVALID_VALUE, "invalid region");

  if(!IsValidImgRegion<>(*this, Target, TargetOrigin, Region))
    return *this;

  // Region is valid.
  this->Region = Region;
  this->TargetOrigin = TargetOrigin;

  // Check row and slice picthes.
  if(SourceRowPitch == 0) 
    SourcePitches[0] = Region[0] * Target->getElementSize();
  else {
    if(SourceRowPitch < Region[0] * Target->getElementSize())
      return NotifyError(CL_INVALID_VALUE, "invalid row pitch");
    SourcePitches[0] = SourceRowPitch;
  }

  if(SourceSlicePitch == 0) 
    SourcePitches[1] = SourcePitches[0] * Region[1];
  else {
    if(SourceSlicePitch < SourcePitches[0] * Region[1])
      return NotifyError(CL_INVALID_VALUE, "invalid slice pitch");
    SourcePitches[1] = SourceSlicePitch;
  }

  return *this;
}

EnqueueWriteImageBuilder &EnqueueWriteImageBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueWriteImageBuilder>(Super);
}

EnqueueWriteImage *EnqueueWriteImageBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueWriteImage(*Target,
                               Source, 
                               Blocking,
                               TargetOrigin,
                               Region,
                               SourcePitches,
                               WaitList);
}

//
// EnqueueCopyImageBuilder implementation.
//

EnqueueCopyImageBuilder::EnqueueCopyImageBuilder(
  CommandQueue &Queue,
  cl_mem TargetImg,
  cl_mem SourceImg) : CommandBuilder(CommandBuilder::EnqueueCopyImageBuilder,
                                     Queue.GetContext()),
                      Target(NULL),
                      Source(NULL),
                      TargetOrigin(NULL),
                      SourceOrigin(NULL),
                      Region(NULL) {
  if(!TargetImg)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is null");

  else if(!(Target = llvm::dyn_cast<Image>(TargetImg)))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is not an image");

  else if(Queue.GetContext() != Target->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and target image have different context");

  if(!SourceImg)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source is null");

  else if(!(Source = llvm::dyn_cast<Image>(SourceImg)))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source is not an image");
  
  else if(Queue.GetContext() != Source->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and source image have different context");

  else if((Source->getChannelOrder() != Target->getChannelOrder()) ||
          (Source->getChannelDataType() != Target->getChannelDataType()))
    NotifyError(CL_IMAGE_FORMAT_MISMATCH, "target and source image use different image formats");

  if(Target)
    CheckDevImgSupport<>(*this, Queue, Target);

  if(Source)
    CheckDevImgSupport<>(*this, Queue, Source);
} 

EnqueueCopyImageBuilder &EnqueueCopyImageBuilder::SetCopyArea(
    const size_t *TargetOrigin,
    const size_t *SourceOrigin,
    const size_t *Region) {
  if(!Target || !Source)
    return *this;

  if(!TargetOrigin || !SourceOrigin)
    return NotifyError(CL_INVALID_VALUE, "origin is null");

  if(!Region)
    return NotifyError(CL_INVALID_VALUE, "region is null");

  if(Region[0] == 0 || Region[1] == 0 || Region [2] == 0)
    return NotifyError(CL_INVALID_VALUE, "invalid region");

  if(!IsValidImgRegion<>(*this, Target, TargetOrigin, Region))
    return *this;

  if(!IsValidImgRegion<>(*this, Source, SourceOrigin, Region))
    return *this;

  // Region is valid for both image objects.
  this->Region = Region;
  this->TargetOrigin = TargetOrigin;
  this->SourceOrigin = SourceOrigin;

  return *this;
}

EnqueueCopyImageBuilder &EnqueueCopyImageBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueCopyImageBuilder>(Super);
}

EnqueueCopyImage *EnqueueCopyImageBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueCopyImage(*Target,
                              *Source,
                              TargetOrigin,
                              SourceOrigin,
                              Region,
                              WaitList);
}

//
// EnqueueCopyImageToBuffer implementation.
//

EnqueueCopyImageToBufferBuilder::EnqueueCopyImageToBufferBuilder(
  CommandQueue &Queue,
  cl_mem TargetBuf,
  cl_mem SourceImg) 
  : CommandBuilder(CommandBuilder::EnqueueCopyImageToBufferBuilder,
                   Queue.GetContext()),
    Target(NULL),
    Source(NULL),
    TargetOffset(0),
    SourceOrigin(NULL),
    Region(NULL) {
  if(!TargetBuf)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is null");

  else if(!(Target = llvm::dyn_cast<Buffer>(TargetBuf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is not a buffer");

  else if(Target->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Target->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "target sub-buffer offset is not aligned with device");
  }

  else if(Queue.GetContext() != Target->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and target buffer have different context");

  if(!SourceImg)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source is null");

  else if(!(Source = llvm::dyn_cast<Image>(SourceImg)))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source is not an image");

  else if(Source->getType() == Image::Image1D_Buffer &&
          Source->getBuffer() == Target)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source created from target buffer");
  
  else if(Queue.GetContext() != Source->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and source image have different context");

  if(Source)
    CheckDevImgSupport(*this, Queue, Source);
}

EnqueueCopyImageToBufferBuilder &EnqueueCopyImageToBufferBuilder::SetCopyArea(
    size_t TargetOffset,
    const size_t *SourceOrigin,
    const size_t *Region) {
  if(!Target || !Source)
    return *this;

  if(!SourceOrigin)
    return NotifyError(CL_INVALID_VALUE, "origin is null");

  if(!Region)
    return NotifyError(CL_INVALID_VALUE, "region is null");

  if(Region[0] == 0 || Region[1] == 0 || Region [2] == 0)
    return NotifyError(CL_INVALID_VALUE, "invalid region");

  if(!IsValidImgRegion<>(*this, Source, SourceOrigin, Region))
    return *this;

  // Region is valid.
  this->Region = Region;
  this->SourceOrigin = SourceOrigin;

  if(TargetOffset + 
     Region[0] * Region[1] * Region[2] * Source->getElementSize() > Target->getSize())
    return NotifyError(CL_INVALID_VALUE, "target region out of bounds");

  this->TargetOffset = TargetOffset;
  
  return *this;
}

EnqueueCopyImageToBufferBuilder &EnqueueCopyImageToBufferBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueCopyImageToBufferBuilder>(Super);
}

EnqueueCopyImageToBuffer *EnqueueCopyImageToBufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueCopyImageToBuffer(*Target,
                                      *Source,
                                      TargetOffset,
                                      SourceOrigin,
                                      Region,
                                      WaitList);
}

//
// EnqueueCopyBufferToImage implementation.
//

EnqueueCopyBufferToImageBuilder::EnqueueCopyBufferToImageBuilder(
  CommandQueue &Queue,
  cl_mem TargetImg,
  cl_mem SourceBuf) 
  : CommandBuilder(CommandBuilder::EnqueueCopyBufferToImageBuilder,
                   Queue.GetContext()),
    Target(NULL),
    Source(NULL),
    TargetOrigin(NULL),
    Region(NULL),
    SourceOffset(0) {
  if(!SourceBuf)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source is null");

  else if(!(Source = llvm::dyn_cast<Buffer>(SourceBuf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source is not an buffer");
 
  else if(Source->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Source->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "source sub-buffer offset is not aligned with device");
  }
 
  else if(Queue.GetContext() != Source->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and source buffer have different context");

  if(!TargetImg)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is null");

  else if(!(Target = llvm::dyn_cast<Image>(TargetImg)))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is not an image");

  else if(Target->getType() == Image::Image1D_Buffer &&
          Target->getBuffer() == Source)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source created from target buffer");

  else if(Queue.GetContext() != Target->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and target image have different context");

  if(Target)
    CheckDevImgSupport<>(*this, Queue, Target);
}

EnqueueCopyBufferToImageBuilder &EnqueueCopyBufferToImageBuilder::SetCopyArea(
    const size_t *TargetOrigin,
    const size_t *Region,
    size_t SourceOffset) {
  if(!Target || !Source)
    return *this;

  if(!TargetOrigin)
    return NotifyError(CL_INVALID_VALUE, "origin is null");

  if(!Region)
    return NotifyError(CL_INVALID_VALUE, "region is null");

  if(Region[0] == 0 || Region[1] == 0 || Region [2] == 0)
    return NotifyError(CL_INVALID_VALUE, "invalid region");

  if(!IsValidImgRegion<>(*this, Target, TargetOrigin, Region))
    return *this;

  // Region is valid.
  this->Region = Region;
  this->TargetOrigin = TargetOrigin;

  if(SourceOffset + 
     Region[0] * Region[1] * Region[2] * Target->getElementSize() > Source->getSize())
    return NotifyError(CL_INVALID_VALUE, "source region out of bounds");

  this->SourceOffset = SourceOffset;
  
  return *this;
}

EnqueueCopyBufferToImageBuilder &EnqueueCopyBufferToImageBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueCopyBufferToImageBuilder>(Super);
}

EnqueueCopyBufferToImage *EnqueueCopyBufferToImageBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueCopyBufferToImage(*Target,
                                      *Source,
                                      TargetOrigin,
                                      Region,
                                      SourceOffset,
                                      WaitList);
}

//
// EnqueueMapBufferBuilder implementation.
//

EnqueueMapBufferBuilder::EnqueueMapBufferBuilder(
  CommandQueue &Queue,
  cl_mem Buf) : CommandBuilder(CommandBuilder::EnqueueMapBufferBuilder,
                               Queue.GetContext()),
                Queue(Queue),
                Source(NULL),
                Blocking(false),
                MapFlags(0),
                Offset(0),
                Size(0),
                MapBuf(NULL) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "map source is null");

  else if(!(Source = llvm::dyn_cast<Buffer>(Buf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "map source is not a buffer");	
 
  else if(Source->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Source->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "source sub-buffer offset is not aligned with device");
  }
   
  else if(Queue.GetContext() != Source->getContext())
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
  if(!Source)
    return *this;

  if(MapFlags & CL_MAP_READ) {
    if(MapFlags & CL_MAP_WRITE_INVALIDATE_REGION)
      return NotifyError(CL_INVALID_VALUE, "invalid flag combination");

    if(Source->getFlags() & MemoryObject::HostWriteOnly ||
       Source->getFlags() & MemoryObject::HostNoAccess)
      return NotifyError(CL_INVALID_OPERATION, "invalid map operation");		
  }
  
  if(MapFlags & CL_MAP_WRITE) {
    if(MapFlags & CL_MAP_WRITE_INVALIDATE_REGION)
      return NotifyError(CL_INVALID_VALUE, "invalid flag combination");

    if(Source->getFlags() & MemoryObject::HostReadOnly ||
       Source->getFlags() & MemoryObject::HostNoAccess)
      return NotifyError(CL_INVALID_OPERATION, "invalid map operation");
  }
  
  if(MapFlags & CL_MAP_WRITE_INVALIDATE_REGION) {
    if((MapFlags & CL_MAP_READ) || (MapFlags & CL_MAP_WRITE))
      return NotifyError(CL_INVALID_VALUE, "invalid flag combination");

    if(Source->getFlags() & MemoryObject::HostReadOnly ||
       Source->getFlags() & MemoryObject::HostNoAccess)
      return NotifyError(CL_INVALID_OPERATION, "invalid map operation");
  }

  this->MapFlags = MapFlags;

  return *this;
}

EnqueueMapBufferBuilder &EnqueueMapBufferBuilder::SetMapArea(size_t Offset, 
                                                             size_t Size) {
  if(!Source)
    return *this;

  if(Offset + Size > Source->getSize())
    return NotifyError(CL_INVALID_VALUE, "out of bounds buffer mapping");

  this->Offset = Offset;
  this->Size = Size;

  return *this;
}

EnqueueMapBufferBuilder &EnqueueMapBufferBuilder::SetMapBuffer(void **MapBuf) {
  if(!Source)
    return *this;

  MemoryObject::MemMappingInfo MapInfo;
  MapInfo.Offset[0] = Offset;
  MapInfo.Size[0] = Size;
  MapInfo.Flags = MapFlags;

  void *MapPtr = Source->acquireMappingAddrFor(Queue.GetDevice());
  if (!Source->addMapping(MapPtr, MapInfo)) {
    Source->releaseMappingAddrFor(Queue.GetDevice());
    return NotifyError(CL_MAP_FAILURE, "cannot get host pointer to mapped data");
  }

  this->MapBuf = MapPtr;
  *MapBuf = MapPtr;

  return *this;
}

EnqueueMapBufferBuilder &EnqueueMapBufferBuilder::SetWaitList(unsigned N, const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueMapBufferBuilder>(Super);
}

EnqueueMapBuffer *EnqueueMapBufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueMapBuffer(*Source, Blocking, MapFlags, Offset, Size, MapBuf, WaitList);
}

//
// EnqueueMapImageBuilder implementation.
//

EnqueueMapImageBuilder::EnqueueMapImageBuilder(
  CommandQueue &Queue,
  cl_mem Img) : CommandBuilder(CommandBuilder::EnqueueMapImageBuilder,
                               Queue.GetContext()),
                Queue(Queue),
                Source(NULL),
                Blocking(false),
                MapFlags(0),
                Origin(NULL),
                Region(NULL),
                MapBuf(NULL) {
  if(!Img)
    NotifyError(CL_INVALID_MEM_OBJECT, "map source is null");

  else if(!(Source = llvm::dyn_cast<Image>(Img)))
    NotifyError(CL_INVALID_MEM_OBJECT, "map source is not an image");	
    
  else if(Queue.GetContext() != Source->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and image have different context");

  if(Source)
    CheckDevImgSupport<>(*this, Queue, Source);
}

EnqueueMapImageBuilder &EnqueueMapImageBuilder::SetBlocking(bool Blocking) {
  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  this->Blocking = Blocking;

  return *this;
}

EnqueueMapImageBuilder &EnqueueMapImageBuilder::SetMapFlags(cl_map_flags MapFlags) {
  if(!Source)
    return *this;

  if(MapFlags & CL_MAP_READ) {
    if(MapFlags & CL_MAP_WRITE_INVALIDATE_REGION)
      return NotifyError(CL_INVALID_VALUE, "invalid flag combination");

    if(Source->getFlags() & MemoryObject::HostWriteOnly ||
       Source->getFlags() & MemoryObject::HostNoAccess)
      return NotifyError(CL_INVALID_OPERATION, "invalid map operation");		
  }
  
  if(MapFlags & CL_MAP_WRITE) {
    if(MapFlags & CL_MAP_WRITE_INVALIDATE_REGION)
      return NotifyError(CL_INVALID_VALUE, "invalid flag combination");

    if(Source->getFlags() & MemoryObject::HostReadOnly ||
       Source->getFlags() & MemoryObject::HostNoAccess)
      return NotifyError(CL_INVALID_OPERATION, "invalid map operation");
  }
  
  if(MapFlags & CL_MAP_WRITE_INVALIDATE_REGION) {
    if((MapFlags & CL_MAP_READ) || (MapFlags & CL_MAP_WRITE))
      return NotifyError(CL_INVALID_VALUE, "invalid flag combination");

    if(Source->getFlags() & MemoryObject::HostReadOnly ||
       Source->getFlags() & MemoryObject::HostNoAccess)
      return NotifyError(CL_INVALID_OPERATION, "invalid map operation");
  }

  this->MapFlags = MapFlags;

  return *this;
}

EnqueueMapImageBuilder &EnqueueMapImageBuilder::SetMapArea(
    const size_t *Origin,
    const size_t *Region) {
  if(!Source)
    return *this;

  if(!Origin)
    return NotifyError(CL_INVALID_VALUE, "origin is null");

  this->Origin = Origin;

  if(!Region)
    return NotifyError(CL_INVALID_VALUE, "region is null");

  if(Region[0] == 0 || Region[1] == 0 || Region [2] == 0)
    return NotifyError(CL_INVALID_VALUE, "invalid region");

  if(!IsValidImgRegion<>(*this, Source, Origin, Region))
    return *this;

  // Region is valid.
  this->Region = Region;
  
  return *this;
}

EnqueueMapImageBuilder &EnqueueMapImageBuilder::SetMapPitches(size_t *ImageRowPitch,
                                                              size_t *ImageSlicePitch) {
  if(!Source || !Region)
    return *this;

  if(!ImageRowPitch)
    return NotifyError(CL_INVALID_VALUE, "image row pitch is null");

  switch(Source->getType()) {
  case Image::Image1D_Array:
  case Image::Image2D_Array:
  case Image::Image3D:
    if(!ImageSlicePitch)
      return NotifyError(CL_INVALID_VALUE, "image slice pitch is null");
    break;
  default:
    break;
  }

  // In this case image object storage area is used as if it
  // was the result of a mapping operation, so we have to deal
  // with extra data outside the specified regions.
  MapPitches[0] = Source->getRowPitch();
  MapPitches[1] = Source->getSlicePitch(); 

  *ImageRowPitch = MapPitches[0];
  *ImageSlicePitch = MapPitches[1];

  return *this;
}

EnqueueMapImageBuilder &EnqueueMapImageBuilder::SetMapBuffer(void **MapBuf) {
  if(!Source || !Origin || !Region)
    return *this;

  MemoryObject::MemMappingInfo MapInfo;

  // Mapping informations for 1d image buffers are identical to
  // those used for buffer objects.
  if(Source->getType() == Image::Image1D_Buffer) {
    MapInfo.Offset[0] = Origin[0] * Source->getElementSize();
    MapInfo.Size[0] = Region[0] * Source->getElementSize();
  } else {
    memcpy(MapInfo.Offset, Origin, sizeof(size_t) * 3);
    memcpy(MapInfo.Size, Region, sizeof(size_t) * 3);
  }

  MapInfo.Flags = MapFlags;

  void *MapPtr = Source->acquireMappingAddrFor(Queue.GetDevice());
  if (!Source->addMapping(MapPtr, MapInfo)) {
    Source->releaseMappingAddrFor(Queue.GetDevice());
    return NotifyError(CL_MAP_FAILURE, "cannot get host pointer to mapped data");
  }
    
  this->MapBuf = MapPtr;
  *MapBuf = MapPtr;

  return *this;
}

EnqueueMapImageBuilder &EnqueueMapImageBuilder::SetWaitList(unsigned N, const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueMapImageBuilder>(Super);
}

EnqueueMapImage *EnqueueMapImageBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueMapImage(*Source, 
                             Blocking,
                             MapFlags,
                             Origin,
                             Region,
                             MapBuf,
                             MapPitches,
                             WaitList);
}

//
// EnqueueUnmapMemObjectBuilder implementation.
//

EnqueueUnmapMemObjectBuilder::EnqueueUnmapMemObjectBuilder(
  Context &Ctx, 
  cl_mem MemObj, 
  void *MappedPtr) : CommandBuilder(CommandBuilder::EnqueueUnmapMemObjectBuilder,
                                    Ctx),
                     MemObj(llvm::cast<MemoryObject>(MemObj)),
                     MappedPtr(MappedPtr) {
  if(!MemObj)
    NotifyError(CL_INVALID_MEM_OBJECT, "memory object is null");
  
  else if(Ctx != llvm::cast<MemoryObject>(MemObj)->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and buffer have different context");
  
  if(!MappedPtr)
    NotifyError(CL_INVALID_VALUE, "pointer to mapped data is null");
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
// EnqueueMarkerBuilder implementation.
//

EnqueueMarkerBuilder::EnqueueMarkerBuilder(CommandQueue &Queue)
  : CommandBuilder(CommandBuilder::EnqueueMarkerBuilder,
                   Queue.GetContext()),
    Queue(Queue) { }

EnqueueMarkerBuilder
&EnqueueMarkerBuilder::SetWaitList(unsigned N, const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }

  return llvm::cast<EnqueueMarkerBuilder>(Super);
}

EnqueueMarker *EnqueueMarkerBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);
    
  if(ErrCode)
    *ErrCode = CL_SUCCESS;
    
  return new EnqueueMarker(WaitList);
}

//
// EnqueueReadBufferRectBuilder implementation.
//

EnqueueReadBufferRectBuilder::EnqueueReadBufferRectBuilder(
  CommandQueue &Queue,
  cl_mem Buf,
  void *Ptr) : CommandBuilder(CommandBuilder::EnqueueReadBufferRectBuilder,
                              Queue.GetContext()),
               Target(Ptr),
               Source(NULL),
               Blocking(false),
               Region(NULL),
               TargetOrigin(NULL),
               SourceOrigin(NULL),
               TargetOffset(0),
               SourceOffset(0) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is null");

  else if(!(Source = llvm::dyn_cast<Buffer>(Buf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is not a buffer");
  
  else if(Source->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Source->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "source sub-buffer offset is not aligned with device");
  }

  else if((Source->getFlags() & MemoryObject::HostWriteOnly) ||
          (Source->getFlags() & MemoryObject::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid read buffer operation");
  
  else if(Queue.GetContext() != Source->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and buffer have different context");
    
  if(!Target)
    NotifyError(CL_INVALID_VALUE, "pointer to data sink is null");
}

EnqueueReadBufferRectBuilder &EnqueueReadBufferRectBuilder::SetBlocking(
  bool Blocking) {
  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  this->Blocking = Blocking;

  return *this;
}

EnqueueReadBufferRectBuilder &EnqueueReadBufferRectBuilder::SetRegion(
  const size_t *Region) {
  if(!Region)
    return NotifyError(CL_INVALID_VALUE, "region is null");

  if(Region[0] == 0 || Region[1] == 0 || Region [2] == 0)
    return NotifyError(CL_INVALID_VALUE,
                       "invalid region");
  
  this->Region = Region; 

  return *this;
}

EnqueueReadBufferRectBuilder &EnqueueReadBufferRectBuilder::SetTargetOffset(
  const size_t *TargetOrigin,
  size_t TargetRowPitch,
  size_t TargetSlicePitch) {
  if(!Region)
    return *this;

  if(!TargetOrigin)
    return NotifyError(CL_INVALID_VALUE, "target origin is null");

  if(TargetRowPitch == 0) TargetPitches[0] = Region[0];
  else {
    if(TargetRowPitch < Region[0])
      return NotifyError(CL_INVALID_VALUE,
                         "invalid host row pitch");
    
    TargetPitches[0] = TargetRowPitch;
  }
  
  if(TargetSlicePitch == 0) 
    TargetPitches[1] = Region[1] * TargetPitches[0];
  else {
    if((TargetSlicePitch < Region[1] * TargetPitches[0]) ||
        (TargetSlicePitch % TargetPitches[0] !=0))
      return NotifyError(CL_INVALID_VALUE,
                         "invalid host slice pitch");
    
    TargetPitches[1] = TargetSlicePitch;
  }
  
  TargetOffset = TargetOrigin[0]
               + TargetOrigin[1] * TargetPitches[0]
               + TargetOrigin[2] * TargetPitches[1];
               
  return *this;
}

EnqueueReadBufferRectBuilder &EnqueueReadBufferRectBuilder::SetSourceOffset(
  const size_t *SourceOrigin,
  size_t SourceRowPitch,
  size_t SourceSlicePitch) {
  if(!Source || !Region)
    return *this;

  if(!SourceOrigin)
    return NotifyError(CL_INVALID_VALUE, "source origin is null");

  if(SourceRowPitch == 0) SourcePitches[0] = Region[0];
  else {
    if(SourceRowPitch < Region[0])
      return NotifyError(CL_INVALID_VALUE,
                         "invalid buffer row pitch");
    
    SourcePitches[0] = SourceRowPitch;
  }
  
  if(SourceSlicePitch == 0) 
    SourcePitches[1] = Region[1] * SourcePitches[0];
  else {
    if((SourceSlicePitch < Region[1] * SourcePitches[0]) ||
        (SourceSlicePitch % SourcePitches[0] !=0))
      return NotifyError(CL_INVALID_VALUE,
                         "invalid buffer slice pitch");
    
    SourcePitches[1] = SourceSlicePitch;
  }
  
  if(Source->getSize() < (SourceOrigin[0] + (Region[0] - 1))
                       + (SourceOrigin[1] + (Region[1] - 1)) * SourcePitches[0]
                       + (SourceOrigin[2] + (Region[2] - 1)) * SourcePitches[1])
    return NotifyError(CL_INVALID_VALUE, "region out of bound");
    
  SourceOffset = SourceOrigin[0] 
               + SourceOrigin[1] * SourcePitches[0]
               + SourceOrigin[2] * SourcePitches[1];
               
  return *this;
}

EnqueueReadBufferRectBuilder &EnqueueReadBufferRectBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");
  
  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueReadBufferRectBuilder>(Super);
}

EnqueueReadBufferRect *EnqueueReadBufferRectBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueReadBufferRect(Target, 
                                   *Source, 
                                   Blocking,
                                   Region,
                                   TargetOffset, 
                                   SourceOffset, 
                                   TargetPitches, 
                                   SourcePitches, 
                                   WaitList);
}

//
// EnqueueWriteBufferRectBuilder implementation.
//

EnqueueWriteBufferRectBuilder::EnqueueWriteBufferRectBuilder(
  CommandQueue &Queue,
  cl_mem Buf,
  const void *Ptr) : CommandBuilder(CommandBuilder::EnqueueWriteBufferRectBuilder,
                                    Queue.GetContext()),
                     Target(NULL),
                     Source(Ptr),
                     Blocking(false),
                     Region(NULL),
                     TargetOrigin(NULL),
                     SourceOrigin(NULL),
                     TargetOffset(0),
                     SourceOffset(0) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is null");

  else if(!(Target = llvm::dyn_cast<Buffer>(Buf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is not a buffer");
  
  else if(Target->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Target->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "target sub-buffer offset is not aligned with device");
  }
  
  else if((Target->getFlags() & MemoryObject::HostWriteOnly) ||
          (Target->getFlags() & MemoryObject::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid write buffer operation");
  
  else if(Queue.GetContext() != Target->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and buffer have different context");
    
  if(!Source)
    NotifyError(CL_INVALID_VALUE, "pointer to data source is null");
}

EnqueueWriteBufferRectBuilder &EnqueueWriteBufferRectBuilder::SetBlocking(
  bool Blocking) {
  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");

  this->Blocking = Blocking;

  return *this;
}

EnqueueWriteBufferRectBuilder &EnqueueWriteBufferRectBuilder::SetRegion(
  const size_t *Region) {
  if(!Region)
    return NotifyError(CL_INVALID_VALUE, "region is null");

  if(Region[0] == 0 || Region[1] == 0 || Region [2] == 0)
    return NotifyError(CL_INVALID_VALUE,
                       "invalid region");
  
  this->Region = Region;
  
  return *this;
}

EnqueueWriteBufferRectBuilder &EnqueueWriteBufferRectBuilder::SetTargetOffset(
  const size_t *TargetOrigin,
  size_t TargetRowPitch,
  size_t TargetSlicePitch) {
  if(!Target || !Region)
    return *this;

  if(!TargetOrigin)
    return NotifyError(CL_INVALID_VALUE, "target origin is null");

  if(TargetRowPitch == 0) TargetPitches[0] = Region[0];
  else {
    if(TargetRowPitch < Region[0])
      return NotifyError(CL_INVALID_VALUE,
                         "invalid buffer row pitch");
    
    TargetPitches[0] = TargetRowPitch;
  }
  
  if(TargetSlicePitch == 0) 
    TargetPitches[1] = Region[1] * TargetPitches[0];
  else {
    if((TargetSlicePitch < Region[1] * TargetPitches[0]) ||
        (TargetSlicePitch % TargetPitches[0] !=0))
      return NotifyError(CL_INVALID_VALUE,
                         "invalid buffer slice pitch");
    
    TargetPitches[1] = TargetSlicePitch;
  }
  
  if(Target->getSize() < (TargetOrigin[0] + (Region[0] - 1))
                       + (TargetOrigin[1] + (Region[1] - 1)) * TargetPitches[0]
                       + (TargetOrigin[2] + (Region[2] - 1)) * TargetPitches[1])
    return NotifyError(CL_INVALID_VALUE, "region out of bound");
    
  TargetOffset = TargetOrigin[0]
               + TargetOrigin[1] * TargetPitches[0]
               + TargetOrigin[2] * TargetPitches[1];
  
  return *this;
}

EnqueueWriteBufferRectBuilder &EnqueueWriteBufferRectBuilder::SetSourceOffset(
  const size_t *SourceOrigin,
  size_t SourceRowPitch,
  size_t SourceSlicePitch) {
  if(!Region)
    return *this;

  if(!SourceOrigin)
    return NotifyError(CL_INVALID_VALUE, "source origin is null");

  if(SourceRowPitch == 0) SourcePitches[0] = Region[0];
  else {
    if(SourceRowPitch < Region[0])
      return NotifyError(CL_INVALID_VALUE,
                         "invalid host row pitch");
    
    SourcePitches[0] = SourceRowPitch;
  }
  
  if(SourceSlicePitch == 0) 
    SourcePitches[1] = Region[1] * SourcePitches[0];
  else {
    if((SourceSlicePitch < Region[1] * SourcePitches[0]) ||
        (SourceSlicePitch % SourcePitches[0] !=0))
      return NotifyError(CL_INVALID_VALUE,
                         "invalid host slice pitch");
    
    SourcePitches[1] = SourceSlicePitch;
  }
      
  SourceOffset = SourceOrigin[0] 
               + SourceOrigin[1] * SourcePitches[0]
               + SourceOrigin[2] * SourcePitches[1];
               
  return *this;
}

EnqueueWriteBufferRectBuilder &EnqueueWriteBufferRectBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  if(Blocking && IsWaitListInconsistent())
    return NotifyError(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST,
                       "cannot block on an inconsistent wait list");
  
  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueWriteBufferRectBuilder>(Super);
}

EnqueueWriteBufferRect *EnqueueWriteBufferRectBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueWriteBufferRect(*Target, 
                                    Source, 
                                    Blocking, 
                                    Region,
                                    TargetOffset, 
                                    SourceOffset, 
                                    TargetPitches, 
                                    SourcePitches, 
                                    WaitList);
}

//
// EnqueueCopyBufferRectBuilder implementation.
//

EnqueueCopyBufferRectBuilder::EnqueueCopyBufferRectBuilder(
  CommandQueue &Queue,
  cl_mem TargetBuf,
  cl_mem SourceBuf) : CommandBuilder(CommandBuilder::EnqueueCopyBufferRectBuilder,
                                     Queue.GetContext()),
                      Target(NULL),
                      Source(NULL),
                      Region(NULL),
                      TargetOrigin(NULL),
                      SourceOrigin(NULL),
                      TargetOffset(0),
                      SourceOffset(0) {
  if(!TargetBuf)
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is null");

  else if(!(Target = llvm::dyn_cast<Buffer>(TargetBuf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is not a buffer");
   
  else if(Target->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Target->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "target sub-buffer offset is not aligned with device");
  }
 
  else if(Queue.GetContext() != Target->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and target buffer have different context");
    
  if(!SourceBuf)
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is null");

  else if(!(Source = llvm::dyn_cast<Buffer>(SourceBuf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is not a buffer");
  
  else if(Source->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Source->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "source sub-buffer offset is not aligned with device");
  }

  else if(Queue.GetContext() != Source->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and source buffer have different context");
}

EnqueueCopyBufferRectBuilder &EnqueueCopyBufferRectBuilder::SetRegion(
  const size_t *Region) {
  if(Region[0] == 0 || Region[1] == 0 || Region [2] == 0)
    return NotifyError(CL_INVALID_VALUE,
                       "invalid region");
  
  this->Region = Region;
  
  return *this;
}

EnqueueCopyBufferRectBuilder &EnqueueCopyBufferRectBuilder::SetTargetOffset(
  const size_t *TargetOrigin,
  size_t TargetRowPitch,
  size_t TargetSlicePitch) {
  if(!Target || !Region)
    return *this;

  if(!TargetOrigin)
    return NotifyError(CL_INVALID_VALUE, "target origin is null");

  if(TargetRowPitch == 0) TargetPitches[0] = Region[0];
  else {
    if(TargetRowPitch < Region[0])
      return NotifyError(CL_INVALID_VALUE,
                         "invalid target buffer row pitch");
    
    TargetPitches[0] = TargetRowPitch;
  }
  
  if(TargetSlicePitch == 0) 
    TargetPitches[1] = Region[1] * TargetPitches[0];
  else {
    if((TargetSlicePitch < Region[1] * TargetPitches[0]) ||
        (TargetSlicePitch % TargetPitches[0] !=0))
      return NotifyError(CL_INVALID_VALUE,
                         "invalid target buffer slice pitch");
    
    TargetPitches[1] = TargetSlicePitch;
  }
  
  if(Target->getSize() < (TargetOrigin[0] + (Region[0] - 1))
                       + (TargetOrigin[1] + (Region[1] - 1)) * TargetPitches[0]
                       + (TargetOrigin[2] + (Region[2] - 1)) * TargetPitches[1])
    return NotifyError(CL_INVALID_VALUE, "region out of bound");
    
  TargetOffset = TargetOrigin[0]
               + TargetOrigin[1] * TargetPitches[0]
               + TargetOrigin[2] * TargetPitches[1];
               
  this->TargetOrigin = TargetOrigin;
             
  return *this;
}

EnqueueCopyBufferRectBuilder &EnqueueCopyBufferRectBuilder::SetSourceOffset(
  const size_t *SourceOrigin,
  size_t SourceRowPitch,
  size_t SourceSlicePitch) {
  if(!Source || !Region)
    return *this;

  if(!SourceOrigin)
    return NotifyError(CL_INVALID_VALUE, "source origin is null");
  if(SourceRowPitch == 0) SourcePitches[0] = Region[0];
  else {
    if(SourceRowPitch < Region[0])
      return NotifyError(CL_INVALID_VALUE,
                         "invalid host row pitch");
    
    SourcePitches[0] = SourceRowPitch;
  }
  
  if(SourceSlicePitch == 0) 
    SourcePitches[1] = Region[1] * SourcePitches[0];
  else {
    if((SourceSlicePitch < Region[1] * SourcePitches[0]) ||
        (SourceSlicePitch % SourcePitches[0] !=0))
      return NotifyError(CL_INVALID_VALUE,
                         "invalid host slice pitch");
    
    SourcePitches[1] = SourceSlicePitch;
  }

  if(Source->getSize() < (SourceOrigin[0] + (Region[0] - 1))
                       + (SourceOrigin[1] + (Region[1] - 1)) * SourcePitches[0]
                       + (SourceOrigin[2] + (Region[2] - 1)) * SourcePitches[1])
    return NotifyError(CL_INVALID_VALUE, "region out of bound");
    
  SourceOffset = SourceOrigin[0] 
               + SourceOrigin[1] * SourcePitches[0]
               + SourceOrigin[2] * SourcePitches[1];
               
  this->SourceOrigin = SourceOrigin;
  
  return *this;
}

// The algorithm for overlap checking is taken from OpenCL v.1.2 specifications
// Appendix E.
EnqueueCopyBufferRectBuilder &EnqueueCopyBufferRectBuilder::CheckCopyOverlap() {
  if(!Target || !Source || !Region || !TargetOrigin || !SourceOrigin)
    return *this;

  // Target and Source are the same buffer objects or they are different sub-buffers
  // of the same associated buffer object.
  if((Target == Source) ||
     ((Target->isSubBuffer() && Source->isSubBuffer()) &&
      (Target->getParent() == Source->getParent()))) {
    if((TargetPitches[0] != SourcePitches[0]) && 
       (TargetPitches[1] != SourcePitches[1]))
      return NotifyError(CL_INVALID_VALUE, "different pitches between target and source buffer.");
   
    // Row and slice pitches are the same.
    size_t RowPitch = TargetPitches[0];
    size_t SlicePitch = TargetPitches[1];
    
    const size_t SourceMin[] = { SourceOrigin[0], SourceOrigin[1], SourceOrigin[2] };
    const size_t SourceMax[] = { SourceOrigin[0] + Region[0],    
                                 SourceOrigin[1] + Region[1],
                                 SourceOrigin[2] + Region[2] };

    const size_t TargetMin[] = { TargetOrigin[0], TargetOrigin[1], TargetOrigin[2] };
    const size_t TargetMax[] = { TargetOrigin[0] + Region[0],
                                 TargetOrigin[1] + Region[1],
                                 TargetOrigin[2] + Region[2] };

    // Check for overlap
    bool Overlap = true;
    for(unsigned I = 0; I < 3 && !Overlap; ++I) {
      Overlap = Overlap && (SourceMin[I] < TargetMax[I])
                        && (SourceMax[I] > TargetMin[I]);
    }

    size_t TargetStart = TargetOffset;
    size_t TargetEnd = TargetStart + (Region[2] * SlicePitch +
                                      Region[1] * RowPitch + Region[0]);
    
    size_t SourceStart = SourceOffset;
    size_t SourceEnd = SourceStart + (Region[2] * SlicePitch +
                                      Region[1] * RowPitch + Region[0]);
    
    if(!Overlap) {
      size_t DeltaSourceX = (SourceOrigin[0] + Region[0] > RowPitch) ?
                             SourceOrigin[0] + Region[0] - RowPitch : 0;
      size_t DeltaTargetX = (TargetOrigin[0] + Region[0] > RowPitch) ?
                             TargetOrigin[0] + Region[0] - RowPitch : 0;
      
      if((DeltaSourceX > 0 && DeltaSourceX > TargetOrigin[0]) ||
         (DeltaTargetX > 0 && DeltaTargetX > SourceOrigin[0])) {
        
        if((SourceStart <= TargetStart && TargetStart < SourceEnd) ||
           (TargetStart <= SourceStart && SourceStart < TargetEnd))
          Overlap = true;
          
      }
      
      if(Region[2] > 1) {
        size_t SourceHeight = SlicePitch / RowPitch;
        size_t TargetHeight = SlicePitch / RowPitch;
      
        size_t DeltaSourceY = (SourceOrigin[1] + Region[1] > SourceHeight) ?
                               SourceOrigin[1] + Region[1] - SourceHeight : 0;
        size_t DeltaTargetY = (TargetOrigin[1] + Region[1] > TargetHeight) ?
                               TargetOrigin[1] + Region[1] - TargetHeight : 0;
      
        if((DeltaSourceY > 0 && DeltaSourceY > TargetOrigin[1]) ||
           (DeltaTargetY > 0 && DeltaTargetY > SourceOrigin[1])) {    
          if((SourceStart <= TargetStart && TargetStart < SourceEnd) ||
             (TargetStart <= SourceStart && SourceStart < TargetEnd))
            Overlap = true;            
        }
      }
    }
    
    if(Overlap)
      return NotifyError(CL_MEM_COPY_OVERLAP, "source and target regions overlap");
  }
  
  return *this;
}

EnqueueCopyBufferRectBuilder &EnqueueCopyBufferRectBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueCopyBufferRectBuilder>(Super);
}

EnqueueCopyBufferRect *EnqueueCopyBufferRectBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueCopyBufferRect(*Target, 
                                   *Source,
                                   Region,
                                   TargetOffset, 
                                   SourceOffset, 
                                   TargetPitches, 
                                   SourcePitches, 
                                   WaitList);
}

//
// EnqueueBarrierBuilder implementation.
//

EnqueueBarrierBuilder::EnqueueBarrierBuilder(CommandQueue &Queue)
  : CommandBuilder(CommandBuilder::EnqueueBarrierBuilder,
                   Queue.GetContext()) { }

EnqueueBarrierBuilder
&EnqueueBarrierBuilder::SetWaitList(unsigned N, const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }

  return llvm::cast<EnqueueBarrierBuilder>(Super);
}

EnqueueBarrier *EnqueueBarrierBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);
    
  if(ErrCode)
    *ErrCode = CL_SUCCESS;
    
  return new EnqueueBarrier(WaitList);
}

//
// EnqueueFillBufferBuilder implementation.
//

EnqueueFillBufferBuilder::EnqueueFillBufferBuilder(
  CommandQueue &Queue,
  cl_mem Buf,
  const void *Pattern) : CommandBuilder(CommandBuilder::EnqueueFillBufferBuilder, Queue.GetContext()),
                         Target(NULL),
                         Source(Pattern),
                         SourceSize(0),
                         TargetOffset(0),
                         TargetSize(0) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "fill target is null");

  else if(!(Target = llvm::dyn_cast<Buffer>(Buf)))
    NotifyError(CL_INVALID_MEM_OBJECT, "fill target is not a buffer");

  else if(Target->isSubBuffer()) {
    Device &Dev = Queue.GetDevice();
    if((Dev.GetMemoryBaseAddressAlignment() != 0) &&
       ((Target->getOrigin() % Dev.GetMemoryBaseAddressAlignment()) != 0))
      NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, 
                  "target sub-buffer offset is not aligned with device");
  }

  else if(Queue.GetContext() != Target->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and buffer have different context");
    
  if(!Source)
    NotifyError(CL_INVALID_VALUE, "pointer to pattern is null");  
}

EnqueueFillBufferBuilder &EnqueueFillBufferBuilder::SetPatternSize(
  size_t PatternSize) {
  if(PatternSize == 0)
    return NotifyError(CL_INVALID_VALUE, "pattern size is zero");
    
  if((PatternSize & (PatternSize - 1)) || 
     (PatternSize > 128))
    return NotifyError(CL_INVALID_VALUE, "invalid pattern size");
  
  SourceSize = PatternSize;
  
  return *this;
}

EnqueueFillBufferBuilder &EnqueueFillBufferBuilder::SetFillRegion(
  size_t Offset,
  size_t Size) {
  if(!Target || !SourceSize)
    return *this;

  if((Offset % SourceSize) != 0)
    return NotifyError(CL_INVALID_VALUE, "fill offset is not a multiple of pattern size");
    
  if((Size % SourceSize) != 0)
    return NotifyError(CL_INVALID_VALUE, "fill size is not a multiple of pattern size");
  
  if(Offset + Size > Target->getSize())
    return NotifyError(CL_INVALID_VALUE, "fill region size exceeds destination buffer capacity");
  
  TargetOffset = Offset;
  TargetSize = Size;
  
  return *this;
}

EnqueueFillBufferBuilder &EnqueueFillBufferBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueFillBufferBuilder>(Super);
}

EnqueueFillBuffer *EnqueueFillBufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueFillBuffer(*Target, Source, SourceSize, TargetOffset, TargetSize, WaitList);
}

//
// EnqueueFillImageBuilder implementation.
//

EnqueueFillImageBuilder::EnqueueFillImageBuilder(
  CommandQueue &Queue,
  cl_mem Img,
  const void *FillColor) : CommandBuilder(CommandBuilder::EnqueueFillImageBuilder,
                                          Queue.GetContext()),
                            Target(NULL),
                            Source(FillColor),
                            TargetOrigin(NULL),
                            TargetRegion(NULL) {
  if(!Img)
    NotifyError(CL_INVALID_MEM_OBJECT, "fill target is null");

  else if(!(Target = llvm::dyn_cast<Image>(Img)))
    NotifyError(CL_INVALID_MEM_OBJECT, "fill target is not an image");
  
  else if(Queue.GetContext() != Target->getContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and image have different context");
    
  if(!Source)
    NotifyError(CL_INVALID_VALUE, "pointer to fill color is null");  

  if(Target)
    CheckDevImgSupport<>(*this, Queue, Target);
}

EnqueueFillImageBuilder &EnqueueFillImageBuilder::SetFillRegion(
    const size_t *TargetOrigin,
    const size_t *TargetRegion) {
  if(!Target)
    return *this;

  if(!TargetOrigin)
    return NotifyError(CL_INVALID_VALUE, "origin is null");

  this->TargetOrigin = TargetOrigin;

  if(!TargetRegion)
    return NotifyError(CL_INVALID_VALUE, "region is null");

  if(TargetRegion[0] == 0 || TargetRegion[1] == 0 || TargetRegion [2] == 0)
    return NotifyError(CL_INVALID_VALUE, "invalid region");

  if(!IsValidImgRegion<>(*this, Target, TargetOrigin, TargetRegion))
    return *this;

  // Region is valid.
  this->TargetRegion = TargetRegion;
  
  return *this;
}

EnqueueFillImageBuilder &EnqueueFillImageBuilder::SetWaitList(
  unsigned N,
  const cl_event *Evs) {
  CommandBuilder &Super = CommandBuilder::SetWaitList(N, Evs);

  for(Command::const_event_iterator I = WaitList.begin(), 
                                    E = WaitList.end(); 
                                    I != E; 
                                    ++I) {
    if(Ctx != (*I)->GetContext())
      return NotifyError(CL_INVALID_CONTEXT,
                         "command queue and event in wait list with different context");
  }
  
  return llvm::cast<EnqueueFillImageBuilder>(Super);
}

EnqueueFillImage *EnqueueFillImageBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new EnqueueFillImage(*Target, Source, TargetOrigin, TargetRegion, WaitList);
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

  const KernelDescriptor &Desc = this->Kern->getDescriptor();

  if(!Desc.isBuiltForDevice(&Dev))
    NotifyError(CL_INVALID_PROGRAM_EXECUTABLE,
                "kernel not built for current device");

  if(Desc.getContext() != Ctx)
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
  const KernelDescriptor &Desc = Kern->getDescriptor();
  if(!LocalWorkSizes) {
    if(Desc.hasRequiredWorkGroupSizes(&Dev))
      NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                  "kernel requires fixed local work size");

    if (!Dev.ComputeGlobalWorkPartition(GlobalWorkSizes, this->LocalWorkSizes))
      NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                  "unable to partition global work on device");
    return *this;
  }

  const auto &MaxWorkItemSizes = Dev.GetMaxWorkItemSizes();
  size_t WorkGroupSize = 1;

  llvm::SmallVector<size_t, 4> ReqWorkGroupSizes;
  Desc.getRequiredWorkGroupSizes(ReqWorkGroupSizes, &Dev);

  for(unsigned I = 0; I < WorkDimensions; ++I) {
    if(LocalWorkSizes[I] > MaxWorkItemSizes[I])
      return NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                         "work group size exceeds device limits");

    if(GlobalWorkSizes[I] % LocalWorkSizes[I])
      return NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                         "work group size does not divide "
                         "number of work items");

    if(Desc.hasRequiredWorkGroupSizes(&Dev) &&
       (I >= 3 || ReqWorkGroupSizes[I] != LocalWorkSizes[I]))
      return NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                         "work group size does not match "
                         "the one requested by the kernel");

    this->LocalWorkSizes.push_back(LocalWorkSizes[I]);
    WorkGroupSize *= LocalWorkSizes[I];
  }

  if(WorkGroupSize > Dev.GetMaxWorkGroupSize())
    return NotifyError(CL_INVALID_WORK_GROUP_SIZE,
                       "work group size exceeds device limits");

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

EnqueueNativeKernelBuilder &EnqueueNativeKernelBuilder::SetMemoryLocations(
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

    ptrdiff_t Offset = reinterpret_cast<const uint8_t*>(MemLocs[I]) -
                       reinterpret_cast<const uint8_t*>(RawArgs.first);
    Locs[Offset] = llvm::cast<MemoryObject>(MemObjs[I]);
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

  return new EnqueueNativeKernel(Func, RawArgs, Locs, WaitList);
}

//
// Utility functions.
//

// Function used by image command builders to check for
// images size and format support by device attached to
// the given command queue.
template<class ImgCmdBuilderType>
ImgCmdBuilderType &opencrun::CheckDevImgSupport(
    ImgCmdBuilderType &Bld,
    CommandQueue &Queue,
    Image *Img) {
  if(!Img)
    return Bld; 

  // Check if the image dimensions are supported by the device attached
  // to the command queue.
  switch(Img->getType()) {
  case Image::Image1D_Array:
    if(Img->getArraySize() > Queue.GetDevice().GetImageMaxArraySize())
      Bld.NotifyError(CL_INVALID_IMAGE_SIZE, 
          "image size unsupported by target device associated with queue");  
    break;
  case Image::Image1D:
    if(Img->getWidth() > Queue.GetDevice().GetImage2DMaxWidth())
      Bld.NotifyError(CL_INVALID_IMAGE_SIZE, 
          "image size unsupported by target device associated with queue");  
    break;
  case Image::Image1D_Buffer:
    if(Img->getWidth() > Queue.GetDevice().GetImageMaxBufferSize())
      Bld.NotifyError(CL_INVALID_IMAGE_SIZE,
          "image size unsupported by target device associated with queue");  
    break;
  case Image::Image2D_Array:
    if(Img->getArraySize() > Queue.GetDevice().GetImageMaxArraySize())
      Bld.NotifyError(CL_INVALID_IMAGE_SIZE, 
          "image size unsupported by target device associated with queue");  
    break;
  case Image::Image2D:
    if((Img->getWidth() > Queue.GetDevice().GetImage2DMaxWidth()) ||
        (Img->getHeight() > Queue.GetDevice().GetImage2DMaxHeight()))
      Bld.NotifyError(CL_INVALID_IMAGE_SIZE,
          "image size unsupported by target device associated with queue");  
    break;
  case Image::Image3D:
    if((Img->getWidth() > Queue.GetDevice().GetImage3DMaxWidth()) ||
        (Img->getHeight() > Queue.GetDevice().GetImage3DMaxHeight()) ||
        (Img->getDepth() > Queue.GetDevice().GetImage3DMaxDepth()))
      Bld.NotifyError(CL_INVALID_IMAGE_SIZE,
          "image size unsupported by device associated with queue");  
    break;
  default:
    Bld.NotifyError(CL_INVALID_VALUE,
        "image type unsupported by device associated with queue");
  }

  cl_image_format ImgFmt = Img->getImageFormat();
  if(!Queue.GetDevice().isImageFormatSupported(&ImgFmt))
    Bld.NotifyError(CL_IMAGE_FORMAT_NOT_SUPPORTED, 
        "image format unsupported by device associated with queue");

  return Bld;
}

// Function used by image command builders to check that (Origin, Region) 
// specifies a valid region for the given image object.
template<class ImgCmdBuilderType>
bool opencrun::IsValidImgRegion(
    ImgCmdBuilderType &Bld,
    Image *Img,
    const size_t *Origin,
    const size_t *Region) {
  if(!Img || !Origin || !Region)
    return false; 

  switch(Img->getType()) {
  case Image::Image1D:
  case Image::Image1D_Buffer:
    if(Origin[1] != 0 || Origin[2] != 0) {
      Bld.NotifyError(CL_INVALID_VALUE, "invalid origin");
      return false;
    }

    if(Region[1] != 1 || Region[2] != 1) {
      Bld.NotifyError(CL_INVALID_VALUE, "invalid region");
      return false;
    }

    if(Origin[0] + Region[0] > Img->getWidth()) {
      Bld.NotifyError(CL_INVALID_VALUE, "region out of bounds");
      return false;
    }

    break;
  case Image::Image1D_Array:
    if(Origin[2] != 0) {
      Bld.NotifyError(CL_INVALID_VALUE, "invalid origin");
      return false;
    }

    if(Region[2] != 1) {
      Bld.NotifyError(CL_INVALID_VALUE, "invalid region");
      return false;
    }

    if((Origin[0] + Region[0] > Img->getWidth()) ||
       (Origin[1] + Region[1] > Img->getArraySize())) {
      Bld.NotifyError(CL_INVALID_VALUE, "region out of bounds");
      return false;
    }

    break;
  case Image::Image2D:
    if(Origin[2] != 0) {
      Bld.NotifyError(CL_INVALID_VALUE, "invalid origin");
      return false;
    }

    if(Region[2] != 1) {
      Bld.NotifyError(CL_INVALID_VALUE, "invalid region");
      return false;
    }

    if((Origin[0] + Region[0] > Img->getWidth()) ||
       (Origin[1] + Region[1] > Img->getHeight())) {
      Bld.NotifyError(CL_INVALID_VALUE, "region out of bounds");
      return false;
    }

    break;
  case Image::Image2D_Array:
    if((Origin[0] + Region[0] > Img->getWidth()) ||
       (Origin[1] + Region[1] > Img->getHeight()) ||
       (Origin[2] + Region[2] > Img->getArraySize())) {
      Bld.NotifyError(CL_INVALID_VALUE, "region out of bounds");
      return false;
    }

    break;
  case Image::Image3D:
    if((Origin[0] + Region[0] > Img->getWidth()) ||
       (Origin[1] + Region[1] > Img->getHeight()) ||
       (Origin[2] + Region[2] > Img->getDepth())) {
      Bld.NotifyError(CL_INVALID_VALUE, "region out of bounds");
      return false;
    }

    break;
  default:
    Bld.NotifyError(CL_INVALID_VALUE, "invalid image type");
    return false;
  }

  return true;
}
