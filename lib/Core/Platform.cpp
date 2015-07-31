
#include "opencrun/Core/Platform.h"
#include "opencrun/Core/Device.h"
#include "opencrun/Device/Devices.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/ManagedStatic.h"

using namespace opencrun;

static llvm::ManagedStatic<Platform> OpenCRunPlat;

Platform::Platform() {
  initializeCPUDevice(*this);
}

Platform::~Platform() {
  llvm::DeleteContainerPointers(CPUs);
  llvm::DeleteContainerPointers(GPUs);
  llvm::DeleteContainerPointers(Accelerators);
  llvm::DeleteContainerPointers(Customs);
}

void Platform::addDevice(Device *D) {
  switch (D->GetType()) {
  case Device::CPUType:
    CPUs.insert(D);
    break;
  case Device::GPUType:
    GPUs.insert(D);
    break;
  case Device::AcceleratorType:
    Accelerators.insert(D);
    break;
  case Device::CustomType:
    Customs.insert(D);
    break;
  }
}

Platform &opencrun::GetOpenCRunPlatform() {
  return *OpenCRunPlat;
}
