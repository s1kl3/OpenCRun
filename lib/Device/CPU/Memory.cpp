
#include "opencrun/Device/CPU/Memory.h"

using namespace opencrun;
using namespace opencrun::cpu;

//
// GlobalMemory implementation.
//

GlobalMemory::GlobalMemory(size_t Size) : Size(Size), Available(Size) { }

GlobalMemory::~GlobalMemory() {
  for(MappingsContainer::iterator I = Mappings.begin(),
                                  E = Mappings.end();
                                  I != E;
                                  ++I)
    sys::Free(I->second);
}

void *GlobalMemory::Alloc(MemoryObj &MemObj) {
  size_t RequestedSize = MemObj.GetSize();

  llvm::sys::ScopedLock Lock(ThisLock);

  if(Available < RequestedSize)
    return NULL;

  void *Addr = sys::CacheAlignedAlloc(RequestedSize);

  if(Addr) {
    Mappings[&MemObj] = Addr;
    Available -= RequestedSize;
  }

  return Addr;
}

// For the CPU target, the device memory and host memory share the
// same physical AS, so the model is simplified, if compared to a
// generic device with separated ASs:

// 1b) In this case no new memory allocation is requiered and the
// provided host_ptr is returned; the buffer object will be stored
// at the location pointed by host_ptr, which is supposed to be
// already allocated in host code.
void *GlobalMemory::Alloc(HostBuffer &Buf) {
  llvm::sys::ScopedLock Lock(ThisLock);
  
  // If Buf is a sub-buffer object the GetHostPtr method returns
  // host_ptr + origin, where host_ptr is the argument value specified
  // when the parent memory object is created.
  void *Addr = Buf.GetHostPtr();
  
  if(Addr) Mappings[llvm::cast<MemoryObj>(&Buf)] = Addr;
  
  return Addr;
}

// 2b) In this case new memory is allocated as the storage area for
// the buffer object, and it will be accessible to the host code 
// too, since the ASs are the same.
void *GlobalMemory::Alloc(HostAccessibleBuffer &Buf) {
  void *Addr;

  if(Buf.IsSubBuffer()) {
    llvm::sys::ScopedLock Lock(ThisLock);

    Buffer *Parent = Buf.GetParent();
    Addr = reinterpret_cast<void *>(
            reinterpret_cast<uintptr_t>(Mappings[llvm::cast<MemoryObj>(Parent)]) + Buf.GetOffset()
           );

    if(Addr) Mappings[llvm::cast<MemoryObj>(&Buf)] = Addr;
  } else {
    Addr = Alloc(llvm::cast<MemoryObj>(Buf));

    // CL_MEM_ALLOC_HOST_PTR and CL_MEM_COPY_HOST_PTR can be used
    // togheter.
    if(Buf.HasHostPtr())
      std::memcpy(Addr, Buf.GetHostPtr(), Buf.GetSize());
  }

  return Addr;
}

// 3b) As in the previous case, since all device memory is physically 
// undistinct from host memory.
void *GlobalMemory::Alloc(DeviceBuffer &Buf) {
  void *Addr;

  if(Buf.IsSubBuffer()) {
    llvm::sys::ScopedLock Lock(ThisLock);

    Buffer *Parent = Buf.GetParent();
    Addr = reinterpret_cast<void *>(
            reinterpret_cast<uintptr_t>(Mappings[llvm::cast<MemoryObj>(Parent)]) + Buf.GetOffset()
           );

    if(Addr) Mappings[llvm::cast<MemoryObj>(&Buf)] = Addr;
  } else {
    Addr = Alloc(llvm::cast<MemoryObj>(Buf));

    if(Buf.HasHostPtr())
      std::memcpy(Addr, Buf.GetHostPtr(), Buf.GetSize());
  }
  return Addr;
}

// 1i) See (1b).
void *GlobalMemory::Alloc(HostImage &Img) {
  llvm::sys::ScopedLock Lock(ThisLock);
  
  void *Addr = Img.GetHostPtr();
  
  if(Addr) Mappings[llvm::cast<MemoryObj>(&Img)] = Addr;

  if(Img.GetImageType() == Image::Image1D_Buffer) {
    Buffer *Buf = Img.GetBuffer();
    if(!Buf)
      return NULL;
    
    if(!Mappings.count(Buf))
      return NULL;
    
    std::memcpy(Addr, Mappings[Buf], Buf->GetSize());

    Buf->AddAttachedImage(&Img);
  } 

  return Addr;
}

