
#include "Memory.h"

using namespace opencrun;
using namespace opencrun::cpu;

//
// GlobalMemory implementation.
//

GlobalMemory::GlobalMemory(size_t Size) : Size(Size), Available(Size) { }

GlobalMemory::~GlobalMemory() {
  for(auto I = Mappings.begin(), E = Mappings.end(); I != E; ++I)
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

// HostAccessibleBuffers and DeviceBuffers need memory to be allocated.
// See (2b) and (3b) below.
void *GlobalMemory::AllocBufferStorage(Buffer &Buf) {
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

    // CL_MEM_COPY_HOST_PTR
    if(Buf.HasHostPtr())
      std::memcpy(Addr, Buf.GetHostPtr(), Buf.GetSize());
  }

  return Addr;
}

// HostAccessibleImages and DeviceImages need memory to be allocated
// with the only exception of Image1D_Buffer objects. See (2i) and (3i).
void *GlobalMemory::AllocImageStorage(Image &Img) {
  void *Addr;

  if(Img.GetImageType() == Image::Image1D_Buffer) {
    llvm::sys::ScopedLock Lock(ThisLock);
    Buffer *Buf = Img.GetBuffer();
    if(!Buf)
      return NULL;
    
    auto It = Mappings.find(Buf);
    if(It == Mappings.end())
      return NULL;

    Addr = It->second;
    Mappings[llvm::cast<MemoryObj>(&Img)] = Addr;
    Buf->Retain();
    return Addr;
  }

  Addr = Alloc(llvm::cast<MemoryObj>(Img));

  // CL_MEM_COPY_HOST_PTR
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

// For the CPU target, the device memory and host memory share the
// same physical AS, so the model is simplified, if compared to a
// generic device with separated ASs:

// 1b) CL_MEM_USE_HOST_PTR - In this case no new memory allocation
// is requiered and the provided host_ptr is returned; the buffer
// object will be stored at the location pointed by host_ptr, which
// is assumed to be already allocated in host code.
void *GlobalMemory::Alloc(HostBuffer &Buf) {
  llvm::sys::ScopedLock Lock(ThisLock);
  
  // If Buf is a sub-buffer object the GetHostPtr method returns
  // host_ptr + origin, where host_ptr is the argument value specified
  // when the parent memory object was created.
  void *Addr = Buf.GetHostPtr();
  
  if(Addr) Mappings[llvm::cast<MemoryObj>(&Buf)] = Addr;
  
  return Addr;
}

// 2b) CL_MEM_ALLOC_HOST_PTR - In this case new memory is allocated as
// the storage area for the buffer object, and it will be accessible
// to the host code too, since the ASs are the same.
void *GlobalMemory::Alloc(HostAccessibleBuffer &Buf) {
  return AllocBufferStorage(Buf);
}

// 3b) As in the previous case, since all device memory is physically 
// undistinct from host memory.
void *GlobalMemory::Alloc(DeviceBuffer &Buf) {
  return AllocBufferStorage(Buf);
}

// 1i) See (1b).
void *GlobalMemory::Alloc(HostImage &Img) {
  llvm::sys::ScopedLock Lock(ThisLock);
  void *Addr;

  if(Img.GetImageType() == Image::Image1D_Buffer) {
    Buffer *Buf = Img.GetBuffer();
    if(!Buf)
      return NULL;
    
    auto It = Mappings.find(Buf);
    if(It == Mappings.end())
      return NULL;
    
    Buf->Retain();
    Addr = It->second;
    Mappings[llvm::cast<MemoryObj>(&Img)] = Addr;
    return Addr;
  }

  Addr = Img.GetHostPtr();
  
  if(Addr) Mappings[llvm::cast<MemoryObj>(&Img)] = Addr;

  return Addr;
}

// 2i) See (2b).
void *GlobalMemory::Alloc(HostAccessibleImage &Img) {
  return AllocImageStorage(Img);
}

// 3i) See (3b).
void *GlobalMemory::Alloc(DeviceImage &Img) {
  return AllocImageStorage(Img);
}

void GlobalMemory::Free(MemoryObj &MemObj) {
  llvm::sys::ScopedLock Lock(ThisLock);

  auto It = Mappings.find(&MemObj);
  if(!Mappings.count(&MemObj))
    return;

  // In case of a sub-buffer we simply remove the corresponding element from
  // the container and return. The effective deallocation of memory is done when
  // its parent buffer will be released.
  if(Buffer *Buf = llvm::dyn_cast<Buffer>(&MemObj))
    if(Buf->IsSubBuffer()) {
      Mappings.erase(It);
      return;
    }

  // For other memory objects (except HostBuffer and HostImages types) we need 
  // to account for available memory increase.
  if(MemObj.GetType() != MemoryObj::HostBuffer &&
     MemObj.GetType() != MemoryObj::HostImage)
    Available += MemObj.GetSize();

  // For an HostBuffer and and HostImages the memory allocation/deallocation is
  // under the control and responsability of the host code.
  if(!llvm::isa<HostBuffer>(MemObj) && !llvm::isa<HostImage>(MemObj)) {
    // The storage aread of an Image1D_Buffer belongs to the
    // associated buffer so it will be freed by the buffer itself.
    if(Image *Img = llvm::dyn_cast<Image>(&MemObj))
      if(Img->GetImageType() == Image::Image1D_Buffer) {
        Mappings.erase(It);
        Img->GetBuffer()->Release();
      }

    sys::Free(It->second);
  }
    
  Mappings.erase(It);
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

void *LocalMemory::Alloc(size_t ObjSize) {
  void *Addr = Next;

  uintptr_t NextAddr = reinterpret_cast<uintptr_t>(Next) + ObjSize;
  Next = reinterpret_cast<void *>(NextAddr);

  assert(NextAddr - reinterpret_cast<uintptr_t>(Base) <= Size &&
         "Not enough space");

  return Addr;
}
