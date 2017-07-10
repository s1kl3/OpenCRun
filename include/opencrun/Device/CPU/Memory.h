
#ifndef OPENCRUN_DEVICE_CPU_MEMORY_H
#define OPENCRUN_DEVICE_CPU_MEMORY_H

#include "opencrun/Core/Context.h"
#include "opencrun/Core/MemoryObj.h"
#include "opencrun/System/Hardware.h"
#include "opencrun/System/OS.h"

#include "llvm/Support/Mutex.h"

#include <map>

using namespace opencrun;

namespace opencrun {
namespace cpu {

class Memory {
public:
  typedef std::map<MemoryObj *, void *> MappingsContainer;
};

class GlobalMemory : public Memory {
public:
  GlobalMemory(size_t Size);
  ~GlobalMemory();

public:
  void *Alloc(HostBuffer &Buf);
  void *Alloc(HostAccessibleBuffer &Buf);
  void *Alloc(DeviceBuffer &Buf);
  void *Alloc(HostImage &Img);
  void *Alloc(HostAccessibleImage &Img);
  void *Alloc(DeviceImage &Img);

  void Free(MemoryObj &MemObj);

public:
  void GetMappings(MappingsContainer &Mappings) {
    llvm::sys::ScopedLock Lock(ThisLock);

    Mappings = this->Mappings;
  }

  void *operator[](MemoryObj &MemObj) {
    llvm::sys::ScopedLock Lock(ThisLock);

    return Mappings.count(&MemObj) ? Mappings[&MemObj] : NULL;
  }

private:
  void *Alloc(MemoryObj &MemObj);
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
