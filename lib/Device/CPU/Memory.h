
#ifndef OPENCRUN_DEVICE_CPU_MEMORY_H
#define OPENCRUN_DEVICE_CPU_MEMORY_H

#include "opencrun/Core/Context.h"
#include "opencrun/Core/MemoryObject.h"
#include "opencrun/System/Hardware.h"
#include "opencrun/System/OS.h"

#include "llvm/Support/Mutex.h"

#include <map>

using namespace opencrun;

namespace opencrun {
namespace cpu {

class LocalMemory {
public:
  LocalMemory(const sys::HardwareCache &Cache);
  ~LocalMemory();

public:
  void Reset(size_t StaticSize = 0);

  void *Alloc(size_t ObjSize);

  void *GetStaticPtr() const { return Static; }

private:
  size_t Size;

  void *Base;
  void *Next;
  void *Static;
};

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_MEMORY_H
