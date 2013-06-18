#include "OCLTypeDefEmitter.h"
#include "OCLBuiltinDefEmitter.h"
#include "OCLBuiltinImplEmitter.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/TableGen/Main.h"

using namespace opencrun;

enum ActionType {
  GenOCLTypeDef,
  GenOCLBuiltinDef,
  GenOCLBuiltinDefTarget,
  GenOCLBuiltinImpl
};

llvm::cl::opt<ActionType>
Action(llvm::cl::desc("Action to perform"),
       llvm::cl::values(clEnumValN(GenOCLTypeDef, 
                                   "gen-ocl-types", 
                                   "Generate 'ocltype.h' header file"),
                        clEnumValN(GenOCLBuiltinDef,
                                   "gen-ocl-lib-def",
                                   "Generate 'ocldef.h' header file"),
                        clEnumValN(GenOCLBuiltinDefTarget,
                                   "gen-ocl-lib-def-target",
                                   "Generate 'ocldef.TARGET.h' header file"),
                        clEnumValN(GenOCLBuiltinImpl,
                                   "gen-ocl-lib-impl",
                                   "Generate OpenCL C library implementation"),
                        clEnumValEnd));

static bool OCLGenMain(llvm::raw_ostream &OS, llvm::RecordKeeper &R) {
  switch (Action) {
  case GenOCLTypeDef:
    return EmitOCLTypeDef(OS, R);
  case GenOCLBuiltinDef:
    return EmitOCLBuiltinDef(OS, R);
  case GenOCLBuiltinDefTarget:
    return EmitOCLBuiltinDefTarget(OS, R);
  case GenOCLBuiltinImpl:
    return EmitOCLBuiltinImpl(OS, R);
  }
  return false;
}

int main(int argc, char *argv[]) {
  llvm::PrettyStackTraceProgram X(argc, argv);

  llvm::sys::PrintStackTraceOnErrorSignal();
  llvm::cl::ParseCommandLineOptions(argc, argv);

  return llvm::TableGenMain(argv[0], OCLGenMain);
}
