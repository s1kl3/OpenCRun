#include "OCLBuiltinDefEmitter.h"
#include "OCLEmitterUtils.h"
#include "OCLBuiltin.h"
#include "OCLType.h"

#include "llvm/ADT/Twine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace opencrun;


namespace {

OCLTypesContainer OCLTypes;
OCLBuiltinsContainer OCLBuiltins;

}

void EmitOCLBuiltinPrototype(llvm::raw_ostream &OS, const OCLBuiltin &B) {
  std::list<BuiltinSignature> Alts;


  OS << "// Builtin: " << B.getName() << "\n\n";

  OS << "#define " << B.getName() << " __builtin_ocl_" << B.getName() << "\n\n";

  for (OCLBuiltin::iterator BI = B.begin(), BE = B.end(); BI != BE; ++BI) {
    const OCLBuiltinVariant &BV = *BI;

    ExpandSignature(BV, Alts);
  }

  SortBuiltinSignatureList(Alts);

  llvm::BitVector GroupReq;

  for (std::list<BuiltinSignature>::iterator  
       I = Alts.begin(), E = Alts.end(); I != E; ++I) {
    BuiltinSignature &S = *I;

    llvm::BitVector Req;
    ComputeRequiredExt(S, Req);
    if (Req != GroupReq) {
      EmitRequiredExtEnd(OS, GroupReq);
      GroupReq = Req;
      EmitRequiredExtBegin(OS, GroupReq);
      OS << "\n";
    }

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
  EmitRequiredExtEnd(OS, GroupReq);
  OS << "\n";
}

bool opencrun::EmitOCLBuiltinDef(llvm::raw_ostream &OS,
                                 llvm::RecordKeeper &R) {
  LoadOCLTypes(R, OCLTypes);
  LoadOCLBuiltins(R, OCLBuiltins);
 
  emitSourceFileHeader("OCL Builtin definitions", OS);

  OS << "#ifndef OPENCRUN_OCLDEF_H\n";
  OS << "#define OPENCRUN_OCLDEF_H\n\n";

  OS << "#define __opencrun_overload __attribute__((overloadable))\n\n";
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
