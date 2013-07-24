#include "OCLEmitter.h"
#include "OCLConstant.h"

#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace opencrun;


namespace {
  OCLConstantsContainer OCLConsts;
}

static void EmitOCLConstants(llvm::raw_ostream &OS, bool TargetConst) {

  llvm::StringRef GlobalGroup = "";

  for (unsigned i = 0, e = OCLConsts.size(); i != e; ++i) {
    const OCLConstant &C = *OCLConsts[i];

    if (C.isTarget() != TargetConst) continue;

    llvm::StringRef Group = C.getGroup();
    if (Group != GlobalGroup) {
      OS << "\n";
      GlobalGroup = Group;
      OS << "// " << Group << "\n";
    }

    OS << "#define " << C.getName() << " (" << C.getValue() << ")\n";
  }
} 

bool opencrun::EmitOCLConstantDefs(llvm::raw_ostream &OS, 
                                llvm::RecordKeeper &R) {
  LoadOCLConstants(R, OCLConsts);

  emitSourceFileHeader("OCL Constants", OS);

  OS << "#ifndef OPENCRUN_OCLCONST_H\n";
  OS << "#define OPENCRUN_OCLCONST_H\n\n";

  EmitOCLConstants(OS, false);

  OS << "\n#endif\n";

  return false;
}

bool opencrun::EmitOCLConstantDefsTarget(llvm::raw_ostream &OS, 
                                      llvm::RecordKeeper &R) {
  LoadOCLConstants(R, OCLConsts);

  emitSourceFileHeader("OCL Target constants", OS);

  OS << "#ifndef OPENCRUN_OCLCONST_TARGET_H\n";
  OS << "#define OPENCRUN_OCLCONST_TARGET_H\n\n";

  EmitOCLConstants(OS, true);

  OS << "\n#endif\n";

  return false;
}
