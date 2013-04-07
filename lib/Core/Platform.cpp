
#include "opencrun/Core/Platform.h"

#include "llvm/Support/ManagedStatic.h"

using namespace opencrun;

Platform::Platform() {
  GetCPUDevices(CPUs);
}

static llvm::ManagedStatic<Platform> OpenCRunPlat;

Platform &opencrun::GetOpenCRunPlatform() {
  return *OpenCRunPlat;
}

static void __init_runtime(void) __attribute__((constructor));
static void __finalize_runtime(void) __attribute__((destructor));

void __init_runtime(void) {
  llvm::llvm_start_multithreaded();
}

void __finalize_runtime(void) {
  llvm::llvm_shutdown();
}

