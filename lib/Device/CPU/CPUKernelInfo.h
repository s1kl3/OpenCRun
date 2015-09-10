#ifndef CPUKERNEL_INFO_H
#define CPUKERNEL_INFO_H

#include "opencrun/Util/ModuleInfo.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"

namespace opencrun {

struct CPUKernelInfo : public KernelInfo {
  CPUKernelInfo(const KernelInfo &KI)
   : KernelInfo(KI) {}

  uint32_t getNumStaticLocalStructs() const {
    if (llvm::MDNode *MD = getCustomInfo("static_local_infos")) {
      auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(1));
      return Cst->getZExtValue();
    }
    return 0; 
  }

  uint64_t getStaticLocalSize() const {
    if (llvm::MDNode *MD = getCustomInfo("static_local_infos")) {
      auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(2));
      return Cst->getZExtValue();
    }
    return 0;
  }

  unsigned getNumStaticLocalAreas() const {
    if (llvm::MDNode *MD = getCustomInfo("static_local_infos"))
      // The first two operands are the MDString "static_local_info" 
      // and the i64 storage size required for automatic local
      // variables declared in the kernel's call-chain.
      return (MD->getNumOperands() - 3);
    return 0;
  }

  uint32_t getStaticLocalIndex(unsigned Idx) const {
    llvm::MDNode *MD = getCustomInfo("static_local_infos");
    assert(MD && "Automatic local storage infos not available!");
    llvm::MDNode *StructMD = llvm::dyn_cast<llvm::MDNode>(MD->getOperand(Idx + 3));
    assert(StructMD && "Unexpected meta-data operand type!");
    auto *Cst =
      llvm::mdconst::extract<llvm::ConstantInt>(StructMD->getOperand(0));
    return Cst->getZExtValue();
  }

  uint64_t getStaticLocalOffset(unsigned Idx) const {
    llvm::MDNode *MD = getCustomInfo("static_local_infos");
    assert(MD && "Automatic local storage infos not available!");
    llvm::MDNode *StructMD = llvm::dyn_cast<llvm::MDNode>(MD->getOperand(Idx + 3));
    assert(StructMD && "Unexpected meta-data operand type!");
    auto *Cst =
      llvm::mdconst::extract<llvm::ConstantInt>(StructMD->getOperand(1));
    return Cst->getZExtValue();
  }
};

}

#endif
