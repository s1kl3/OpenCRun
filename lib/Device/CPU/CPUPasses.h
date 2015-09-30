#ifndef CPUPASSES_H
#define CPUPASSES_H

#include "llvm/ADT/StringRef.h"

namespace llvm {
class Pass;
}

namespace opencrun {
llvm::Pass *createAutomaticLocalVariablesPass();

namespace cpu {

llvm::Pass *createGroupParallelStubPass(llvm::StringRef Name = "");

}
}

extern "C" void LinkInOpenCRunCPUPasses();

#endif
