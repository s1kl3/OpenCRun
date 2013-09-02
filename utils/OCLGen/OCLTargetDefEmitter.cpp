#include "OCLEmitter.h"
#include "OCLEmitterUtils.h"
#include "OCLGen.h"

#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/TableGenBackend.h"

bool opencrun::EmitOCLTargetDefs(llvm::raw_ostream &OS,
                                        llvm::RecordKeeper &R) {
  emitSourceFileHeader("OCL Target definitions", OS);

  OS << "#ifndef OPENCRUN_OCLTARGET_H\n";
  OS << "#define OPENCRUN_OCLTARGET_H\n\n";

  std::vector<llvm::Record *> Features = 
    R.getAllDerivedDefinitions("OCLTargetFeature");

  PredicatesGuardEmitter Requires(OS);

  for (unsigned i = 0, e = Features.size(); i != e; ++i) {
    PredicateSet Preds;

    std::vector<llvm::Record*> Predicates = 
      Features[i]->getValueAsListOfDefs("Predicates");

    for (unsigned j = 0, k = Predicates.size(); j != k; ++j) {
      const OCLPredicate &P = OCLPredicatesTable::get(*Predicates[j]);

      if (!llvm::isa<OCLAddressSpace>(&P))
        Preds.insert(&P);
    }

    Requires.Push(Preds);

    std::vector<llvm::Record*> L =
      Features[i]->getValueAsListOfDefs("Features");

    for (unsigned j = 0, k = L.size(); j != k; ++j) {
      llvm::Record *R = L[j];

      const OCLPredicate &P = OCLPredicatesTable::get(*R);

      if (!llvm::isa<OCLAddressSpace>(&P))
        OS << "#define " << P.getFullName() << "\n";
    }

  }

  Requires.Finalize();

  OS << "\n";
  OS << "#include <ocltype.h>\n";
  OS << "#include <ocltype." << TargetName << ".h>\n";
  OS << "#include <oclconst.h>\n";
  OS << "#include <oclconst." << TargetName << ".h>\n";
  OS << "#include <oclbuiltin.h>\n";

  OS << "\n#endif\n";
  return false;
}
