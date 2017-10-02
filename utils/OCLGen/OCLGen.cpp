#include "OCLEmitter.h"
#include "OCLGen.h"

#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/TableGen/Main.h"


using namespace opencrun;

enum ActionType {
  GenOCLConstants,
  GenOCLConstantsTarget,
  GenOCLTypeDefs,
  GenOCLTypeDefsTarget,
  GenOCLBuiltinDefs,
  GenOCLBuiltinImpls,
  GenOCLTargetDefs
};

llvm::cl::opt<ActionType>
Action(llvm::cl::desc("Action to perform"),
       llvm::cl::values(
                        clEnumValN(GenOCLConstants, 
                                   "gen-ocl-constants", 
                                   "Generate 'oclconst.h' header file"),
                        clEnumValN(GenOCLConstantsTarget, 
                                   "gen-ocl-constants-target", 
                                   "Generate 'oclconst.TARGET.h' header file"),
                        clEnumValN(GenOCLTypeDefs, 
                                   "gen-ocl-types", 
                                   "Generate 'ocltype.h' header file"),
                        clEnumValN(GenOCLTypeDefsTarget, 
                                   "gen-ocl-types-target", 
                                   "Generate 'ocltype.TARGET.h' header file"),
                        clEnumValN(GenOCLBuiltinDefs,
                                   "gen-ocl-builtin-defs",
                                   "Generate 'oclbuiltin.h' header file"),
                        clEnumValN(GenOCLTargetDefs,
                                   "gen-ocl-target-defs",
                                   "Generate 'ocldef.TARGET.h' header file"),
                        clEnumValN(GenOCLBuiltinImpls,
                                   "gen-ocl-builtin-impls",
                                   "Generate OpenCL builtins implementation")));

llvm::cl::opt<std::string> TargetName("target",
                                      llvm::cl::desc("Reference target name"), 
                                      llvm::cl::init(""));

static bool OCLGenMain(llvm::raw_ostream &OS, llvm::RecordKeeper &R) {
  switch (Action) {
  default: break;
  case GenOCLConstants:
    return EmitOCLConstantDefs(OS, R);
  case GenOCLConstantsTarget:
    return EmitOCLConstantDefsTarget(OS, R);
  case GenOCLTypeDefs:
    return EmitOCLTypeDefs(OS, R);
  case GenOCLTypeDefsTarget:
    return EmitOCLTypeDefsTarget(OS, R);
  case GenOCLBuiltinDefs:
    return EmitOCLBuiltinDefs(OS, R);
  case GenOCLTargetDefs:
    return EmitOCLTargetDefs(OS, R);
  case GenOCLBuiltinImpls:
    return EmitOCLBuiltinImpls(OS, R);
  }
  return false;
}

int main(int argc, char *argv[]) {
  llvm::PrettyStackTraceProgram X(argc, argv);

  llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  return llvm::TableGenMain(argv[0], &OCLGenMain);
}
