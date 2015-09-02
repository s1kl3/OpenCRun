
#include "opencrun/Core/Context.h"
#include "opencrun/Core/CommandQueue.h"
#include "opencrun/Core/Device.h"
#include "opencrun/System/Env.h"

#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/Support/raw_ostream.h"

using namespace opencrun;

#define RETURN_WITH_ERROR(VAR, ERRCODE, MSG) \
  {                                          \
  ReportDiagnostic(MSG);                     \
  if(VAR)                                    \
    *VAR = ERRCODE;                          \
  return NULL;                               \
  }

Context::Context(Platform &Plat,
                 llvm::SmallVector<Device *, 2> &Devs,
                 ContextErrCallbackClojure &ErrNotify) :
  Devices(Devs),
  UserDiag(ErrNotify) {
  if(sys::HasEnv("OPENCRUN_INTERNAL_DIAGNOSTIC")) {
    clang::TextDiagnosticPrinter *DiagClient;
    
    DiagOptions.ShowColors = true;

    DiagClient = new clang::TextDiagnosticPrinter(llvm::errs(), &DiagOptions);
    DiagClient->setPrefix("opencrun");

    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagIDs;

    DiagIDs = new clang::DiagnosticIDs();
    Diag.reset(new clang::DiagnosticsEngine(DiagIDs, &DiagOptions, DiagClient));
  }

  // Implicit retain on each associated sub-device.
  for(device_iterator I = device_begin(), E = device_end(); I != E; ++I)
    if((*I)->IsSubDevice())
      (*I)->Retain();
}

Context::~Context() {
  // Implicit release on each associated sub-device.
  for(device_iterator I = device_begin(), E = device_end(); I != E; ++I)
    if((*I)->IsSubDevice())
      (*I)->Release();
}

CommandQueue *Context::GetQueueForDevice(Device &Dev,
                                         bool OutOfOrder,
                                         bool EnableProfile,
                                         cl_int *ErrCode) {
  if(!std::count(Devices.begin(), Devices.end(), &Dev))
    RETURN_WITH_ERROR(ErrCode,
                      CL_INVALID_DEVICE,
                      "device not associated with this context");

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  if(OutOfOrder)
    return new OutOfOrderQueue(*this, Dev, EnableProfile);
  else
    return new InOrderQueue(*this, Dev, EnableProfile);
}

void Context::ReportDiagnostic(llvm::StringRef Msg) {
  if(Diag) {
    llvm::sys::ScopedLock Lock(DiagLock);
    auto Diags = Diag->getDiagnosticIDs();
    Diag->Report(Diags->getCustomDiagID(clang::DiagnosticIDs::Error, Msg));
  }

  UserDiag.Notify(Msg);
}

void Context::ReportDiagnostic(clang::TextDiagnosticBuffer &DiagInfo) {
  if(Diag) {
    llvm::sys::ScopedLock Lock(DiagLock);

    DiagInfo.FlushDiagnostics(*Diag);
  }
}
