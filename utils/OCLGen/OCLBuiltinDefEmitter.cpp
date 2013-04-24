#include "OCLBuiltinDefEmitter.h"
#include "OCLType.h"
#include "OCLBuiltin.h"

#include "llvm/Support/ErrorHandling.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace opencrun;


namespace {

OCLTypesContainer OCLTypes;
OCLBuiltinsContainer OCLBuiltins;

}

void EmitOCLTypeSignature(llvm::raw_ostream &OS, const OCLType &T,
                          bool ShowNames, unsigned Index) {
  if (llvm::isa<OCLGroupType>(&T)) 
    llvm_unreachable("Illegal basic type!");

  if (const OCLPointerType *P = llvm::dyn_cast<OCLPointerType>(&T)) {
    // Modifiers
    if (P->hasModifier(OCLPointerType::M_Const))
      OS << "const ";

    // Base type
    EmitOCLTypeSignature(OS, P->getBaseType(), false, 0U);
    OS << " ";

    // Address Space
    OS << "__opencrun_as(" << P->getAddressSpace() << ") ";

    // Star
    OS << "*";
  } else {
    // Typen name
    OS << T;
  }

  if (ShowNames)
    OS << " " << "param" << Index;
}

void EmitOCLBuiltinPrototype(llvm::raw_ostream &OS, const OCLBuiltin &B) {
  std::list<BuiltinSignature> Alts;


  OS << "// Builtin: " << B.getName() << "\n\n";

  OS << "#define " << B.getName() << " __builtin_ocl_" << B.getName() << "\n\n";

  for (OCLBuiltin::iterator BI = B.begin(), BE = B.end(); BI != BE; ++BI) {
    const OCLBuiltinVariant &BV = *BI;

    Alts.clear();
    ExpandSignature(BV, Alts);

    for (std::list<BuiltinSignature>::iterator 
         I = Alts.begin(), E = Alts.end(); I != E; ++I) {
      BuiltinSignature &S = *I;
      OS << "__opencrun_overload\n";
      EmitOCLTypeSignature(OS, *S[0], false, 0U);
      OS << " ";
      OS << "__builtin_ocl_" << B.getName();
      OS << "(";
      for (unsigned i = 1, e = S.size(); i != e; ++i) {
        EmitOCLTypeSignature(OS, *S[i], true, i);
        if (i + 1 != e) OS << ", ";
      }
      OS << ");\n\n";
    }
  }
}

bool opencrun::EmitOCLBuiltinDef(llvm::raw_ostream &OS,
                                 llvm::RecordKeeper &R) {
  LoadOCLTypes(R, OCLTypes);
  LoadOCLBuiltins(R, OCLBuiltins);
 
  emitSourceFileHeader("OCL Builtin definitions", OS);

  OS << "#ifndef OPENCRUN_OCLDEF_H\n";
  OS << "#define OPENCRUN_OCLDEF_H\n\n";

  OS << "#define __opencrun_overload __attribute__((overloadable))\n";
  OS << "#define __opencrun_as(n) __attribute__(( address_space(n) ))\n\n";

  for (unsigned i = 0, e = OCLBuiltins.size(); i != e; ++i) {
    EmitOCLBuiltinPrototype(OS, *OCLBuiltins[i]);
  }

  OS << "#ifndef OPENCRUN_LIB_IMPL\n";
  OS << "#undef __opencrun_as(n)\n";
  OS << "#undef __opencrun_overload\n";
  OS << "#endif\n";

  OS << "\n#endif\n";
  return false;
}
