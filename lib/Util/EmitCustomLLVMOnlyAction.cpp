
#include "opencrun/Util/EmitCustomLLVMOnlyAction.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/MDBuilder.h"
#include "clang/AST/Attr.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/AddressSpaces.h"
#include "clang/Frontend/MultiplexConsumer.h"

using namespace opencrun;

namespace {

class EmitCustomLLVMOnlyVisitor 
  : public clang::RecursiveASTVisitor<EmitCustomLLVMOnlyVisitor> {
public:
  explicit EmitCustomLLVMOnlyVisitor(clang::ASTContext &ASTCtx,
                                     EmitCustomLLVMOnlyAction::KernelToASMap &KernToAS) 
    : ASTCtx(ASTCtx),
      KernToAS(KernToAS) { }

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    // We are interested only in OpenCL C kernel functions.
    if (!FD->hasAttr<clang::OpenCLKernelAttr>())
      return true;

    std::string KernelFnName = FD->getNameInfo().getAsString();
    
    for (unsigned I = 0, E = FD->getNumParams(); I != E; ++I) {
      const clang::ParmVarDecl *Parm = FD->getParamDecl(I);
      clang::QualType ParmTy = Parm->getType();

      if (ParmTy->isPointerType()) {
        clang::QualType PointeeTy = ParmTy->getPointeeType();

        unsigned AS = PointeeTy.getAddressSpace();

        if (AS < clang::LangAS::Offset || 
            AS >= clang::LangAS::Offset + clang::LangAS::Count)
          KernToAS[KernelFnName].push_back(AS);
          
        KernToAS[KernelFnName].push_back(OpenCRunAddrSpaceMap[AS - clang::LangAS::Offset]);
      }
    }

    return true;
  }

private:
  clang::ASTContext &ASTCtx;  
  EmitCustomLLVMOnlyAction::KernelToASMap &KernToAS;
};

class EmitCustomLLVMOnlyConsumer : public clang::ASTConsumer {
public:
  explicit EmitCustomLLVMOnlyConsumer(clang::ASTContext &Context,
                                      EmitCustomLLVMOnlyAction::KernelToASMap &KernToAS)
    : Visitor(Context, KernToAS) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  EmitCustomLLVMOnlyVisitor Visitor;
};

} // End anonymouse namespace.

clang::ASTConsumer *
EmitCustomLLVMOnlyAction::CreateASTConsumer(clang::CompilerInstance &CI, 
                                            llvm::StringRef InFile) {

  llvm::SmallVector<clang::ASTConsumer *, 2> Chain;
  Chain.push_back(EmitLLVMOnlyAction::CreateASTConsumer(CI, InFile));
  Chain.push_back(new EmitCustomLLVMOnlyConsumer(CI.getASTContext(), KernToAS));

  return new clang::MultiplexConsumer(Chain);

}

void EmitCustomLLVMOnlyAction::AddKernelArgMetadata(llvm::Module &Mod,
                                                    llvm::LLVMContext &Ctx) {
  
  llvm::NamedMDNode *OpenCLKernelMetadata =
    Mod.getOrInsertNamedMetadata("opencl.kernels");

  assert(OpenCLKernelMetadata && "No OpenCL kernel function metadata!");

  llvm::SmallVector<llvm::MDNode *, 5> SavedMDNodes;
  // Traverse kernel functions.
  for (unsigned KI = 0, KE = OpenCLKernelMetadata->getNumOperands(); KI != KE; ++KI) {
    llvm::SmallVector<llvm::Value *, 10> NewVals;
    
    llvm::MDNode *KernMD = OpenCLKernelMetadata->getOperand(KI);

    assert(KernMD->getNumOperands() > 0 && 
        llvm::isa<llvm::Function>(KernMD->getOperand(0)) && 
        "Bad kernel metadata!");

    llvm::Function *KernFn = llvm::cast<llvm::Function>(KernMD->getOperand(0));
    std::string KernFnName = KernFn->getName().str();

    // The current kernel function has no AS qualifier for its arguments so we
    // add its MDNode to our container as is and skip to the next kernel function.
    if (!KernToAS.count(KernFnName)) {
      SavedMDNodes.push_back(KernMD);
      continue;
    }

    // Traverse current kernel function operands.
    for (unsigned I = 0, E = KernMD->getNumOperands(); I != E; ++I) {
      NewVals.push_back(KernMD->getOperand(I));
    }

    // Prepare Values to insert as additional metadata.
    llvm::SmallVector<llvm::Value*, 8> ASquals;
    ASquals.push_back(llvm::MDString::get(Ctx, "opencrun_kernel_arg_addr_space"));    

    for (AS_const_iterator I = KernToAS[KernFnName].begin(),
                           E = KernToAS[KernFnName].end();
                           I != E;
                           ++I)
      ASquals.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), *I));

    // Add new metadata containing OpenCRun AS infos.
    NewVals.push_back(llvm::MDNode::get(Ctx, ASquals));

    SavedMDNodes.push_back(llvm::MDNode::get(Ctx, NewVals));
  }

  // Clear all NamedMDNode content.
  OpenCLKernelMetadata->dropAllReferences();

  // Restore content with supplemental informations.
  for (llvm::SmallVector<llvm::MDNode *, 5>::iterator I = SavedMDNodes.begin(),
                                                      E = SavedMDNodes.end();
                                                      I != E;
                                                      ++I)
    OpenCLKernelMetadata->addOperand(*I);

}
