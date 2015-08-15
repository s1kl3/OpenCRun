
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

void *GlobalMemory::Alloc(MemoryObject &MemObj) {
  size_t RequestedSize = MemObj.getSize();

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

  if(Buf.isSubBuffer()) {
    llvm::sys::ScopedLock Lock(ThisLock);

    Buffer *Parent = Buf.getParent();
    Addr = reinterpret_cast<void *>(
            reinterpret_cast<uintptr_t>(Mappings[Parent]) + Buf.getOrigin()
           );

    if(Addr) Mappings[&Buf] = Addr;
  } else {
    Addr = Alloc(llvm::cast<MemoryObject>(Buf));

    // CL_MEM_COPY_HOST_PTR
    if(Buf.getFlags() & Buffer::CopyHostPtr)
      std::memcpy(Addr, Buf.getHostPtr(), Buf.getSize());
  }

  return Addr;
}

// HostAccessibleImages and DeviceImages need memory to be allocated
// with the only exception of Image1D_Buffer objects. See (2i) and (3i).
void *GlobalMemory::AllocImageStorage(Image &Img) {
  void *Addr;

  if(Img.getType() == Image::Image1D_Buffer) {
    llvm::sys::ScopedLock Lock(ThisLock);
    Buffer *Buf = Img.getBuffer();
    if(!Buf)
      return NULL;
    
    auto It = Mappings.find(Buf);
    if(It == Mappings.end())
      return NULL;

    Addr = It->second;
    Mappings[&Img] = Addr;
    Buf->Retain();
    return Addr;
  }

  Addr = Alloc(llvm::cast<MemoryObject>(Img));

  // CL_MEM_COPY_HOST_PTR
  if(Img.getFlags() & Image::CopyHostPtr) {
    size_t FreeBytes = Img.getSize();
    size_t RowSize = Img.getWidth() * Img.getElementSize();
    for(size_t I = 0; I < Img.getArraySize(); ++I)
      for(size_t Z = 0; Z < Img.getDepth(); ++Z)
        for(size_t Y = 0; Y < Img.getHeight() && FreeBytes >= RowSize; ++Y, FreeBytes -= RowSize)
          std::memcpy(
              reinterpret_cast<void *>(
                reinterpret_cast<uintptr_t>(Addr) + 
                Img.getWidth() * Img.getHeight() * Img.getElementSize() * I +
                Img.getWidth() * Img.getElementSize() * Y + 
                Img.getHeight() * Img.getElementSize() * Z
                ),
              reinterpret_cast<const void *>(
                reinterpret_cast<uintptr_t>(Img.getHostPtr()) + 
                Img.getSlicePitch() * I +
                Img.getRowPitch() * Y + 
                Img.getSlicePitch() * Z
                ),
              RowSize
              );
  }
    
  return Addr;
}

void *GlobalMemory::Alloc(Buffer &Buf) {
  
  // If Buf is a sub-buffer object the GetHostPtr method returns
  // host_ptr + origin, where host_ptr is the argument value specified
  // when the parent memory object was created.
  if (Buf.getFlags() & Buffer::UseHostPtr) {
    void *Addr = Buf.getHostPtr();
    llvm::sys::ScopedLock Lock(ThisLock);
    
    if (Addr)
      Mappings[&Buf] = Addr;
    
    return Addr;
  }

  return AllocBufferStorage(Buf);
}

void *GlobalMemory::Alloc(Image &Img) {
  if(Img.getType() == Image::Image1D_Buffer) {
    Buffer *Buf = Img.getBuffer();
    if(!Buf)
      return NULL;
    
    auto It = Mappings.find(Buf);
    if(It == Mappings.end())
      return NULL;
    
    Buf->Retain();
    llvm::sys::ScopedLock Lock(ThisLock);
    void *Addr = It->second;
    Mappings[&Img] = Addr;
    return Addr;
  }

  if (Img.getFlags() & Buffer::UseHostPtr) {
    void *Addr = Img.getHostPtr();
    if (Addr)
      Mappings[&Img] = Addr;

    return Addr;
  }

  return AllocImageStorage(Img);
}

void GlobalMemory::Free(MemoryObject &MemObj) {
  llvm::sys::ScopedLock Lock(ThisLock);

  auto It = Mappings.find(&MemObj);
  if(!Mappings.count(&MemObj))
    return;

  // In case of a sub-buffer we simply remove the corresponding element from
  // the container and return. The effective deallocation of memory is done when
  // its parent buffer will be released.
  if(Buffer *Buf = llvm::dyn_cast<Buffer>(&MemObj))
    if(Buf->isSubBuffer()) {
      Mappings.erase(It);
      return;
    }

  if(!(MemObj.getFlags() & MemoryObject::UseHostPtr)) {
    // The storage aread of an Image1D_Buffer belongs to the
    // associated buffer so it will be freed by the buffer itself.
    if(Image *Img = llvm::dyn_cast<Image>(&MemObj))
      if(Img->getType() == Image::Image1D_Buffer) {
        Mappings.erase(It);
        Img->getBuffer()->Release();
      }

    sys::Free(It->second);
    Available += MemObj.getSize();
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
