
#ifndef OPENCRUN_PASSES_FOOTPRINTESTIMATE_H
#define OPENCRUN_PASSES_FOOTPRINTESTIMATE_H

#include "opencrun/Util/Footprint.h"
#include "opencrun/Util/PassOptions.h"

#include "llvm/IR/Instruction.h"
#include "llvm/Pass.h"

#include <map>

namespace opencrun {

class FootprintEstimate : public Footprint, public llvm::FunctionPass {
public:
  static char ID;

public:
  FootprintEstimate(llvm::StringRef Kernel = "") :
    llvm::FunctionPass(ID),
    Kernel(GetKernelOption(Kernel)) { }

public:
  virtual bool runOnFunction(llvm::Function &Fun);

  virtual const char *getPassName() const {
    return "Footprint estimate";
  }

  virtual void print(llvm::raw_ostream &OS,
                     const llvm::Module *Mod = NULL) const;

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

  virtual void verifyAnalysis() const {
    // Now it makes no sense to verify this analysis. If it will becomes more
    // complex, then some verification code should be written.
  }

private:
  void AnalyzeMemoryUsage(llvm::Instruction &I);

  size_t GetSizeLowerBound(llvm::Type &Ty);

private:
  std::string Kernel;
};

} // End namespace opencrun.

#endif // OPENCRUN_PASSES_FOOTPRINTESTIMATE_H
