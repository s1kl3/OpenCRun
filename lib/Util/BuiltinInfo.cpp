#include "opencrun/Util/BuiltinInfo.h"

#include "llvm/ADT/Twine.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"

using namespace opencrun;

static
llvm::Type *parseType(llvm::LLVMContext &Ctx,
                      const llvm::DataLayout &DL,
                      llvm::StringRef::iterator &I,
                      llvm::StringRef::iterator E) {
  if (I == E) return 0;

  switch (*I++) {
  case 'P': {
    llvm::Type *PointeeTy = parseType(Ctx, DL, I, E);
    assert(PointeeTy);
    return PointeeTy->getPointerTo();
  }
  case 'c': return llvm::Type::getInt8Ty(Ctx);
  case 's': return llvm::Type::getInt16Ty(Ctx);
  case 'i': return llvm::Type::getInt32Ty(Ctx);
  case 'l': return llvm::Type::getInt64Ty(Ctx);
  case 'e': return llvm::StructType::get(Ctx)->getPointerTo();
  case 'z': return llvm::Type::getIntNTy(Ctx, DL.getPointerSizeInBits());
  case 'v': return llvm::Type::getVoidTy(Ctx);
  default: break;
  }

  return 0;
}

static
llvm::FunctionType *buildFunctionType(llvm::LLVMContext &Ctx,
                                      const llvm::DataLayout &DL,
                                      llvm::StringRef Format) {
  llvm::StringRef::iterator I = Format.begin(), E = Format.end();

  llvm::Type *ResTy = parseType(Ctx, DL, I, E);
  assert(ResTy);

  llvm::SmallVector<llvm::Type *, 8> Args;
  while (I != E) {
    llvm::Type *Ty = parseType(Ctx, DL, I, E);
    assert(Ty);

    Args.push_back(Ty);
  }

  return llvm::FunctionType::get(ResTy, Args, false);
}


llvm::Function *DeviceBuiltinInfo::getPrototype(llvm::Module &Mod,
                                                llvm::StringRef Name,
                                                llvm::StringRef Format) {
  if (llvm::Function *F = Mod.getFunction(Name))
    return F;

  llvm::LLVMContext &Ctx = Mod.getContext();
  llvm::DataLayout DL(&Mod);

  return llvm::Function::Create(buildFunctionType(Ctx, DL, Format),
                                llvm::Function::ExternalLinkage, Name, &Mod);
}
