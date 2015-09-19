#ifndef OPENCRUN_UTIL_LLVMCODEGENACTION_H
#define OPENCRUN_UTIL_LLVMCODEGENACTION_H

#include "clang/Frontend/FrontendAction.h"

namespace llvm {
class LLVMContext;
class Module;
}

namespace opencrun {

class LLVMCodeGenConsumer;

class LLVMCodeGenAction : public clang::ASTFrontendAction {
public:
  LLVMCodeGenAction(llvm::LLVMContext &Ctx);
  ~LLVMCodeGenAction();

  std::unique_ptr<llvm::Module> takeModule() { return std::move(TheModule); }

protected:
  std::unique_ptr<clang::ASTConsumer>
      CreateASTConsumer(clang::CompilerInstance &CI,
                        llvm::StringRef InFile) override;

  void EndSourceFileAction() override;

private:
  LLVMCodeGenConsumer *Consumer;
  std::unique_ptr<llvm::Module> TheModule;
  llvm::LLVMContext &Context;
};

}

#endif
