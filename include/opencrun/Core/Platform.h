
#ifndef OPENCRUN_CORE_PLATFORM_H
#define OPENCRUN_CORE_PLATFORM_H

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/StringRef.h"

struct _cl_platform_id { };

namespace opencrun {

class Device;

class PlatformInfo {
public:
  llvm::StringRef GetProfile() const { return "FULL_PROFILE"; }
  llvm::StringRef GetVersion() const { return "OpenCL 1.2"; }
  llvm::StringRef GetName() const { return "OpenCRun"; }
  llvm::StringRef GetVendor() const { return "UC 2.0"; }
  llvm::StringRef GetExtensions() const { return ""; }
};

class Platform : public _cl_platform_id, public PlatformInfo {
public:
  static bool classof(const _cl_platform_id *Plat) { return true; }

public:
  typedef llvm::SmallPtrSet<Device *, 2> DevicesContainer;
  typedef DevicesContainer::iterator device_iterator;

public:
  Platform();
  ~Platform();
  Platform(const Platform &) = delete;
  void operator=(const Platform &) = delete;

  device_iterator cpu_begin() { return CPUs.begin(); }
  device_iterator cpu_end() { return CPUs.end(); }

  device_iterator gpu_begin() { return GPUs.begin(); }
  device_iterator gpu_end() { return GPUs.end(); }

  device_iterator accelerator_begin() { return Accelerators.begin(); }
  device_iterator accelerator_end() { return Accelerators.end(); }

  device_iterator custom_begin() { return Customs.begin(); }
  device_iterator custom_end() { return Customs.end(); }

  void addDevice(Device *D);

private:
  DevicesContainer CPUs;
  DevicesContainer GPUs;
  DevicesContainer Accelerators;
  DevicesContainer Customs;
};

Platform &GetOpenCRunPlatform();

} // End namespace opencrun.

#endif // OPENCRUN_CORE_PLATFORM_H
