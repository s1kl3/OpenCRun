
#include "opencrun/Core/MemoryObj.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/Device.h"

using namespace opencrun;

//
// MemoryObj implementation.
//

MemoryObj::~MemoryObj() {
  Ctx->DestroyMemoryObj(*this);
}

bool MemoryObj::AddNewMapping(void *MapBuf,
                              size_t Offset,
                              size_t Size,
                              unsigned long MapFlags) {
  if(!MapBuf)
    return false;
    
  if(Size == 0)
    return false;
    
  if(MapFlags == 0)
    return false;
  
  MappingInfo Infos;

  Infos.Offset = Offset;
  Infos.Size = Size;
  Infos.MapFlags = MapFlags;
  
  llvm::sys::ScopedLock Lock(ThisLock);
  
  // This will handle also the (CL_MAP_READ | CL_MAP_WRITE) case.
  if(MapFlags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION)) {
    if(Maps.count(MapBuf))
      return false;

    // We have to check that the mapped region doesn't overlap
    // with any other mapped region of the same memory object.
    for(maps_iterator I = Maps.begin(), E = Maps.end(); I != E; ++I) {
      if((Offset <= (I->second.Offset + I->second.Size))
        && ((Offset + Size) >=  I->second.Offset))
        return false;
    }
  
    Maps.insert(std::pair<void *, MappingInfo>(MapBuf, Infos));
  }
  
  // This will handle read-only mappings.
  else if(MapFlags & CL_MAP_READ)
    // There can be multiple read mappings of overlapped regions.
    Maps.insert(std::pair<void *, MappingInfo>(MapBuf, Infos));
  
  return true;
}

bool MemoryObj::RemoveMapping(void *MapBuf) {
  llvm::sys::ScopedLock Lock(ThisLock);
  
  // We remove the first found element with the given key value. In case of
  // read mappings with the same host address, this will remove the first
  // mapping record in the container (despite of its MappingInfo values). 
  // For write mappings this will remove the only existing record in the 
  // container.
  maps_iterator it = Maps.find(MapBuf);
  if(it == Maps.end())
    return false;
    
  Maps.erase(it);
  
  return true;
}													

bool MemoryObj::IsValidMappingPtr(void *MapBuf) {
  llvm::sys::ScopedLock Lock(ThisLock);
  
  return Maps.count(MapBuf) > 0 ? true : false;
}

MemoryObj::MappingInfo *MemoryObj::GetMappingInfo(void *MapBuf) {
  maps_iterator it = Maps.find(MapBuf);
  if(it == Maps.end())
    return NULL;
    
  return &(it->second);
}

//
// BufferBuilder implementation.
//

#define RETURN_WITH_ERROR(VAR) \
  {                            \
  if(VAR)                      \
    *VAR = this->ErrCode;      \
  return NULL;                 \
  }

BufferBuilder::BufferBuilder(Context &Ctx, size_t Size) :
  Ctx(Ctx),
  Size(Size),
  HostPtr(NULL),
  HostPtrMode(MemoryObj::NoHostPtrUsage),
  AccessProt(MemoryObj::InvalidProtection),
  HostAccessProt(MemoryObj::HostNoProtection),
  ErrCode(CL_SUCCESS) {
  if(!Size) {
    NotifyError(CL_INVALID_BUFFER_SIZE, "buffer size must be greater than 0");
    return;
  }

  for(Context::device_iterator I = Ctx.device_begin(), E = Ctx.device_end();
                               I != E;
                               ++I)
    if(Size > (*I)->GetMaxMemoryAllocSize()) {
      NotifyError(CL_INVALID_BUFFER_SIZE,
                  "buffer size exceed device capabilities");
      return;
    }
}

BufferBuilder &BufferBuilder::SetUseHostMemory(bool Enabled, void* Storage) {
  if(Enabled) {
    if(!Storage)
      return NotifyError(CL_INVALID_HOST_PTR, "missing host storage pointer");

    if((HostPtrMode == MemoryObj::AllocHostPtr) || (HostPtrMode == MemoryObj::CopyHostPtr))
      return NotifyError(CL_INVALID_VALUE,
                         "multiple buffer storage specifiers not allowed");

    HostPtrMode = MemoryObj::UseHostPtr;
    HostPtr = Storage;
  }

  return *this;
}

