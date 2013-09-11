
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
// hence we return the address of the provided host buffer and update the
// available global memory size. In this way the OpenCL kernel will
// directly operate on the host buffer which will be always coherent and
// we use a half of memory.
void *GlobalMemory::Alloc(HostBuffer &Buf) {
	size_t HostBufferSize = Buf.GetSize();
	
	llvm::sys::ScopedLock Lock(ThisLock);
	
	if(Available < HostBufferSize)
		return NULL;
		
	void *Addr = Buf.GetStorageData();
	
	if(Addr) {
		Mappings[llvm::cast<MemoryObj>(&Buf)] = Addr;
		Available -= HostBufferSize;
	}
	
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

void GlobalMemory::Free(MemoryObj &MemObj) {
  llvm::sys::ScopedLock Lock(ThisLock);

  MappingsContainer::iterator I = Mappings.find(&MemObj);
  if(!Mappings.count(&MemObj))
    return;
		
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
