
#include "opencrun/Device/Devices.h"

#include "llvm/Support/ManagedStatic.h"

using namespace opencrun;

//
// CPUContainer implementation.
//

namespace {

class CPUContainer {
public:
  typedef llvm::SmallPtrSet<CPUDevice *, 2> CPUsContainer;

public:
  CPUContainer() {
    sys::Hardware &HW = sys::GetHardware();

    // A device for each Machine in the System (it may be a cluster system).
    for(sys::Hardware::machine_iterator I = HW.machine_begin(),
                                        E = HW.machine_end();
                                        I != E;
                                        ++I) {
      CPUDevice *CPU = new CPUDevice(*I);

      // The reference count is always equal to 1 for 
      // root devices and it's never released.
      CPU->Retain();

      CPUs.insert(CPU);
    }
  }

  ~CPUContainer() { llvm::DeleteContainerPointers(CPUs); }

public:
  CPUsContainer &GetSystemCPUs() { return CPUs; }

private:
  CPUsContainer CPUs;
};

} // End anonymous namespace.

llvm::ManagedStatic<CPUContainer> CPUDevices;

void opencrun::GetCPUDevices(llvm::SmallPtrSet<CPUDevice *,2> &CPUs) {
    CPUs = CPUDevices->GetSystemCPUs();
}
