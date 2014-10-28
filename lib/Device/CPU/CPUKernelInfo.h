#ifndef CPUKERNEL_INFO_H
#define CPUKERNEL_INFO_H

#include "opencrun/Util/ModuleInfo.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"

namespace opencrun {

struct CPUKernelInfo : public KernelInfo {
  CPUKernelInfo(const KernelInfo &KI)
   : KernelInfo(KI) {}

  uint64_t getStaticLocalSize() const {
    if (llvm::MDNode *MD = getCustomInfo("static_local_size"))
      return llvm::cast<llvm::ConstantInt>(MD->getOperand(1))->getZExtValue();
    return 0;
  }
};

}

#endif
