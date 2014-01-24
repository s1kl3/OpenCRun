
#ifndef OPENCRUN_DEVICE_CPUPASSES_ALLPASSES_H
#define OPENCRUN_DEVICE_CPUPASSES_ALLPASSES_H

#include "llvm/Pass.h"

namespace opencrun {

llvm::Pass *CreateGroupParallelStubPass(llvm::StringRef Kernel = "");
llvm::Pass *createAutomaticLocalVariablesPass();

} // End namespace opencrun.

extern "C" void LinkInOpenCRunCPUPasses();

#endif // OPENCRUN_DEVICE_CPUPASSES_ALLPASSES_H
