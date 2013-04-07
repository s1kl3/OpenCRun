
#include "opencrun/Util/OpenCLMetadataHandler.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/ErrorHandling.h"

using namespace opencrun;

//
// ParserState implementation.
//

namespace {

class SignatureBuilder {
public:
  typedef llvm::SmallVector<llvm::Type *, 4> TypesContainer;

public:
  SignatureBuilder(const llvm::StringRef Name,
                   llvm::Module &Mod) : Name(Name),
                                        Mod(Mod) {
    ResetState();
    InitTargetInfo();
  }

public:
  void SetUnsignedPrefix() { Unsigned = true; }
  void SetLongPrefix() { Long = true; }

  void SetInteger() { Integer = true; }
  void SetSizeT() { SizeT = true; }
  void SetVoid() { Void = true; }

  void SetTypeDone() {
    llvm::Type *Ty;

    if(Integer && Unsigned && !Long)
      Ty = GetUIntTy();
    else if(Integer && Unsigned && Long)
      Ty = GetULongTy();
    else if(SizeT)
      Ty = GetSizeTTy();
    else if(Void)
      Ty = GetVoidTy();
    else
      llvm_unreachable("Type not specified");

    Types.push_back(Ty);

    ResetState();
  }

  llvm::Function *CreateFunction() {
    if(Types.size() < 1)
      llvm_unreachable("Not enough arguments");

    llvm::Type *RetTy = Types.front();
    Types.erase(Types.begin());

    llvm::FunctionType *FunTy = llvm::FunctionType::get(RetTy, Types, false);

    llvm::Function *Fun;
    Fun = llvm::Function::Create(FunTy,
                                 llvm::Function::ExternalLinkage,
                                 Name,
                                 &Mod);

    return Fun;
  }

private:
  void ResetState() {
    Unsigned = Long = false;
    Integer = SizeT = Void = false;
  }

  void InitTargetInfo() {
    TargetOpts = new clang::TargetOptions();
    TargetOpts->Triple = Mod.getTargetTriple();

    DiagIDs = new clang::DiagnosticIDs();

    Diag = new clang::DiagnosticsEngine(DiagIDs, NULL);

    TargetInfo.reset(clang::TargetInfo::CreateTargetInfo(*Diag, &*TargetOpts));

    delete Diag;
  }

  llvm::Type *GetUIntTy() {
    return llvm::Type::getInt16Ty(Mod.getContext());
  }

  llvm::Type *GetULongTy() {
    return llvm::Type::getInt64Ty(Mod.getContext());
  }

  llvm::Type *GetSizeTTy() {
    return llvm::Type::getIntNTy(Mod.getContext(), TargetInfo->getSizeType());
  }

  llvm::Type *GetVoidTy() {
    return llvm::Type::getVoidTy(Mod.getContext());
  }

private:
  const llvm::StringRef Name;
  llvm::Module &Mod;

  // Prefixes.
  bool Unsigned, Long;

  // Types.
  bool Integer, SizeT, Void;

  TypesContainer Types;

  clang::DiagnosticsEngine *Diag;
  llvm::OwningPtr<clang::TargetInfo> TargetInfo;
  llvm::IntrusiveRefCntPtr<clang::TargetOptions> TargetOpts;
  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagIDs;
};

} // End anonymous namespace.

//
// OpenCLMetadataHandler implementation.
//

OpenCLMetadataHandler::OpenCLMetadataHandler(llvm::Module &Mod) : Mod(Mod) {
  // TODO: replace with oclgen.
  #define BUILTIN(N, S) \
    Builtins["__builtin_ocl_" N] = S;
  #include "opencrun/Util/Builtins.def"
  #undef BUILTIN
}

llvm::Function *
OpenCLMetadataHandler::GetKernel(llvm::StringRef KernName) const {
  llvm::Function *Kern = Mod.getFunction(KernName);

  if(!Kern)
    return NULL;

  llvm::NamedMDNode *KernsMD = Mod.getNamedMetadata("opencl.kernels");

  if(!KernsMD)
    return NULL;

  bool IsKernel = false;

  for(unsigned I = 0, E = KernsMD->getNumOperands(); I != E && !IsKernel; ++I) {
    llvm::MDNode &KernMD = *KernsMD->getOperand(I);

    IsKernel = KernMD.getOperand(KernelSignatureMID) == Kern;
  }

  return IsKernel ? Kern : NULL;
}

