
#ifndef OPENCRUN_PASSES_ASYNCCOPYSPECIALIZATION_H
#define OPENCRUN_PASSES_ASYNCCOPYSPECIALIZATION_H

#include "opencrun/Util/PassOptions.h"

#include "llvm/IR/Instruction.h"
#include "llvm/Pass.h"

namespace opencrun {

class AsyncCopySpecialization : public llvm::FunctionPass {
public:
  static char ID;

public:
  AsyncCopySpecialization(llvm::StringRef Kernel = "") :
    llvm::FunctionPass(ID),
    Kernel(GetKernelOption(Kernel)) { }

public:
  virtual bool runOnFunction(llvm::Function &Fun);
  
  virtual const char *getPassName() const {
    return "Asynchronous copy and prefetch function specialization";
  }

private:
  std::string Kernel;
};

} // End namespace opencrun.

#endif // OPENCRUN_PASSES_ASYNCCOPYSPECIALIZATION_H
