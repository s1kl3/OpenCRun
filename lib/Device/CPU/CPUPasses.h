#ifndef CPUPASSES_H
#define CPUPASSES_H

#include "llvm/ADT/StringRef.h"

namespace llvm {
class Pass;
}

namespace opencrun {
namespace cpu {

llvm::Pass *createAutomaticLocalVariablesPass();
llvm::Pass *createGroupParallelStubPass(llvm::StringRef Name = "");

}
}

extern "C" void LinkInOpenCRunCPUPasses();

#endif
