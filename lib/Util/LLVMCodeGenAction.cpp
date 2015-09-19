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
                      const clang::HeaderSearchOptions &HeaderSearchOpts,
                      const clang::PreprocessorOptions &PPOpts,
                      const clang::CodeGenOptions &CodeGenOpts,
                      const std::string &InFile,
                      llvm::LLVMContext &C)
   : Gen(clang::CreateLLVMCodeGen(Diags, InFile, HeaderSearchOpts, PPOpts,
                                  CodeGenOpts, C)),
     TyGen(*Gen->GetModule()) {}

  std::unique_ptr<llvm::Module> takeModule() { return std::move(TheModule); }

  void Initialize(clang::ASTContext &Ctx) override {
    Gen->Initialize(Ctx);
    TheModule.reset(Gen->GetModule());
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

  void HandleVTable(clang::CXXRecordDecl *RD) override {
    Gen->HandleVTable(RD);
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

static bool isKernelMetadata(llvm::Metadata *MD, llvm::StringRef Name) {
  if (llvm::MDNode *MDN = llvm::dyn_cast<llvm::MDNode>(MD))
    if (auto *S = llvm::dyn_cast<llvm::MDString>(MDN->getOperand(0).get()))
      return S->getString() == Name;

  return false;
}

static llvm::MDNode *getKernelMD(llvm::NamedMDNode *Kernels,
                                  llvm::Function *F) {
  for (unsigned i = 0, e = Kernels->getNumOperands(); i != e; ++i) {
    auto *CurMD = Kernels->getOperand(i);
    if (llvm::mdconst::extract<llvm::Constant>(CurMD->getOperand(0)) == F)
      return CurMD;
  }
  return 0;
}

static void replaceKernelMD(llvm::NamedMDNode *Kernels, llvm::Function *F,
                            llvm::MDNode *KernMD) {
  for (unsigned i = 0, e = Kernels->getNumOperands(); i != e; ++i) {
    auto *CurMD = Kernels->getOperand(i);
    if (llvm::mdconst::extract<llvm::Constant>(CurMD->getOperand(0)) == F) {
      Kernels->setOperand(i, KernMD);
      break;
    }
  }
}

void LLVMCodeGenConsumer::regenerateKernelInfo(const clang::FunctionDecl *FD) {
  std::string KernelName = FD->getNameInfo().getAsString();

  // OpenCL kernel names are not mangled!
  llvm::Function *F = TheModule->getFunction(KernelName);

  if (!F)
    return;

  clang::ASTContext &ASTCtx = FD->getASTContext();
  llvm::LLVMContext &Ctx = F->getContext();
  llvm::Type *I32Ty = llvm::Type::getInt32Ty(Ctx);

  llvm::SmallVector<llvm::Metadata*, 8> ArgAddrSpace;
  ArgAddrSpace.push_back(llvm::MDString::get(Ctx, "kernel_arg_addr_space"));

  llvm::SmallVector<llvm::Metadata*, 8> Sign;
  Sign.push_back(llvm::MDString::get(Ctx, "signature"));

  for (unsigned I = 0, E = FD->getNumParams(); I != E; ++I) {
    const clang::ParmVarDecl *ParamDecl = FD->getParamDecl(I);
    clang::QualType ParamTy = FD->getParamDecl(I)->getType();

    // Recompute address space info
    unsigned LangAS = ParamTy->isPointerType()
                       ? ParamTy->getPointeeType().getAddressSpace() : 0;
    unsigned AS = opencl::convertAddressSpace(LangAS);
    ArgAddrSpace.push_back(llvm::ConstantAsMetadata::get(
                              llvm::ConstantInt::get(I32Ty, AS)));

    // Access qualifiers
    const clang::OpenCLImageAccessAttr *CLIA =
      ParamDecl->getAttr<clang::OpenCLImageAccessAttr>();

    // Generate argument type descriptor
    Sign.push_back(TyGen.get(ASTCtx, ParamTy, CLIA).getMDNode());
  }

  llvm::NamedMDNode *Kernels = TheModule->getNamedMetadata("opencl.kernels");
  assert(Kernels && "No 'opencl.kernels' metadata!");

  llvm::MDNode *KernMD = getKernelMD(Kernels, F);
  assert(KernMD && "Missing kernel infos!");

  llvm::SmallVector<llvm::Metadata *, 8> MDs;
  for (unsigned I = 0, E = KernMD->getNumOperands(); I != E; ++I) {
    auto *Cur = KernMD->getOperand(I).get();
    if (isKernelMetadata(Cur, "kernel_arg_addr_space"))
      Cur = llvm::MDNode::get(Ctx, ArgAddrSpace);

    MDs.push_back(Cur);
  }

  llvm::SmallVector<llvm::Metadata *, 8> CustomInfo;
  CustomInfo.push_back(llvm::MDString::get(Ctx, "custom_info"));
  CustomInfo.push_back(llvm::MDNode::get(Ctx, Sign));

  MDs.push_back(llvm::MDNode::get(Ctx, CustomInfo));

  auto *NewKernMD = llvm::MDNode::get(Ctx, MDs);
  replaceKernelMD(Kernels, F, NewKernMD);
}

LLVMCodeGenAction::LLVMCodeGenAction(llvm::LLVMContext &Ctx)
 : Context(Ctx) {}

LLVMCodeGenAction::~LLVMCodeGenAction() {
  TheModule.reset();
}

std::unique_ptr<clang::ASTConsumer> LLVMCodeGenAction::
CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) {
  std::unique_ptr<LLVMCodeGenConsumer> Result(
    new LLVMCodeGenConsumer(CI.getDiagnostics(), CI.getHeaderSearchOpts(),
                            CI.getPreprocessorOpts(), CI.getCodeGenOpts(),
                            InFile, Context));
  Consumer = Result.get();
  return std::move(Result);
}

void LLVMCodeGenAction::EndSourceFileAction() {
  if (!getCompilerInstance().hasASTConsumer())
    return;

  TheModule = std::move(Consumer->takeModule());
}
