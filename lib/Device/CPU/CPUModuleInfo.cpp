#include "CPUModuleInfo.h"

using namespace opencrun;

llvm::MDNode *CPUModuleInfo::getCustomInfo(llvm::StringRef Name) const {
  if (auto *Info = Mod.getNamedMetadata("opencrun.module.info"))
    for (auto *MD : Info->operands()) {
      llvm::MDString *S = llvm::cast<llvm::MDString>(MD->getOperand(0));
      if (S->getString() == Name)
        return MD;
    }
  return nullptr;
}
