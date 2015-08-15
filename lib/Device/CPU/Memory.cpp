
#include "Memory.h"

using namespace opencrun;
using namespace opencrun::cpu;

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
