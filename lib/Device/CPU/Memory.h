
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

class Memory {
public:
  typedef std::map<MemoryObject *, void *> MappingsContainer;
};

class GlobalMemory : public Memory {
public:
  GlobalMemory(size_t Size);
  ~GlobalMemory();

public:
  void *Alloc(Buffer &Buf);
  void *Alloc(Image &Img);

  void Free(MemoryObject &MemObj);

public:
  void GetMappings(MappingsContainer &Mappings) {
    llvm::sys::ScopedLock Lock(ThisLock);

    Mappings = this->Mappings;
  }

  void *operator[](MemoryObject &MemObj) {
    llvm::sys::ScopedLock Lock(ThisLock);

    return Mappings.count(&MemObj) ? Mappings[&MemObj] : NULL;
  }

private:
  void *Alloc(MemoryObject &MemObj);
  void *AllocBufferStorage(Buffer &Buf);
  void *AllocImageStorage(Image &Img);

private:
  llvm::sys::Mutex ThisLock;
  MappingsContainer Mappings;

  size_t Size;
  size_t Available;
};

class LocalMemory : public Memory {
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
