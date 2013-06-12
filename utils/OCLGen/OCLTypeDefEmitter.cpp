#include "OCLTypeDefEmitter.h"
#include "OCLEmitterUtils.h"
#include "OCLType.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Casting.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace opencrun;

namespace {

OCLTypesContainer OCLTypes;

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

void EmitOCLTypeDef(llvm::raw_ostream &OS, const OCLScalarType *S) {

}

void EmitOCLTypeDef(llvm::raw_ostream &OS, const OCLVectorType *V) {
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
  llvm::BitVector GroupReq;
  for (unsigned i = 0, e = OCLTypes.size(); i != e; ++i) {
    if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(OCLTypes[i])) {
      const llvm::BitVector &Req = V->getBaseType().getRequiredTypeExt();
      if (Req != GroupReq) {
        EmitRequiredExtEnd(OS, GroupReq);
        OS << "\n";
        GroupReq = Req;
        EmitRequiredExtBegin(OS, GroupReq); 
      }

      OS << "typedef " << V->getBaseType().getName()
         << " __attribute__((ext_vector_type(" << V->getWidth() << "))) "
         << V->getName() << ";\n";
    }
  }
  EmitRequiredExtEnd(OS, GroupReq);
}

bool opencrun::EmitOCLTypeDef(llvm::raw_ostream &OS, 
                              llvm::RecordKeeper &R) {
  LoadOCLTypes(R, OCLTypes);

  emitSourceFileHeader("OCL Type definitions", OS);

  OS << "#ifndef OPENCRUN_OCLTYPE_H\n";
  OS << "#define OPENCRUN_OCLTYPE_H\n\n";

  OS << "#include <stddef.h>\n\n";

  OS << "// Scalar types\n";
  EmitOCLScalarTypes(OS);
  OS << "\n";

  OS <<"// Vector types\n";
  EmitOCLVectorTypes(OS);

  OS << "\n#endif\n";
  
  return false;
}
