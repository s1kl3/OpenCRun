#include "OCLEmitter.h"
#include "OCLEmitterUtils.h"
#include "OCLType.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Casting.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace opencrun;

namespace {

OCLTypesContainer OCLTypes;
OCLOpaqueTypeDefsContainer OCLOpaqueTypeDefs;

}

const char *BasicIntegerTy(unsigned BitWidth) {
  // Here we assume that the frontend enforce correct OpenCL bitwidth to 
  // builtin types.
  switch(BitWidth) {
    default: break;
    case 8: return "char";
    case 16: return "short";
    case 32: return "int";
    case 64: return "long";
  }
  llvm::PrintFatalError("Unsupported integer type!");
}

void EmitOCLScalarTypes(llvm::raw_ostream &OS) {
  for (unsigned i = 0, e = OCLTypes.size(); i != e; ++i) {
    if (const OCLIntegerType *I = llvm::dyn_cast<OCLIntegerType>(OCLTypes[i])) {
      if (I->isUnsigned()) {
        OS << "typedef unsigned " << BasicIntegerTy(I->getBitWidth()) 
           << " " << I->getName() << ";\n";
      }
    }
  }
}

void EmitOCLVectorTypes(llvm::raw_ostream &OS) {
  PredicateSet GroupPreds;
  for (unsigned i = 0, e = OCLTypes.size(); i != e; ++i) {
    if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(OCLTypes[i])) {
      const PredicateSet &Preds = V->getBaseType().getPredicates();
      if (Preds != GroupPreds) {
        EmitPredicatesEnd(OS, GroupPreds);
        OS << "\n";
        GroupPreds = Preds;
        EmitPredicatesBegin(OS, GroupPreds); 
      }

      OS << "typedef " << V->getBaseType().getName()
         << " __attribute__((ext_vector_type(" << V->getWidth() << "))) "
         << V->getName() << ";\n";
    }
  }
  EmitPredicatesEnd(OS, GroupPreds);
}

void EmitOCLOpaqueTypeDefs(llvm::raw_ostream &OS, bool TargetDefs) {
  for (unsigned i = 0, e = OCLOpaqueTypeDefs.size(); i != e; ++i) {
    const OCLOpaqueTypeDef &D = *OCLOpaqueTypeDefs[i];
    if (D.isTarget() != TargetDefs) continue;

    EmitPredicatesBegin(OS, D.getPredicates());
    OS << "typedef " << D.getDef() << " " << D.getType() << ";\n";
    EmitPredicatesEnd(OS, D.getPredicates());
  }
}

bool opencrun::EmitOCLTypeDefs(llvm::raw_ostream &OS, 
                               llvm::RecordKeeper &R) {
  LoadOCLTypes(R, OCLTypes);
  LoadOCLOpaqueTypeDefs(R, OCLOpaqueTypeDefs);

  emitSourceFileHeader("OCL Type definitions", OS);

  OS << "#ifndef OPENCRUN_OCLTYPE_H\n";
  OS << "#define OPENCRUN_OCLTYPE_H\n\n";

  OS << "#include <stddef.h>\n\n";
  
  OS << "// Scalar types\n";
  EmitOCLScalarTypes(OS);
  OS << "\n";

  OS << "// Vector types\n";
  EmitOCLVectorTypes(OS);
  OS << "\n";

  OS << "// Opaque types\n";
  EmitOCLOpaqueTypeDefs(OS, false);

  OS << "\n#endif\n";
  
  return false;
}

bool opencrun::EmitOCLTypeDefsTarget(llvm::raw_ostream &OS, 
                                     llvm::RecordKeeper &R) {
  LoadOCLTypes(R, OCLTypes);
  LoadOCLOpaqueTypeDefs(R, OCLOpaqueTypeDefs);

  emitSourceFileHeader("OCL Target type definitions", OS);

  OS << "#ifndef OPENCRUN_OCLTYPE_TARGET_H\n";
  OS << "#define OPENCRUN_OCLTYPE_TARGET_H\n\n";

  OS << "// Target opaque types\n";
  EmitOCLOpaqueTypeDefs(OS, true);

  OS << "\n#endif\n";

  return false;
}
