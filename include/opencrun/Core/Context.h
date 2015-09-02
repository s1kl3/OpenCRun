
#ifndef OPENCRUN_CORE_CONTEXT_H
#define OPENCRUN_CORE_CONTEXT_H

#include "CL/opencl.h"

#include "opencrun/Core/MemoryObject.h"
#include "opencrun/Util/MTRefCounted.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Mutex.h"

struct _cl_context { };

namespace opencrun {

class Platform;
class Device;
class CommandQueue;

class ContextErrCallbackClojure {
public:
  typedef void (CL_CALLBACK *Signature)(const char *,
                                        const void *,
                                        size_t,
                                        void *);
  typedef void *UserDataSignature;

public:
  ContextErrCallbackClojure(Signature Callback, UserDataSignature UserData) :
    Callback(Callback),
    UserData(UserData) { }

public:
  void Notify(llvm::StringRef Msg) const {
    if(Callback)
      Callback(Msg.begin(), NULL, 0, UserData);
  }

private:
  const Signature Callback;
  const UserDataSignature UserData;
};

class Context : public _cl_context, public MTRefCountedBase<Context> {
public:
  static bool classof(const _cl_context *Ctx) { return true; }

public:
  typedef llvm::SmallVector<Device *, 2> DevicesContainer;

  typedef DevicesContainer::iterator device_iterator;

public:
  device_iterator device_begin() { return Devices.begin(); }
  device_iterator device_end() { return Devices.end(); }
  llvm::iterator_range<device_iterator> devices() {
    return { device_begin(), device_end()  };
  }

public:
  DevicesContainer::size_type device_size() const { return Devices.size(); }

public:
  Context(Platform &Plat,
          DevicesContainer &Devs,
          ContextErrCallbackClojure &ErrNotify);

  ~Context();

public:
  CommandQueue *GetQueueForDevice(Device &Dev,
                                  bool OutOfOrder,
                                  bool EnableProfile,
                                  cl_int *ErrCode = NULL);

  void ReportDiagnostic(llvm::StringRef Msg);
  void ReportDiagnostic(clang::TextDiagnosticBuffer &DiagInfo);

public:
  bool IsAssociatedWith(const Device &Dev) const {
    return std::count(Devices.begin(), Devices.end(), &Dev);
  }

public:
  bool operator==(const Context &That) const { return this == &That; }
  bool operator!=(const Context &That) const { return this != &That; }

private:
  DevicesContainer Devices;

  ContextErrCallbackClojure UserDiag;

  llvm::sys::Mutex DiagLock;
  clang::DiagnosticOptions DiagOptions;
  std::unique_ptr<clang::DiagnosticsEngine> Diag;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_CONTEXT_H
