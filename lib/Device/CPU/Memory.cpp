
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

// For the CPU target the device global memory is in the host AS,
// hence we return the address of the provided host buffer.
// In this way the OpenCL kernel will directly operate on the host 
// buffer which will be always coherent and we use a half of memory.
void *GlobalMemory::Alloc(HostBuffer &Buf) {
  llvm::sys::ScopedLock Lock(ThisLock);
    
  void *Addr = Buf.GetStorageData();
  
  if(Addr) Mappings[llvm::cast<MemoryObj>(&Buf)] = Addr;
  
  return Addr;
}

// In case of a CPU target, the device memory is in the same AS 
// of the host memory, so we allocate the buffer object and its
// address will be accessible by the host.
void *GlobalMemory::Alloc(HostAccessibleBuffer &Buf) {
  void *Addr = Alloc(llvm::cast<MemoryObj>(Buf));

  // CL_MEM_ALLOC_HOST_PTR and CL_MEM_COPY_HOST_PTR can be used
  // togheter.
  if(Buf.HasInitializationData())
    std::memcpy(Addr, Buf.GetInitializationData(), Buf.GetSize());

  return Addr;
}

void *GlobalMemory::Alloc(DeviceBuffer &Buf) {
  void *Addr = Alloc(llvm::cast<MemoryObj>(Buf));

  if(Buf.HasInitializationData())
    std::memcpy(Addr, Buf.GetInitializationData(), Buf.GetSize());

  return Addr;
}

void *GlobalMemory::Alloc(HostImage &Img) {
  llvm::sys::ScopedLock Lock(ThisLock);
  
  void *Addr = Img.GetStorageData();
  
  if(Addr) Mappings[llvm::cast<MemoryObj>(&Img)] = Addr;

  if(Img.GetImageType() == Image::Image1D_Buffer) {
    Buffer *Buf = Img.GetBuffer();
    if(!Buf)
      return NULL;
    
    if(!Mappings.count(Buf))
      return NULL;
    
    std::memcpy(Addr, Mappings[Buf], Buf->GetSize());
  } 

  return Addr;
}

void *GlobalMemory::Alloc(HostAccessibleImage &Img) {
  void *Addr = Alloc(llvm::cast<MemoryObj>(Img));

  if(Img.GetImageType() == Image::Image1D_Buffer) {
    Buffer *Buf = Img.GetBuffer();
    if(!Buf)
      return NULL;
    
    if(!Mappings.count(Buf))
      return NULL;
    
    std::memcpy(Addr, Mappings[Buf], Buf->GetSize());
  }
  
  // CL_MEM_ALLOC_HOST_PTR and CL_MEM_COPY_HOST_PTR can be used
  // togheter.
  if(Img.HasInitializationData()) {
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
                reinterpret_cast<uintptr_t>(Img.GetInitializationData()) + 
                Img.GetSlicePitch() * I +
                Img.GetRowPitch() * Y + 
                Img.GetSlicePitch() * Z
                ),
              RowSize
              );
  }
    
  return Addr;
}

void *GlobalMemory::Alloc(DeviceImage &Img) {
  void *Addr = Alloc(llvm::cast<MemoryObj>(Img));
  
  if(Img.GetImageType() == Image::Image1D_Buffer) {
    Buffer *Buf = Img.GetBuffer();
    if(!Buf)
      return NULL;
    
    if(!Mappings.count(Buf))
      return NULL;
    
    std::memcpy(Addr, Mappings[Buf], Buf->GetSize());
  }

  if(Img.HasInitializationData()) {
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
                  reinterpret_cast<uintptr_t>(Img.GetInitializationData()) + 
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
}

LocalMemory::~LocalMemory() {
  sys::Free(Base);
}

void LocalMemory::Reset(size_t AutomaticVarSize) {
  assert(AutomaticVarSize <= Size && "Not enough space");

  uintptr_t NextAddr = reinterpret_cast<uintptr_t>(Base) + AutomaticVarSize;
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
