
#ifndef OPENCRUN_PASSES_AGGRESSIVEINLINER_H
#define OPENCRUN_PASSES_AGGRESSIVEINLINER_H

#include "opencrun/Util/PassOptions.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Transforms/IPO/Inliner.h"

#include <climits>

namespace opencrun {

class AggressiveInliner : public llvm::LegacyInlinerBase {
public:
  static char ID;

public:
  AggressiveInliner(llvm::StringRef Kernel = "") :
    llvm::LegacyInlinerBase(ID, false),
    Kernel(GetKernelOption(Kernel)),
    AllInlined(true) { }

public:
  virtual bool runOnSCC(llvm::CallGraphSCC &SCC) override;

  virtual llvm::StringRef getPassName() const override {
    return "Aggressive inliner";
  }

  virtual llvm::InlineCost getInlineCost(llvm::CallSite CS) override {
    if(NotVisit.count(CS.getCaller()) ||
       NotVisit.count(CS.getCalledFunction()))
      return llvm::InlineCost::getNever();
    else
      return llvm::InlineCost::getAlways();
  }

  virtual float getInlineFudgeFactor(llvm::CallSite CS) {
    return 1.0f;
  }

  virtual void resetCachedCostInfo(llvm::Function *Caller) { }

  virtual void growCachedCostInfo(llvm::Function *Caller,
                                  llvm::Function *Callee) { }

  using Pass::doInitialization;
  using Pass::doFinalization;

  virtual bool doInitialization(llvm::CallGraph &CG) override;
  virtual bool doFinalization(llvm::CallGraph &CG) override;

public:
  bool IsAllInlined() const { return AllInlined; }

private:
  std::string Kernel;

  llvm::SmallPtrSet<const llvm::Function *, 16> NotVisit;
  bool AllInlined;
};

} // End namespace opencrun.

#endif // OPENCRUN_PASSES_AGGRESSIVEINLINER_H
