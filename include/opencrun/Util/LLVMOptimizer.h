#ifndef OPENCRUN_UTIL_LLVMOPTIMIZERACTION_H
#define OPENCRUN_UTIL_LLVMOPTIMIZERACTION_H

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

namespace clang {
class CodeGenOptions;
class LangOptions;
class TargetOptions;
}

namespace llvm {
class Module;
class TargetMachine;
}

namespace opencrun {

class Device;

struct LLVMOptimizerParams {
  LLVMOptimizerParams(const clang::LangOptions &LangOpts,
                      const clang::CodeGenOptions &GodeGenOpts,
                      const clang::TargetOptions &TargetOpts)
   : LangOpts(LangOpts), CodeGenOpts(CodeGenOpts), TargetOpts(TargetOpts) {}
  
  const clang::LangOptions &LangOpts;
  const clang::CodeGenOptions &CodeGenOpts;
  const clang::TargetOptions &TargetOpts;
};

class LLVMOptimizer {
public:
  LLVMOptimizer(const clang::LangOptions &LangOpts,
                const clang::CodeGenOptions &CodeGenOpts,
                const clang::TargetOptions &TargetOpts,
                Device &Dev);

  void run(llvm::Module *M);

private:
  LLVMOptimizerParams Params;
  llvm::PassManagerBuilder PMBuilder;
  Device &Dev;
};

}

#endif
