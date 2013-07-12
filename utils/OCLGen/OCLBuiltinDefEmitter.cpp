#include "OCLBuiltin.h"
#include "OCLEmitter.h"
#include "OCLEmitterUtils.h"
#include "OCLGen.h"
#include "OCLType.h"

#include "llvm/ADT/Twine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace opencrun;


namespace {

OCLTypesContainer OCLTypes;
OCLBuiltinsContainer OCLBuiltins;

}

void EmitOCLBuiltinPrototype(llvm::raw_ostream &OS, const OCLBuiltin &B) {
  BuiltinSignatureList Alts;

  OS << "// Builtin: " << B.getName() << "\n\n";

  OS << "#define " << B.getName() << " __builtin_ocl_" << B.getName() << "\n\n";

  for (OCLBuiltin::iterator BI = B.begin(), BE = B.end(); BI != BE; ++BI) {
    const OCLBuiltinVariant &BV = *BI->second;

    ExpandSignature(BV, Alts);
  }

  Alts.sort(BuiltinSignatureCompare());

  PredicateSet GroupPreds;

  for (BuiltinSignatureList::iterator I = Alts.begin(), 
       E = Alts.end(); I != E; ++I) {
    BuiltinSignature &S = *I;

    PredicateSet Preds;
    ComputePredicates(S, Preds, true);
    if (Preds != GroupPreds) {
      EmitPredicatesEnd(OS, GroupPreds);
      GroupPreds = Preds;
      EmitPredicatesBegin(OS, GroupPreds);
    }
    
    if (Alts.size() > 1)
      OS << "__opencrun_overload\n";

    EmitOCLTypeSignature(OS, *S[0]);
    OS << " ";
    OS << "__builtin_ocl_" << B.getName();
    OS << "(";
    std::string ParamName = "param";
    for (unsigned i = 1, e = S.size(); i != e; ++i) {
      EmitOCLTypeSignature(OS, *S[i], ParamName + llvm::Twine(i).str());
      if (i + 1 != e) OS << ", ";
    }
    OS << ");\n\n";
  }
  EmitPredicatesEnd(OS, GroupPreds);
  OS << "\n";
}

bool opencrun::EmitOCLBuiltinDefs(llvm::raw_ostream &OS,
                                  llvm::RecordKeeper &R) {
  LoadOCLTypes(R, OCLTypes);
  LoadOCLBuiltins(R, OCLBuiltins);
 
  emitSourceFileHeader("OCL Builtin definitions", OS);

  OS << "#ifndef OPENCRUN_OCLBUILTIN_H\n";
  OS << "#define OPENCRUN_OCLBUILTIN_H\n\n";

  OS << "#define __opencrun_overload __attribute__((overloadable))\n\n";

  for (unsigned i = 0, e = OCLBuiltins.size(); i != e; ++i) {
    EmitOCLBuiltinPrototype(OS, *OCLBuiltins[i]);
  }

  OS << "#ifndef OPENCRUN_LIB_IMPL\n";
  OS << "#undef __opencrun_overload\n";
  OS << "#endif\n";

  OS << "\n#endif\n";
  return false;
}
