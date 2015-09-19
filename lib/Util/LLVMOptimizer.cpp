#include "opencrun/Util/LLVMOptimizer.h"
#include "opencrun/Core/Device.h"

#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/IPO.h"

using namespace opencrun;

using namespace clang;
using namespace llvm;

static TargetLibraryInfoImpl *createTLII(Triple &TargetTriple,
                                         const CodeGenOptions &CGOpts) {
  auto *TLII = new TargetLibraryInfoImpl(TargetTriple);
  if (!CGOpts.SimplifyLibCalls)
    TLII->disableAllFunctions();

  switch (CGOpts.getVecLib()) {
  case clang::CodeGenOptions::Accelerate:
    TLII->addVectorizableFunctionsFromVecLib(TargetLibraryInfoImpl::Accelerate);
    break;
  default:
    break;
  }
  return TLII;
}

LLVMOptimizer::LLVMOptimizer(const clang::LangOptions &LangOpts,
                             const clang::CodeGenOptions &CodeGenOpts,
                             const clang::TargetOptions &TargetOpts,
                             Device &Dev)
 : Params(LangOpts, CodeGenOpts, TargetOpts), Dev(Dev) {
  Dev.addOptimizerExtensions(PMBuilder, Params);

  if (!CodeGenOpts.DisableLLVMPasses) {
    auto Inlining = CodeGenOpts.getInlining();
    unsigned OptLevel = CodeGenOpts.OptimizationLevel;

    // Handle disabling of LLVM optimization, where we want to preserve the
    // internal module before any optimization.
    if (CodeGenOpts.DisableLLVMOpts) {
      OptLevel = 0;
      Inlining = CodeGenOpts.NoInlining;
    }

    PMBuilder.OptLevel = OptLevel;
    PMBuilder.SizeLevel = CodeGenOpts.OptimizeSize;
    PMBuilder.BBVectorize = CodeGenOpts.VectorizeBB;
    PMBuilder.SLPVectorize = CodeGenOpts.VectorizeSLP;
    PMBuilder.LoopVectorize = CodeGenOpts.VectorizeLoop;

    PMBuilder.DisableUnitAtATime = !CodeGenOpts.UnitAtATime;
    PMBuilder.DisableUnrollLoops = !CodeGenOpts.UnrollLoops;
    PMBuilder.MergeFunctions = CodeGenOpts.MergeFunctions;
    PMBuilder.PrepareForLTO = CodeGenOpts.PrepareForLTO;
    PMBuilder.RerollLoops = CodeGenOpts.RerollLoops;

    // Figure out TargetLibraryInfo.
    Triple TargetTriple(Params.TargetOpts.Triple);
    PMBuilder.LibraryInfo = createTLII(TargetTriple, CodeGenOpts);

    switch (Inlining) {
    case clang::CodeGenOptions::NoInlining: break;
    case clang::CodeGenOptions::NormalInlining: {
      PMBuilder.Inliner =
          createFunctionInliningPass(OptLevel, CodeGenOpts.OptimizeSize);
      break;
    }
    case clang::CodeGenOptions::OnlyAlwaysInlining:
      // Respect always_inline.
      if (OptLevel == 0)
        // Do not insert lifetime intrinsics at -O0.
        PMBuilder.Inliner = createAlwaysInlinerPass(false);
      else
        PMBuilder.Inliner = createAlwaysInlinerPass();
      break;
    }
  }
}

static void setupTargetMachine(llvm::TargetMachine *TM,
                               const LLVMOptimizerParams &P) {
  const clang::CodeGenOptions &CodeGenOpts = P.CodeGenOpts;
  const clang::TargetOptions &TargetOpts = P.TargetOpts;

  CodeGenOpt::Level OptLevel = CodeGenOpt::Default;
  switch (CodeGenOpts.OptimizationLevel) {
  default: break;
  case 0: OptLevel = CodeGenOpt::None; break;
  case 1: OptLevel = CodeGenOpt::Less; break;
  case 3: OptLevel = CodeGenOpt::Aggressive; break;
  }

  TM->setOptLevel(OptLevel);

  if (!TargetOpts.Reciprocals.empty())
    TM->Options.Reciprocals = TargetRecip(TargetOpts.Reciprocals);
  else
    TM->Options.Reciprocals = TargetRecip();

  // Set FP fusion mode.
  switch (CodeGenOpts.getFPContractMode()) {
  case CodeGenOptions::FPC_Off:
    TM->Options.AllowFPOpFusion = FPOpFusion::Strict;
    break;
  case CodeGenOptions::FPC_On:
    TM->Options.AllowFPOpFusion = FPOpFusion::Standard;
    break;
  case CodeGenOptions::FPC_Fast:
    TM->Options.AllowFPOpFusion = FPOpFusion::Fast;
    break;
  }

  TM->Options.LessPreciseFPMADOption = CodeGenOpts.LessPreciseFPMAD;
  TM->Options.NoInfsFPMath = CodeGenOpts.NoInfsFPMath;
  TM->Options.NoNaNsFPMath = CodeGenOpts.NoNaNsFPMath;
  TM->Options.UnsafeFPMath = CodeGenOpts.UnsafeFPMath;
}

static Pass *createTTI(TargetMachine *TM) {
  assert(TM);
  return createTargetTransformInfoWrapperPass(TM->getTargetIRAnalysis());
}

void LLVMOptimizer::run(Module *M) {
  if (!M)
    return;

  auto *TM = Dev.getTargetMachine();
  setupTargetMachine(TM, Params);

  // Set up the per-function pass manager.
  legacy::FunctionPassManager *FPM = new legacy::FunctionPassManager(M);
  FPM->add(createTTI(TM));
  PMBuilder.populateFunctionPassManager(*FPM);

  // Set up the per-module pass manager.
  legacy::PassManager *MPM = new legacy::PassManager();
  MPM->add(createTTI(TM));
  PMBuilder.populateModulePassManager(*MPM);

  // Run per-function passes.
  FPM->doInitialization();
  for (Function &F : *M)
    if (!F.isDeclaration())
      FPM->run(F);
  FPM->doFinalization();

  // Run per-module passes.
  MPM->run(*M);
}
