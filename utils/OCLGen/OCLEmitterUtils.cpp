#include "OCLEmitterUtils.h"
#include "OCLTarget.h"
#include "OCLType.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/ErrorHandling.h"

using namespace opencrun;

const char *opencrun::AddressSpaceQualifier(AddressSpaceKind AS) {
  switch (AS) {
  case AS_Private: return "__private";
  case AS_Global: return "__global";
  case AS_Local: return "__local";
  case AS_Constant: return "__constant";
  default: break;
  }

  llvm_unreachable("Invalid address space!");
  return 0;
}

PredicateSet opencrun::ComputePredicates(const OCLBuiltin &B,
                                         const BuiltinSign &Sign, 
                                         bool IgnoreAS) {
  PredicateSet Preds(B.getPredicates().begin(), B.getPredicates().end());
  for (unsigned i = 0, e = Sign.size(); i != e; ++i) {
    const OCLBasicType *B = Sign[i];

    while (llvm::isa<OCLPointerType>(B)) {
      const OCLPointerType *P = llvm::cast<OCLPointerType>(B);
      Preds.insert(P->getPredicates().begin(), P->getPredicates().end());
      B = llvm::cast<OCLBasicType>(&P->getBaseType());
    }

    if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(B))
      B = &V->getBaseType();

    Preds.insert(B->getPredicates().begin(), B->getPredicates().end());
  }
  if (IgnoreAS) {
    for (unsigned i = AS_Begin; i != AS_End; ++i)
      Preds.erase(OCLPredicatesTable::getAddressSpace(AddressSpaceKind(i)));
  }
  return Preds;
}

void opencrun::EmitOCLTypeSignature(llvm::raw_ostream &OS, const OCLType &T,
                                    std::string Name) {
  if (llvm::isa<OCLGroupType>(&T)) 
    llvm_unreachable("Illegal basic type!");

  if (const OCLPointerType *P = llvm::dyn_cast<OCLPointerType>(&T)) {
    // Modifiers
    if (P->hasModifier(OCLPointerType::M_Const))
      OS << "const ";
	if (P->hasModifier(OCLPointerType::M_Restrict))
	  OS << "restrict ";
	if (P->hasModifier(OCLPointerType::M_Volatile))
	  OS << "volatile ";

    // Base type
    EmitOCLTypeSignature(OS, P->getBaseType());
    OS << " ";

    // Address Space
    OS << AddressSpaceQualifier(P->getAddressSpace()) << " ";

    // Star
    OS << "*";
  } else {
    // Typen name
    OS << T;
  }

  if (Name.length())
    OS << " " << Name;
}

std::string OCLBuiltinDecorator::
getExternalName(const BuiltinSign *S, bool NoRound) const {
  if (const OCLCastBuiltin *CB =llvm::dyn_cast<OCLCastBuiltin>(&Builtin)) {
    assert(S);
    std::string buf;
    llvm::raw_string_ostream OS(buf);
    OS << CB->getName() << "_";
    EmitOCLTypeSignature(OS, *(*S)[0]);

    if (const OCLConvertBuiltin *CV = llvm::dyn_cast<OCLConvertBuiltin>(CB)) {
      if (CV->hasSaturation()) OS << "_sat";
      if (!NoRound) OS << "_" << CV->getRoundingMode();
    }

    return OS.str();
  }
  return Builtin.getName();
}

std::string OCLBuiltinDecorator::
getInternalName(const BuiltinSign *S, bool NoRound) const {
  return "__builtin_ocl_" + getExternalName(S);
}

static void EmitGroupGuardBegin(llvm::raw_ostream &OS, llvm::StringRef Group) {
  if (!Group.size()) return;
  OS << "#ifdef OPENCRUN_BUILTIN_" << Group << "\n\n";
}

static void EmitGroupGuardEnd(llvm::raw_ostream &OS, llvm::StringRef Group) {
  if (!Group.size()) return;
  OS << "#endif // OPENCRUN_BUILTIN_" << Group << "\n\n";
}


bool GroupGuardEmitter::Push(llvm::StringRef Group) {
  if (Group == Current) return false;

  EmitGroupGuardEnd(OS, Current);

  Old = Current;
  Current = Group;

  EmitGroupGuardBegin(OS, Current);

  return true;
}

void GroupGuardEmitter::Finalize() {
  EmitGroupGuardEnd(OS, Current);

  Old = Current = "";
}

static void EmitPredicatesBegin(llvm::raw_ostream &OS, 
                                const PredicateSet &Preds) {
  if (Preds.empty()) return;

  llvm::SmallVector<llvm::StringRef, 4> ExtNames;

  unsigned Count = Preds.size();
  OS << "#if";
  for (PredicateSet::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I) {
    const OCLPredicate *P = *I;
    OS << " defined(" << P->getFullName() << ")";
    if (--Count) OS << " &&";

    if (const OCLExtension *E = llvm::dyn_cast<OCLExtension>(P))
      ExtNames.push_back(E->getName());
  }

  if (ExtNames.size()) {
    for (unsigned i = 0, e = ExtNames.size(); i != e; ++i)
      OS << "\n#pragma OPENCL EXTENSION " << ExtNames[i] << " : enable\n";
  }
  OS << "\n";
}

static void EmitPredicatesEnd(llvm::raw_ostream &OS, 
                              const PredicateSet &Preds) {
  if (Preds.empty()) return;
  
  for (PredicateSet::iterator I = Preds.begin(), E = Preds.end(); I != E; ++I)
    if (const OCLExtension *E = llvm::dyn_cast<OCLExtension>(*I))
      OS << "#pragma OPENCL EXTENSION " << E->getName() << " : disable\n";
  OS << "#endif\n";
}

bool PredicatesGuardEmitter::Push(const PredicateSet &Preds) {
  if (Preds == Current) return false;

  EmitPredicatesEnd(OS, Current);

  Old = Current;
  Current = Preds;

  EmitPredicatesBegin(OS, Current);

  return true;
}

void PredicatesGuardEmitter::Finalize() {
  EmitPredicatesEnd(OS, Current);

  Old.clear();
  Current.clear();
}
