
#ifndef OPENCRUN_UTIL_EMITCUSTOMLLVMONLYACTION_H
#define OPENCRUN_UTIL_EMITCUSTOMLLVMONLYACTION_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Frontend/CompilerInstance.h"

#include <map>

namespace opencrun {

// OpenCRun address spaces map, which resembles FakeAddrSpaceMap.
const unsigned OpenCRunAddrSpaceMap[] = {
  1, // opencl_global
  2, // opencl_local
  3  // opencl_constant
};
    
class EmitCustomLLVMOnlyAction : public clang::EmitLLVMOnlyAction {
public:
  typedef llvm::SmallVector<unsigned, 8> ASContainer;
  typedef std::map<std::string, ASContainer> KernelToASMap;

  typedef ASContainer::const_iterator AS_const_iterator;

public:
  EmitCustomLLVMOnlyAction(llvm::LLVMContext *C = 0) : EmitLLVMOnlyAction(C) {}

  clang::ASTConsumer *CreateASTConsumer(clang::CompilerInstance &CI,
                                        llvm::StringRef InFile);
  
  // Invoked to add ASs metadata to the specified llvm::Module.
  void AddKernelArgMetadata(llvm::Module &Mod, llvm::LLVMContext &Ctx);

private:
  KernelToASMap KernToAS;
};

} // End namespace opencrun. 

#endif // OPENCRUN_UTIL_EMITCUSTOMLLVMONLYACTION_H