// 2i) See (2i).
void *GlobalMemory::Alloc(HostAccessibleImage &Img) {
  void *Addr = Alloc(llvm::cast<MemoryObj>(Img));

  if(Img.GetImageType() == Image::Image1D_Buffer) {
    Buffer *Buf = Img.GetBuffer();
    if(!Buf)
      return NULL;
    
    if(!Mappings.count(Buf))
      return NULL;
    
    std::memcpy(Addr, Mappings[Buf], Buf->GetSize());

    Buf->AddAttachedImage(&Img);
  }
  
  // CL_MEM_ALLOC_HOST_PTR and CL_MEM_COPY_HOST_PTR can be used
  // togheter.
  if(Img.HasHostPtr()) {
    size_t FreeBytes = Img.GetSize();
    size_t RowSize = Img.GetWidth() * Img.GetElementSize();
    for(size_t I = 0; I < Img.GetArraySize(); ++I)
      for(size_t Z = 0; Z < Img.GetDepth(); ++Z)
        for(size_t Y = 0; Y < Img.GetHeight() && FreeBytes >= RowSize; ++Y, FreeBytes -= RowSize)
          std::memcpy(
              reinterpret_cast<void *>(
                reinterpret_cast<uintptr_t>(Addr) + 
                Img.GetWidth() * Img.GetHeight() * Img.GetElementSize() * I +
                Img.GetWidth() * Img.GetElementSize() * Y + 
                Img.GetHeight() * Img.GetElementSize() * Z
                ),
              reinterpret_cast<const void *>(
                reinterpret_cast<uintptr_t>(Img.GetHostPtr()) + 
                Img.GetSlicePitch() * I +
                Img.GetRowPitch() * Y + 
                Img.GetSlicePitch() * Z
                ),
              RowSize
              );
  }
    
  return Addr;
}

// 3i) See (3b).
void *GlobalMemory::Alloc(DeviceImage &Img) {
  void *Addr = Alloc(llvm::cast<MemoryObj>(Img));
  
  if(Img.GetImageType() == Image::Image1D_Buffer) {
    Buffer *Buf = Img.GetBuffer();
    if(!Buf)
      return NULL;
    
    if(!Mappings.count(Buf))
      return NULL;
    
    std::memcpy(Addr, Mappings[Buf], Buf->GetSize());

    Buf->AddAttachedImage(&Img);
  }

  if(Img.HasHostPtr()) {
    size_t FreeBytes = Img.GetSize();
    size_t RowSize = Img.GetWidth() * Img.GetElementSize();
    for(size_t I = 0; I < Img.GetArraySize(); ++I)
      for(size_t Z = 0; Z < Img.GetDepth(); ++Z)
        for(size_t Y = 0; Y < Img.GetHeight() && FreeBytes >= RowSize; ++Y, FreeBytes -= RowSize)
            std::memcpy(
                reinterpret_cast<void *>(
                  reinterpret_cast<uintptr_t>(Addr) + 
                  Img.GetWidth() * Img.GetHeight() * Img.GetElementSize() * I +
                  Img.GetWidth() * Img.GetElementSize() * Y + 
                  Img.GetHeight() * Img.GetElementSize() * Z
                  ),
                reinterpret_cast<const void *>(
                  reinterpret_cast<uintptr_t>(Img.GetHostPtr()) + 
                  Img.GetSlicePitch() * I +
                  Img.GetRowPitch() * Y + 
                  Img.GetSlicePitch() * Z
                  ),
                RowSize
                );
  }

  return Addr;
}

void GlobalMemory::Free(MemoryObj &MemObj) {
  llvm::sys::ScopedLock Lock(ThisLock);

  MappingsContainer::iterator I = Mappings.find(&MemObj);
  if(!Mappings.count(&MemObj))
    return;

  // In case of a sub-buffer we simply remove the corresponding element from
  // the container and return.
  if(Buffer *Buf = llvm::dyn_cast<Buffer>(&MemObj))
    if(Buf->IsSubBuffer()) {
      Mappings.erase(I);
      return;
    }

  // For other memory objects (except HostBuffer and HostImages types) we need 
  // to account for available memory increase.
  if(MemObj.GetType() != MemoryObj::HostBuffer &&
     MemObj.GetType() != MemoryObj::HostImage)
    Available += MemObj.GetSize();

  // For an HostBuffer memory allocation/deallocation is under control
  // and responsability of the host code, because we've used the host buffer
  // as a device buffer.
  if(!llvm::isa<HostBuffer>(MemObj))
    sys::Free(I->second);
    
  Mappings.erase(I);
}

//
// LocalMemory implementation.
//

LocalMemory::LocalMemory(const sys::HardwareCache &Cache) :
  Size(Cache.GetSize()) {
  Base = sys::PageAlignedAlloc(Size);
  Next = Base;
  Static = 0;
}

LocalMemory::~LocalMemory() {
  sys::Free(Base);
}

void LocalMemory::Reset(size_t StaticSize) {
  assert(StaticSize <= Size && "Not enough space");

  Static = StaticSize ? Base : 0;

  uintptr_t NextAddr = reinterpret_cast<uintptr_t>(Base) + StaticSize;
  Next = reinterpret_cast<void *>(NextAddr);
}

void *LocalMemory::Alloc(MemoryObj &MemObj) {
  void *Addr = Next;

  uintptr_t NextAddr = reinterpret_cast<uintptr_t>(Next) + MemObj.GetSize();
  Next = reinterpret_cast<void *>(NextAddr);

  assert(NextAddr - reinterpret_cast<uintptr_t>(Base) <= Size &&
         "Not enough space");

  return Addr;
}
