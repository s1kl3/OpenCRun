#ifndef CPUMODULEINFO_H
#define CPUMODULEINFO_H

#include "opencrun/Util/ModuleInfo.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"

namespace opencrun {

class CPUKernelInfo : public KernelInfo {
public:
  CPUKernelInfo(const KernelInfo &KI) : KernelInfo(KI) {}

  llvm::Function *getStub() const {
    if (llvm::MDNode *MD = getCustomInfo("stub"))
      return llvm::mdconst::extract<llvm::Function>(MD->getOperand(1));
    return nullptr;
  }
};

class CPUModuleInfo : public ModuleInfo {
public:
  CPUModuleInfo(const ModuleInfo &MI) : ModuleInfo(MI) {}

  uint64_t getAutomaticLocalsSize() const {
    if (auto *MD = getCustomInfo("automatic_locals")) {
      auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(1));
      return Cst->getZExtValue();
    }
    return 0;
  }

protected:
  llvm::MDNode *getCustomInfo(llvm::StringRef Name) const;
};

}

#endif
