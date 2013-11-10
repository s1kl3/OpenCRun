
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
                                   const size_t *Region,
                                   size_t SourceOffset, 
                                   size_t *TargetPitches,
                                   EventsContainer &WaitList)
  : Command(Command::ReadImage, WaitList),
    Target(Target),
    Source(&Source),
    SourceOffset(SourceOffset) {
  // Convert the region width in bytes.
  this->Region[0] = Region[0] * Source.GetElementSize();
  this->Region[1] = Region[1];
  this->Region[2] = Region[2];
  std::memcpy(this->TargetPitches, TargetPitches, 2 * sizeof(size_t));
}

//
// EnqueueWriteImage implementation.
//

EnqueueWriteImage::EnqueueWriteImage(Image &Target, 
                                     const void *Source, 
                                     bool Blocking, 
                                     const size_t *Region, 
                                     size_t TargetOffset, 
                                     size_t *SourcePitches, 
                                     EventsContainer &WaitList)
  : Command(Command::WriteImage, WaitList),
    Target(&Target),
    Source(Source),
    TargetOffset(TargetOffset) { 
  // Convert the region width in bytes.
  this->Region[0] = Region[0] * Target.GetElementSize();
  this->Region[1] = Region[1];
  this->Region[2] = Region[2];
  std::memcpy(this->SourcePitches, SourcePitches, 2 * sizeof(size_t));
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
// EnqueueUnmapMemObject implementation.
//

EnqueueUnmapMemObject::EnqueueUnmapMemObject(MemoryObj &MemObj,
                                             void *MappedPtr,
                                             EventsContainer &WaitList)
  :	Command(Command::UnmapMemObject, WaitList),
    MemObj(&MemObj),
    MappedPtr(MappedPtr) { }    

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
                  Source(NULL),
                  Target(Target),
                  Blocking(false),
                  Offset(0),
                  Size(0) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is null");

  else if(!(Source = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(Buf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is not a buffer");
  
  else if((Source->GetHostAccessProtection() == MemoryObj::HostWriteOnly) ||
          (Source->GetHostAccessProtection() == MemoryObj::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid read buffer operation");

  else if(Ctx != Source->GetContext())
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

  if(Offset + Size > Source->GetSize())
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
  Context &Ctx,
  cl_mem Buf,
  const void *Source) : CommandBuilder(CommandBuilder::EnqueueWriteBufferBuilder,
                                    Ctx),
                        Target(NULL),
                        Source(Source),
                        Blocking(false),
                        Offset(0),
                        Size(0) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is null");

  else if(!(Target = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(Buf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is not a buffer");
  
  else if((Target->GetHostAccessProtection() == MemoryObj::HostReadOnly) ||
          (Target->GetHostAccessProtection() == MemoryObj::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid read buffer operation");

  else if(Ctx != Target->GetContext())
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

  if(Offset + Size > Target->GetSize())
    return NotifyError(CL_INVALID_VALUE, "data size exceeds buffer capacity");

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
  Context &Ctx,
  cl_mem TargetBuf,
  cl_mem SourceBuf) : CommandBuilder(CommandBuilder::EnqueueCopyBufferBuilder,
                                  Ctx),
                      Target(NULL),
                      Source(NULL),
                      TargetOffset(0),
                      SourceOffset(0),
                      Size(0) {
  if(!TargetBuf)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is null");
  
  else if(!(Target = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(TargetBuf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy target is not a buffer");
    
  else if(Ctx != Target->GetContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and target buffer have different context");
  
  if(!SourceBuf)
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source is null");

  else if(!(Source = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(SourceBuf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "copy source is not a buffer");				 
    
  else if(Ctx != Source->GetContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and source buffer have different context"); 
}

EnqueueCopyBufferBuilder &EnqueueCopyBufferBuilder::SetCopyArea(size_t TargetOffset,
                                                                size_t SourceOffset,
                                                                size_t Size) {
  if(!Target || !Source)
    return *this;

  if(TargetOffset + Size > Target->GetSize())
    return NotifyError(CL_INVALID_VALUE, "data size exceeds destination buffer capacity");
    
  if(SourceOffset + Size > Source->GetSize())
    return NotifyError(CL_INVALID_VALUE, "data size exceeds source buffer capacity");
  
  // TODO: checking for sub-buffers.

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
  Context &Ctx,
  cl_mem Img,
  void *Target) : CommandBuilder(CommandBuilder::EnqueueReadImageBuilder,
                                 Ctx),
                  Source(NULL),
                  Target(Target),
                  Blocking(false),
                  Region(NULL),
                  SourceOffset(0) {
  if(!Img)
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is null");

  else if(!(Source = llvm::dyn_cast<Image>(llvm::cast<MemoryObj>(Img))))
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is not an image");
  
  else if((Source->GetHostAccessProtection() == MemoryObj::HostWriteOnly) ||
          (Source->GetHostAccessProtection() == MemoryObj::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid read image operation");

  else if(Ctx != Source->GetContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and image have different context");
    
  if(!Target)
    NotifyError(CL_INVALID_VALUE, "pointer to data sink is null");
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
    const size_t *Origin,
    const size_t *Region,
    size_t RowPitch,
    size_t SlicePitch) {
  if(Region[0] == 0 || Region[1] == 0 || Region [2] == 0)
    return NotifyError(CL_INVALID_VALUE, "invalid region");

  switch(Source->GetImageType()) {
  case Image::Image1D:
  case Image::Image1D_Buffer:
    if(Origin[1] != 0 || Origin[2] != 0)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if(Region[1] != 1 || Region[2] != 1)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if(Origin[0] + Region[0] > Source->GetWidth())
      return NotifyError(CL_INVALID_VALUE, "region out of bounds");

    break;
  case Image::Image1D_Array:
    if(Origin[2] != 0)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if(Region[2] != 1)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if((Origin[0] + Region[0] > Source->GetWidth()) ||
       (Origin[1] + Region[1] > Source->GetArraySize()))
      return NotifyError(CL_INVALID_VALUE, "region out of bounds");

    break;
  case Image::Image2D:
    if(Origin[2] != 0)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if(Region[2] != 1)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if((Origin[0] + Region[0] > Source->GetWidth()) ||
       (Origin[1] + Region[1] > Source->GetHeight()))
      return NotifyError(CL_INVALID_VALUE, "region out of bounds");

    break;
  case Image::Image2D_Array:
    if((Origin[0] + Region[0] > Source->GetWidth()) ||
       (Origin[1] + Region[1] > Source->GetHeight()) ||
       (Origin[2] + Region[2] > Source->GetArraySize()))
      return NotifyError(CL_INVALID_VALUE, "region out of bounds");

    break;
  case Image::Image3D:
    if((Origin[0] + Region[0] > Source->GetWidth()) ||
       (Origin[1] + Region[1] > Source->GetHeight()) ||
       (Origin[2] + Region[2] > Source->GetDepth()))
      return NotifyError(CL_INVALID_VALUE, "region out of bounds");

    break;
  default:
    return NotifyError(CL_INVALID_VALUE, "invalid image type");
  }

  this->Region = Region;

  // Calculate offset inside the image object.
  SourceOffset = Origin[0] * Source->GetElementSize() +
                 Origin[1] * Source->GetRowPitch() +
                 Origin[2] * Source->GetSlicePitch();

  // Check row and slice picthes.
  if(RowPitch == 0) 
    TargetPitches[0] = Region[0] * Source->GetElementSize();
  else {
    if(RowPitch < Region[0] * Source->GetElementSize())
      return NotifyError(CL_INVALID_VALUE, "invalid row pitch");
    TargetPitches[0] = RowPitch;
  }

  if(SlicePitch == 0) 
    TargetPitches[1] = TargetPitches[0] * Region[1];
  else {
    if(SlicePitch < TargetPitches[0] * Region[1])
      return NotifyError(CL_INVALID_VALUE, "invalid slice pitch");
    TargetPitches[1] = SlicePitch;
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
                              Region,
                              SourceOffset,
                              TargetPitches,
                              WaitList);
}

//
// EnqueueWriteImageBuilder implementation.
//

EnqueueWriteImageBuilder::EnqueueWriteImageBuilder(
  Context &Ctx,
  cl_mem Img,
  const void *Source) : CommandBuilder(CommandBuilder::EnqueueWriteImageBuilder,
                                       Ctx),
                  Source(Source),
                  Target(NULL),
                  Blocking(false),
                  Region(NULL),
                  TargetOffset(0) {
  if(!Img)
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is null");

  else if(!(Target = llvm::dyn_cast<Image>(llvm::cast<MemoryObj>(Img))))
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is not an image");
  
  else if((Target->GetHostAccessProtection() == MemoryObj::HostReadOnly) ||
          (Target->GetHostAccessProtection() == MemoryObj::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid write image operation");

  else if(Ctx != Target->GetContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and image have different context");
    
  if(!Target)
    NotifyError(CL_INVALID_VALUE, "pointer to data source is null");
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
    const size_t *Origin,
    const size_t *Region,
    size_t InputRowPitch,
    size_t InputSlicePitch) {
  if(Region[0] == 0 || Region[1] == 0 || Region [2] == 0)
    return NotifyError(CL_INVALID_VALUE, "invalid region");

  switch(Target->GetImageType()) {
  case Image::Image1D:
  case Image::Image1D_Buffer:
    if(Origin[1] != 0 || Origin[2] != 0)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if(Region[1] != 1 || Region[2] != 1)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if(Origin[0] + Region[0] > Target->GetWidth())
      return NotifyError(CL_INVALID_VALUE, "region out of bounds");

    break;
  case Image::Image1D_Array:
    if(Origin[2] != 0)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if(Region[2] != 1)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if((Origin[0] + Region[0] > Target->GetWidth()) ||
       (Origin[1] + Region[1] > Target->GetArraySize()))
      return NotifyError(CL_INVALID_VALUE, "region out of bounds");

    break;
  case Image::Image2D:
    if(Origin[2] != 0)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if(Region[2] != 1)
      return NotifyError(CL_INVALID_VALUE, "invalid origin");

    if((Origin[0] + Region[0] > Target->GetWidth()) ||
       (Origin[1] + Region[1] > Target->GetHeight()))
      return NotifyError(CL_INVALID_VALUE, "region out of bounds");

    break;
  case Image::Image2D_Array:
    if((Origin[0] + Region[0] > Target->GetWidth()) ||
       (Origin[1] + Region[1] > Target->GetHeight()) ||
       (Origin[2] + Region[2] > Target->GetArraySize()))
      return NotifyError(CL_INVALID_VALUE, "region out of bounds");

    break;
  case Image::Image3D:
    if((Origin[0] + Region[0] > Target->GetWidth()) ||
       (Origin[1] + Region[1] > Target->GetHeight()) ||
       (Origin[2] + Region[2] > Target->GetDepth()))
      return NotifyError(CL_INVALID_VALUE, "region out of bounds");

    break;
  default:
    return NotifyError(CL_INVALID_VALUE, "invalid image type");
  }

  this->Region = Region;

  // Calculate offset inside the image object.
  TargetOffset = Origin[0] * Target->GetElementSize() +
                 Origin[1] * Target->GetRowPitch() +
                 Origin[2] * Target->GetSlicePitch();

  // Check row and slice picthes.
  if(InputRowPitch == 0) 
    SourcePitches[0] = Region[0] * Target->GetElementSize();
  else {
    if(InputRowPitch < Region[0] * Target->GetElementSize())
      return NotifyError(CL_INVALID_VALUE, "invalid input row pitch");
    SourcePitches[0] = InputRowPitch;
  }

  if(InputSlicePitch == 0) 
    SourcePitches[1] = SourcePitches[0] * Region[1];
  else {
    if(InputSlicePitch < SourcePitches[0] * Region[1])
      return NotifyError(CL_INVALID_VALUE, "invalid input slice pitch");
    SourcePitches[1] = InputSlicePitch;
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
                               Region,
                               TargetOffset,
                               SourcePitches,
                               WaitList);
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
    
  if(Ctx != Source->GetContext())
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
  
  else if(Ctx != llvm::cast<MemoryObj>(MemObj)->GetContext())
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
// EnqueueReadBufferRectBuilder implementation.
//

EnqueueReadBufferRectBuilder::EnqueueReadBufferRectBuilder(
  Context &Ctx,
  cl_mem Buf,
  void *Ptr) : CommandBuilder(CommandBuilder::EnqueueReadBufferRectBuilder,
                              Ctx),
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

  else if(!(Source = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(Buf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is not a buffer");
  
  else if((Source->GetHostAccessProtection() == MemoryObj::HostWriteOnly) ||
          (Source->GetHostAccessProtection() == MemoryObj::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid read buffer operation");
  
  else if(Ctx != Source->GetContext())
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
  
  if(Source->GetSize() < (SourceOrigin[0] + (Region[0] - 1))
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
  Context &Ctx,
  cl_mem Buf,
  const void *Ptr) : CommandBuilder(CommandBuilder::EnqueueWriteBufferRectBuilder,
                                    Ctx),
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

  else if(!(Target = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(Buf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is not a buffer");
  
  else if((Target->GetHostAccessProtection() == MemoryObj::HostWriteOnly) ||
          (Target->GetHostAccessProtection() == MemoryObj::HostNoAccess))
    NotifyError(CL_INVALID_OPERATION, "invalid write buffer operation");
  
  else if(Ctx != Target->GetContext())
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
  
  if(Target->GetSize() < (TargetOrigin[0] + (Region[0] - 1))
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
  Context &Ctx,
  cl_mem TargetBuf,
  cl_mem SourceBuf) : CommandBuilder(CommandBuilder::EnqueueCopyBufferRectBuilder,
                                  Ctx),
                      Target(NULL),
                      Source(NULL),
                      Region(NULL),
                      TargetOrigin(NULL),
                      SourceOrigin(NULL),
                      TargetOffset(0),
                      SourceOffset(0) {
  if(!TargetBuf)
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is null");

  else if(!(Target = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(TargetBuf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "write target is not a buffer");
  
  else if(Ctx != Target->GetContext())
    NotifyError(CL_INVALID_CONTEXT, "command queue and target buffer have different context");
    
  if(!SourceBuf)
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is null");

  else if(!(Source = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(SourceBuf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "read source is not a buffer");
  
  else if(Ctx != Source->GetContext())
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
  
  if(Target->GetSize() < (TargetOrigin[0] + (Region[0] - 1))
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

  if(Source->GetSize() < (SourceOrigin[0] + (Region[0] - 1))
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
  if(Target == Source) {
    // Since target and source buffers are the same object they must have the
    // same row and slice pitches.
    if((TargetPitches[0] != SourcePitches[0]) && 
       (TargetPitches[1] != SourcePitches[1]))
      return NotifyError(CL_INVALID_VALUE, "different pitches for the same buffer");
   
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
// EnqueueFillBufferBuilder implementation.
//

EnqueueFillBufferBuilder::EnqueueFillBufferBuilder(
  Context &Ctx,
  cl_mem Buf,
  const void *Pattern) :
  CommandBuilder(CommandBuilder::EnqueueFillBufferBuilder, Ctx),
  Target(NULL),
  Source(Pattern),
  SourceSize(0),
  TargetOffset(0),
  TargetSize(0) {
  if(!Buf)
    NotifyError(CL_INVALID_MEM_OBJECT, "fill target is null");

  else if(!(Target = llvm::dyn_cast<Buffer>(llvm::cast<MemoryObj>(Buf))))
    NotifyError(CL_INVALID_MEM_OBJECT, "fill target is not a buffer");
  
  else if(Ctx != Target->GetContext())
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
  if((Offset % SourceSize) != 0)
    return NotifyError(CL_INVALID_VALUE, "fill offset is not a multiple of pattern size");
    
  if((Size % SourceSize) != 0)
    return NotifyError(CL_INVALID_VALUE, "fill size is not a multiple of pattern size");
  
  if(Offset + Size > Target->GetSize())
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
                         "work group size exceeds device limits");

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
