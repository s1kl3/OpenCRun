#include "OCLEmitterUtils.h"
#include "OCLTarget.h"
#include "OCLType.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/ErrorHandling.h"

using namespace opencrun;

const char *opencrun::AddressSpaceName(OCLPointerType::AddressSpace AS) {
  switch (AS) {
  case OCLPointerType::AS_Private: return "__private";
  case OCLPointerType::AS_Global: return "__global";
  case OCLPointerType::AS_Local: return "__local";
  case OCLPointerType::AS_Constant: return "__constant";
  default: break;
  }

  llvm_unreachable("Invalid address space!");
  return 0;
}

void opencrun::EmitOCLTypeSignature(llvm::raw_ostream &OS, const OCLType &T,
                                    std::string Name) {
  if (llvm::isa<OCLGroupType>(&T)) 
    llvm_unreachable("Illegal basic type!");

  if (const OCLPointerType *P = llvm::dyn_cast<OCLPointerType>(&T)) {
    // Modifiers
    if (P->hasModifier(OCLPointerType::M_Const))
      OS << "const ";

    // Base type
    EmitOCLTypeSignature(OS, P->getBaseType());
    OS << " ";

    // Address Space
    OS << AddressSpaceName(P->getAddressSpace()) << " ";

    // Star
    OS << "*";
  } else {
    // Typen name
    OS << T;
  }

  if (Name.length())
    OS << " " << Name;
}

void opencrun::ComputePredicates(const BuiltinSignature &Sign, 
                                 llvm::BitVector &Preds, bool IgnoreAS) {
  Preds.reset();
  for (unsigned i = 0, e = Sign.size(); i != e; ++i) {
    const OCLBasicType *B = Sign[i];

    while (llvm::isa<OCLPointerType>(B)) {
      const OCLPointerType *P = llvm::cast<OCLPointerType>(B);
      Preds |= P->getPredicates();
      B = llvm::cast<OCLBasicType>(&P->getBaseType());
    }

    if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(B))
      B = &V->getBaseType();

    Preds |= B->getPredicates();
  }
  if (IgnoreAS)
    Preds.reset(Pred_AS_InitValue, Pred_AS_MaxValue);
}

void opencrun::EmitPredicatesBegin(llvm::raw_ostream &OS, 
                                    const llvm::BitVector &Preds) {
  if (!Preds.any()) return;

  llvm::SmallVector<const char *, 4> Names;
  for (unsigned i = Pred_Ext_InitValue; i != Pred_Ext_MaxValue; ++i)
    if (Preds[i]) Names.push_back(PredicateName(i));

  unsigned Count = Preds.count();
  OS << "#if";
  for (unsigned i = Pred_InitValue; i != Pred_MaxValue; ++i)
    if (Preds[i]) {
      OS << " defined(__opencrun_target_" << PredicateName(i) << ")";
      if (--Count) OS << " &&";
    }
  OS << "\n";
  for (unsigned i = 0, e = Names.size(); i != e; ++i)
    OS << "#pragma OPENCL EXTENSION " << Names[i] << " : enable\n";
  OS << "\n";
}

void opencrun::EmitPredicatesEnd(llvm::raw_ostream &OS, 
                                  const llvm::BitVector &Preds) {
  if (!Preds.any()) return;
  
  for (unsigned i = Pred_Ext_InitValue; i != Pred_Ext_MaxValue; ++i)
    if (Preds[i])
      OS << "#pragma OPENCL EXTENSION " << PredicateName(i) 
         << " : disable\n";
  OS << "#endif\n\n";
}

void opencrun::EmitBuiltinGroupBegin(llvm::raw_ostream &OS, 
                                     llvm::StringRef Group) {
  if (!Group.size()) return;
  OS << "#ifdef OPENCRUN_BUILTIN_" << Group << "\n\n";
}

void opencrun::EmitBuiltinGroupEnd(llvm::raw_ostream &OS, 
                                   llvm::StringRef Group) {
  if (!Group.size()) return;
  OS << "#endif // OPENCRUN_BUILTIN_" << Group << "\n";
}
