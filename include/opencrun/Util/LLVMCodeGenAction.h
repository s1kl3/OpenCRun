#ifndef OPENCRUN_UTIL_LLVMCODEGENACTION_H
#define OPENCRUN_UTIL_LLVMCODEGENACTION_H

#include "clang/Frontend/FrontendAction.h"
#include "llvm/ADT/OwningPtr.h"

namespace llvm {
class LLVMContext;
class Module;
}

namespace opencrun {

class LLVMCodeGenConsumer;

class LLVMCodeGenAction : public clang::ASTFrontendAction {
public:
  LLVMCodeGenAction(llvm::LLVMContext *Ctx = 0);
  ~LLVMCodeGenAction();

  llvm::Module *takeModule() { return TheModule.take(); }

protected:
  clang::ASTConsumer *CreateASTConsumer(clang::CompilerInstance &CI,
                                        llvm::StringRef InFile) LLVM_OVERRIDE;

  void EndSourceFileAction() LLVM_OVERRIDE;

private:
  LLVMCodeGenConsumer *Consumer;
  llvm::OwningPtr<llvm::Module> TheModule;
  llvm::LLVMContext *Context;
  bool OwnContext;
};

}

#endif
