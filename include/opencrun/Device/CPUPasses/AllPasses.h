
#ifndef OPENCRUN_DEVICE_CPUPASSES_ALLPASSES_H
#define OPENCRUN_DEVICE_CPUPASSES_ALLPASSES_H

#include "llvm/Pass.h"

namespace opencrun {

class Device;

llvm::Pass *CreateGroupParallelStubPass(const Device *Dev = 0,
                                        llvm::StringRef Kernel = "");
llvm::Pass *createAutomaticLocalVariablesPass();

} // End namespace opencrun.

extern "C" void LinkInOpenCRunCPUPasses();

#endif // OPENCRUN_DEVICE_CPUPASSES_ALLPASSES_H