clang::LangAS::ID
OpenCLMetadataHandler::GetArgAddressSpace(llvm::Function &Kern, unsigned I) {
  llvm::Module &Mod = *Kern.getParent();

  llvm::NamedMDNode *OCLMetadata = Mod.getNamedMetadata("opencl.kernels");

  if (!OCLMetadata) return clang::LangAS::Last;

  for (unsigned k = 0, ke = OCLMetadata->getNumOperands(); k != ke; ++k) {
    llvm::MDNode *KernMD = OCLMetadata->getOperand(k);

    assert(KernMD->getNumOperands() > 0 && 
           llvm::isa<llvm::Function>(KernMD->getOperand(0)) && 
           "Bad kernel metadata!");

    
    if (KernMD->getOperand(0) != &Kern) continue;

    for (unsigned i = 1, e = KernMD->getNumOperands(); i != e; ++i) {
      llvm::MDNode *ArgsMD = llvm::cast<llvm::MDNode>(KernMD->getOperand(i));

      assert(ArgsMD->getNumOperands() > I + 1 && 
             llvm::isa<llvm::MDString>(ArgsMD->getOperand(0)) && 
             "Bad arg metadata!");

      llvm::MDString *InfoKind = 
        llvm::cast<llvm::MDString>(ArgsMD->getOperand(0));

      if (InfoKind->getString() != "kernel_arg_addr_space") continue;

      // The translation is based on the FakeAddressSpaceMap defined in 
      // clang/lib/AST/ASTContext.cpp
      llvm::ConstantInt *AS =
        llvm::cast<llvm::ConstantInt>(ArgsMD->getOperand(I + 1));
      switch (AS->getZExtValue()) {
      case 1: return clang::LangAS::opencl_global;
      case 2: return clang::LangAS::opencl_local;
      case 3: return clang::LangAS::opencl_constant;
      default: return clang::LangAS::Last;
      }
    }
  }

  return clang::LangAS::Last;
}

llvm::Function *OpenCLMetadataHandler::GetBuiltin(llvm::StringRef Name) {
  // Mangle the name.
  llvm::SmallString<32> MangledName("__builtin_ocl_");
  MangledName += Name;

  // Function is not a built-in.
  if(!Builtins.count(MangledName)) {
    std::string ErrMsg = "Unknown builtin: ";
    ErrMsg += Name;

    llvm_unreachable(ErrMsg.c_str());
  }

  llvm::Function *Func = Mod.getFunction(MangledName);

  // Built-in declaration found inside the module.
  if(Func) {
    if(!HasRightSignature(Func, Builtins[MangledName])) {
      std::string ErrMsg = "Malformed builtin declaration: ";
      ErrMsg += MangledName;

      llvm_unreachable(ErrMsg.c_str());
    }
  }

  // Built-in not found in the module, create it.
  else
    Func = BuildBuiltin(MangledName, Builtins[MangledName]);

  return Func;
}

bool OpenCLMetadataHandler::HasRightSignature(
                              const llvm::Function *Func,
                              const llvm::StringRef Signature) const {
  // TODO: do the check. Ask Magni about name mangling.
  return true;
}

llvm::Function *
OpenCLMetadataHandler::BuildBuiltin(const llvm::StringRef Name,
                                    const llvm::StringRef Signature) {
  llvm::StringRef::iterator I = Signature.begin(), E = Signature.end();

  SignatureBuilder Bld(Name, Mod);

  while(I != E) {
    // Scan prefix.

    if(*I == 'U') {
      I++;
      Bld.SetUnsignedPrefix();
    }

    if(*I == 'L') {
      I++;
      Bld.SetLongPrefix();
    }

    // Scan type.

    switch(*I++) {
    case 'i':
      Bld.SetInteger();
      break;

    case 'z':
      Bld.SetSizeT();
      break;

    case 'v':
      Bld.SetVoid();
      break;

    default:
      llvm_unreachable("Unknown type");
    }

    // TODO: scan suffix.

    // Build type.
    Bld.SetTypeDone();
  }

  return Bld.CreateFunction();
}
