#ifndef OPENCRUN_UTIL_LLVMOPTIMIZERACTION_H
#define OPENCRUN_UTIL_LLVMOPTIMIZERACTION_H

#include "llvm/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

namespace clang {
class CodeGenOptions;
class LangOptions;
class TargetOptions;
}

namespace llvm {
class Module;
}

namespace opencrun {

struct LLVMOptimizerParams {
  LLVMOptimizerParams(
                      const clang::LangOptions &langopts,
                      const clang::CodeGenOptions &cgopts,
                      const clang::TargetOptions &targetopts)
   : LangOpts(langopts), CodeGenOpts(cgopts), TargetOpts(targetopts) {}
  
  const clang::LangOptions &LangOpts;
  const clang::CodeGenOptions &CodeGenOpts;
  const clang::TargetOptions &TargetOpts;
};

class LLVMOptimizerBase {
public:
  LLVMOptimizerBase(const clang::LangOptions &langopts,
                    const clang::CodeGenOptions &cgopts,
                    const clang::TargetOptions &targetopts);

  void init();

  void run(llvm::Module *M);

private:
  LLVMOptimizerParams Params;
  llvm::PassManagerBuilder PMBuilder;
  std::unique_ptr<llvm::PassManager> MPM;
  std::unique_ptr<llvm::FunctionPassManager> FPM;

  template<class InterfaceTy> friend class LLVMOptimizer;
};

template<class InterfaceTy>
struct LLVMOptimizerInterfaceTraits;

template<class InterfaceTy>
class LLVMOptimizer {
public:
  LLVMOptimizer(const clang::LangOptions &langopts,
                const clang::CodeGenOptions &cgopts,
                const clang::TargetOptions &targetopts,
                InterfaceTy &IFace)
   : Base(langopts, cgopts, targetopts) {
    LLVMOptimizerInterfaceTraits<InterfaceTy>::
      registerExtensions(IFace, Base.PMBuilder, Base.Params);
    Base.init();
  }

  void run(llvm::Module *M) { Base.run(M); }

private:
  LLVMOptimizerBase Base;
};

}

#endif