BufferBuilder &BufferBuilder::SetAllocHostMemory(bool Enabled) {
  if(Enabled) {
    if(HostPtrMode == MemoryObj::UseHostPtr)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple buffer storage specifiers not allowed");

    HostPtrMode = MemoryObj::AllocHostPtr;
  }

  return *this;
}

BufferBuilder &BufferBuilder::SetCopyHostMemory(bool Enabled, void* Src) {
  if(Enabled) {
    if(!Src)
      return NotifyError(CL_INVALID_HOST_PTR,
                         "missed pointer to initialization data");

    if(HostPtrMode == MemoryObj::UseHostPtr)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple buffer storage specifiers not allowed");

    HostPtrMode = MemoryObj::CopyHostPtr;
    HostPtr = Src;
  }

  return *this;
}

BufferBuilder &BufferBuilder::SetReadWrite(bool Enabled) {
  if(Enabled) {
    if(AccessProt == MemoryObj::ReadOnly || AccessProt == MemoryObj::WriteOnly)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple access protection flags not allowed");

    AccessProt = MemoryObj::ReadWrite;
  }

  return *this;
}

BufferBuilder &BufferBuilder::SetWriteOnly(bool Enabled) {
  if(Enabled) {
    if(AccessProt == MemoryObj::ReadWrite || AccessProt == MemoryObj::ReadOnly)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple access protection flags not allowed");

    AccessProt = MemoryObj::WriteOnly;
  }

  return *this;
}

BufferBuilder &BufferBuilder::SetReadOnly(bool Enabled) {
  if(Enabled) {
    if(AccessProt == MemoryObj::ReadWrite || AccessProt == MemoryObj::WriteOnly)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple access protection flags not allowed");

    AccessProt = MemoryObj::ReadOnly;
  }

  return *this;
}

BufferBuilder &BufferBuilder::SetHostWriteOnly(bool Enabled) {
  if(Enabled) {
    if(HostAccessProt == MemoryObj::HostReadOnly || 
        HostAccessProt == MemoryObj::HostNoAccess)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple host access protection flags not allowed");
    
    // TODO: Implement optimization strategy.
    HostAccessProt = MemoryObj::HostWriteOnly;
  }
  
  return *this;
}

BufferBuilder &BufferBuilder::SetHostReadOnly(bool Enabled) {
  if(Enabled) {
    if(HostAccessProt == MemoryObj::HostWriteOnly || 
        HostAccessProt == MemoryObj::HostNoAccess)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple host access protection flags not allowed");
    
    // TODO: Implement optimization strategy.
    HostAccessProt = MemoryObj::HostReadOnly;
  }
  
  return *this;
}

BufferBuilder &BufferBuilder::SetHostNoAccess(bool Enabled) {
  if(Enabled) {
    if(HostAccessProt == MemoryObj::HostWriteOnly || 
        HostAccessProt == MemoryObj::HostReadOnly)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple host access protection flags not allowed");
    
    // TODO: Implement optimization strategy.
    HostAccessProt = MemoryObj::HostNoAccess;
  }
  
  return *this;
}

Buffer *BufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(HostPtrMode == MemoryObj::UseHostPtr)
    return Ctx.CreateHostBuffer(Size, HostPtr, AccessProt, HostAccessProt, ErrCode);
  else if(HostPtrMode == MemoryObj::AllocHostPtr)
    return Ctx.CreateHostAccessibleBuffer(Size, HostPtr, AccessProt, HostAccessProt, ErrCode);
  else
    return Ctx.CreateDeviceBuffer(Size, HostPtr, AccessProt, HostAccessProt, ErrCode);
}

BufferBuilder &BufferBuilder::NotifyError(cl_int ErrCode, const char *Msg) {
  Ctx.ReportDiagnostic(Msg);
  this->ErrCode = ErrCode;

  return *this;
}
