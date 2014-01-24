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

void EmitOCLGenericBuiltinPrototype(llvm::raw_ostream &OS, 
                                    const OCLGenericBuiltin &B) {
  OCLBuiltinDecorator BD(B);

  OS << "// Builtin: " << BD.getExternalName() << "\n\n";

  OS << "#define " << BD.getExternalName() << " " 
     << BD.getInternalName() << "\n\n";

  BuiltinSignList Alts;
  for (OCLBuiltin::var_iterator BI = B.begin(), BE = B.end(); BI != BE; ++BI) {
    const OCLBuiltinVariant &BV = *BI->second;

    ExpandSigns(BV, Alts);
  }

  Alts.sort(BuiltinSignCompare());

  PredicatesGuardEmitter Preds(OS);

  for (BuiltinSignList::const_iterator I = Alts.begin(), 
       E = Alts.end(); I != E; ++I) {
    const BuiltinSign &S = *I;

    Preds.Push(ComputePredicates(S, true));
    
    if (Alts.size() > 1)
      OS << "__opencrun_overload\n";

    EmitOCLTypeSignature(OS, *S[0]);
    OS << " ";
    OS << BD.getInternalName();
    OS << "(";
    std::string ParamName = "param";
    for (unsigned i = 1, e = S.size(); i != e; ++i) {
      EmitOCLTypeSignature(OS, *S[i], ParamName + llvm::Twine(i).str());
      if (i + 1 != e) OS << ", ";
    }
    OS << ");\n\n";
  }
  Preds.Finalize();
  OS << "\n";
}

void EmitOCLCastBuiltinPrototype(llvm::raw_ostream &OS, 
                                 const OCLCastBuiltin &B) {
  OCLBuiltinDecorator BD(B);

  BuiltinSignList Alts;
  for (OCLBuiltin::var_iterator BI = B.begin(), BE = B.end(); BI != BE; ++BI) {
    const OCLBuiltinVariant &BV = *BI->second;

    ExpandSigns(BV, Alts);
  }

  Alts.sort(BuiltinSignCompare());

  std::list<std::pair<unsigned, BuiltinSignList::iterator> > Ranges;
  ComputeSignsRanges(Alts.begin(), Alts.end(), Ranges);

  PredicatesGuardEmitter Preds(OS);

  for (BuiltinSignList::iterator I = Alts.begin(),
       E = Alts.end(); I != E; ++I) {
    const BuiltinSign &S = *I;

    if (Ranges.front().second == I) {
      Preds.Finalize();
      OS << "\n";

      Ranges.pop_front();

      if (const OCLConvertBuiltin *CB = llvm::dyn_cast<OCLConvertBuiltin>(&B))
        if (CB->getRoundingMode().isDefaultFor(*S[0]))
          OS << "#define " << BD.getExternalName(&S, true) << " "
             << BD.getInternalName(&S) << "\n";

      OS << "#define " << BD.getExternalName(&S) << " "
         << BD.getInternalName(&S) << "\n\n";
    }
 
    Preds.Push(ComputePredicates(S, true));

    if (Ranges.front().first > 1)
      OS << "__opencrun_overload\n";

    EmitOCLTypeSignature(OS, *S[0]);
    OS << " ";
    OS << BD.getInternalName(&S);
    OS << "(";
    std::string ParamName = "param";
    for (unsigned i = 1, e = S.size(); i != e; ++i) {
      EmitOCLTypeSignature(OS, *S[i], ParamName + llvm::Twine(i).str());
      if (i + 1 != e) OS << ", ";
    }
    OS << ");\n\n";
  }
  Preds.Finalize();
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
    const OCLBuiltin *B = OCLBuiltins[i];
    if (llvm::isa<OCLGenericBuiltin>(B))
      EmitOCLGenericBuiltinPrototype(OS, *llvm::cast<OCLGenericBuiltin>(B));
    else if (llvm::isa<OCLCastBuiltin>(B))
      EmitOCLCastBuiltinPrototype(OS, *llvm::cast<OCLCastBuiltin>(B));
    else
      llvm_unreachable(0);
  }

  OS << "#ifndef OPENCRUN_LIB_IMPL\n";
  OS << "#undef __opencrun_overload\n";
  OS << "#endif\n";

  OS << "\n#endif\n";
  return false;
}
