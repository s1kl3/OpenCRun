#include "opencrun/Util/LLVMCodeGenAction.h"
#include "opencrun/Util/OpenCLTypeSystem.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclGroup.h"
#include "clang/Basic/AddressSpaces.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

using namespace opencrun;

namespace opencrun {

class LLVMCodeGenConsumer : public clang::ASTConsumer {
public:
  LLVMCodeGenConsumer(clang::DiagnosticsEngine &Diags,
                      const clang::CodeGenOptions &CGOpts,
                      const clang::TargetOptions &TargetOpts,
                      const clang::LangOptions &LangOpts,
                      const std::string &InFile,
                      llvm::LLVMContext &C)
   : Gen(clang::CreateLLVMCodeGen(Diags, InFile, CGOpts, TargetOpts, C)),
     TheModule(Gen->GetModule()), TyGen(*TheModule) {}

  llvm::Module *takeModule() { return TheModule.release(); }

  void Initialize(clang::ASTContext &Ctx) override {
    Gen->Initialize(Ctx);
  }

  void HandleTranslationUnit(clang::ASTContext &Ctx) override {
    Gen->HandleTranslationUnit(Ctx);

    if (!TheModule) return;

    llvm::Module *M = Gen->ReleaseModule();
    if (!M) {
      TheModule.release();
      return;
    }

    assert(TheModule.get() == M &&
           "Unexpected module change during IR generation");
  }

  void HandleTagDeclDefinition(clang::TagDecl *D) override {
    Gen->HandleTagDeclDefinition(D);
  }

  bool HandleTopLevelDecl(clang::DeclGroupRef D) override {
    Gen->HandleTopLevelDecl(D);

    for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I)
      if (clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(*I))
        if (FD->hasAttr<clang::OpenCLKernelAttr>() && FD->hasBody())
          regenerateKernelInfo(FD);

    return true;
  }

  void HandleCXXStaticMemberVarInstantiation(clang::VarDecl *VD) override {
    Gen->HandleCXXStaticMemberVarInstantiation(VD);
  }

  void CompleteTentativeDefinition(clang::VarDecl *D) override {
    Gen->CompleteTentativeDefinition(D);
  }

  void HandleVTable(clang::CXXRecordDecl *RD,
                    bool DefinitionRequired) override {
    Gen->HandleVTable(RD, DefinitionRequired);
  }

  void HandleLinkerOptionPragma(llvm::StringRef Opts) override {
    Gen->HandleLinkerOptionPragma(Opts);
  }

  void HandleDetectMismatch(llvm::StringRef Name,
                            llvm::StringRef Value) override {
    Gen->HandleDetectMismatch(Name, Value);
  }

  void HandleDependentLibrary(llvm::StringRef Opts) override {
    Gen->HandleDependentLibrary(Opts);
  }

private:
  void regenerateKernelInfo(const clang::FunctionDecl *FD);

private:
  std::unique_ptr<clang::CodeGenerator> Gen;
  std::unique_ptr<llvm::Module> TheModule;
  opencl::TypeGenerator TyGen;
};

}

static bool isKernelMetadata(llvm::Value *V, llvm::StringRef Name) {
  if (llvm::MDNode *MD = llvm::dyn_cast<llvm::MDNode>(V))
    if (llvm::MDString *S = llvm::dyn_cast<llvm::MDString>(MD->getOperand(0)))
      return S->getString() == Name;

  return false;
}

static llvm::MDNode *getKernelMD(llvm::NamedMDNode *Kernels,
                                  llvm::Function *F) {
  for (unsigned i = 0, e = Kernels->getNumOperands(); i != e; ++i) {
    llvm::MDNode *CurMD = Kernels->getOperand(i);
    if (CurMD->getOperand(0) == F)
      return CurMD;
  }
  return 0;
}

void LLVMCodeGenConsumer::regenerateKernelInfo(const clang::FunctionDecl *FD) {
  std::string KernelName = FD->getNameInfo().getAsString();

  // OpenCL kernel names are not mangled!
  llvm::Function *F = TheModule->getFunction(KernelName);

  if (!F) return;

  clang::ASTContext &ASTCtx = FD->getASTContext();
  llvm::LLVMContext &Ctx = F->getContext();
  llvm::Type *I32Ty = llvm::Type::getInt32Ty(Ctx);

  llvm::SmallVector<llvm::Value*, 8> ArgAddrSpace;
  ArgAddrSpace.push_back(llvm::MDString::get(Ctx, "kernel_arg_addr_space"));

  llvm::SmallVector<llvm::Value*, 8> Sign;
  Sign.push_back(llvm::MDString::get(Ctx, "signature"));

  for (unsigned I = 0, E = FD->getNumParams(); I != E; ++I) {
    clang::QualType ParamTy = FD->getParamDecl(I)->getType();

    // Recompute address space info
    unsigned LangAS = ParamTy->isPointerType()
                       ? ParamTy->getPointeeType().getAddressSpace() : 0;
    unsigned AS = opencl::convertAddressSpace(LangAS);
    ArgAddrSpace.push_back(llvm::ConstantInt::get(I32Ty, AS));

    // Generate argument type descriptor
    Sign.push_back(TyGen.get(ASTCtx, ParamTy).getMDNode());
  }

  llvm::NamedMDNode *Kernels = TheModule->getNamedMetadata("opencl.kernels");
  assert(Kernels && "No 'opencl.kernels' metadata!");

  llvm::MDNode *KernMD = getKernelMD(Kernels, F);
  assert(KernMD && "Missing kernel infos!");

  llvm::SmallVector<llvm::Value *, 8> MDs;
  for (unsigned I = 0, E = KernMD->getNumOperands(); I != E; ++I) {
    llvm::Value *Cur = KernMD->getOperand(I);
    if (isKernelMetadata(Cur, "kernel_arg_addr_space"))
      Cur = llvm::MDNode::get(Ctx, ArgAddrSpace);

    MDs.push_back(Cur);
  }

  llvm::SmallVector<llvm::Value *, 8> OpenCRunInfo;
  OpenCRunInfo.push_back(llvm::MDString::get(Ctx, "kernel_info"));
  OpenCRunInfo.push_back(llvm::MDNode::get(Ctx, Sign));

  MDs.push_back(llvm::MDNode::get(Ctx, OpenCRunInfo));

  KernMD->replaceAllUsesWith(llvm::MDNode::get(Ctx, MDs));
}

LLVMCodeGenAction::LLVMCodeGenAction(llvm::LLVMContext *Ctx)
 : Context(Ctx ? Ctx : new llvm::LLVMContext), OwnContext(!Ctx) {}

LLVMCodeGenAction::~LLVMCodeGenAction() {
  TheModule.reset();
  if (OwnContext)
    delete Context;
}

clang::ASTConsumer *LLVMCodeGenAction::
CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) {
  Consumer = new LLVMCodeGenConsumer(CI.getDiagnostics(), CI.getCodeGenOpts(),
                                     CI.getTargetOpts(), CI.getLangOpts(),
                                     InFile, *Context);
  return Consumer;
}

void LLVMCodeGenAction::EndSourceFileAction() {
  if (!getCompilerInstance().hasASTConsumer())
    return;

  TheModule.reset(Consumer->takeModule());
}
