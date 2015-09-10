#include "opencrun/Util/LLVMOptimizer.h"
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

LLVMOptimizerBase::LLVMOptimizerBase(const clang::LangOptions &langopts,
                                     const clang::CodeGenOptions &cgopts,
                                     const clang::TargetOptions &targetopts)
 : Params(langopts, cgopts, targetopts) {
}

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

void LLVMOptimizerBase::init() {
  // Setup code copied from clang/lib/CodeGen/BackendUtil.cpp
  const clang::CodeGenOptions &CodeGenOpts = Params.CodeGenOpts;

  if (CodeGenOpts.DisableLLVMPasses)
    return;

  unsigned OptLevel = CodeGenOpts.OptimizationLevel;
  clang::CodeGenOptions::InliningMethod Inlining = CodeGenOpts.getInlining();

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

static TargetMachine *createTargetMachine(LLVMOptimizerParams &P) {
  const clang::LangOptions &LangOpts = P.LangOpts;
  const clang::CodeGenOptions &CodeGenOpts = P.CodeGenOpts;
  const clang::TargetOptions &TargetOpts = P.TargetOpts;

  // Create the TargetMachine for generating code.
  std::string Error;
  auto *TheTarget = TargetRegistry::lookupTarget(P.TargetOpts.Triple, Error);
  if (!TheTarget)
    return nullptr;

  unsigned CodeModel =
    StringSwitch<unsigned>(CodeGenOpts.CodeModel)
      .Case("small", CodeModel::Small)
      .Case("kernel", CodeModel::Kernel)
      .Case("medium", CodeModel::Medium)
      .Case("large", CodeModel::Large)
      .Case("default", CodeModel::Default)
      .Default(~0u);
  assert(CodeModel != ~0u && "invalid code model!");
  CodeModel::Model CM = static_cast<CodeModel::Model>(CodeModel);

  std::string FeaturesStr;
  if (!TargetOpts.Features.empty()) {
    SubtargetFeatures Features;
    for (const std::string &Feature : TargetOpts.Features)
      Features.AddFeature(Feature);
    FeaturesStr = Features.getString();
  }

  Reloc::Model RM = Reloc::Default;
  if (CodeGenOpts.RelocationModel == "static") {
    RM = Reloc::Static;
  } else if (CodeGenOpts.RelocationModel == "pic") {
    RM = Reloc::PIC_;
  } else {
    assert(CodeGenOpts.RelocationModel == "dynamic-no-pic" &&
           "Invalid PIC model!");
    RM = Reloc::DynamicNoPIC;
  }

  CodeGenOpt::Level OptLevel = CodeGenOpt::Default;
  switch (CodeGenOpts.OptimizationLevel) {
  default: break;
  case 0: OptLevel = CodeGenOpt::None; break;
  case 1: OptLevel = CodeGenOpt::Less; break;
  case 3: OptLevel = CodeGenOpt::Aggressive; break;
  }

  llvm::TargetOptions Options;

  if (!TargetOpts.Reciprocals.empty())
    Options.Reciprocals = TargetRecip(TargetOpts.Reciprocals);

  Options.ThreadModel =
    StringSwitch<ThreadModel::Model>(CodeGenOpts.ThreadModel)
      .Case("posix", ThreadModel::POSIX)
      .Case("single", ThreadModel::Single);

  if (CodeGenOpts.DisableIntegratedAS)
    Options.DisableIntegratedAS = true;

  if (CodeGenOpts.CompressDebugSections)
    Options.CompressDebugSections = true;

  if (CodeGenOpts.UseInitArray)
    Options.UseInitArray = true;

  // Set float ABI type.
  if (CodeGenOpts.FloatABI == "soft" || CodeGenOpts.FloatABI == "softfp")
    Options.FloatABIType = FloatABI::Soft;
  else if (CodeGenOpts.FloatABI == "hard")
    Options.FloatABIType = FloatABI::Hard;
  else {
    assert(CodeGenOpts.FloatABI.empty() && "Invalid float abi!");
    Options.FloatABIType = FloatABI::Default;
  }

  // Set FP fusion mode.
  switch (CodeGenOpts.getFPContractMode()) {
  case CodeGenOptions::FPC_Off:
    Options.AllowFPOpFusion = FPOpFusion::Strict;
    break;
  case CodeGenOptions::FPC_On:
    Options.AllowFPOpFusion = FPOpFusion::Standard;
    break;
  case CodeGenOptions::FPC_Fast:
    Options.AllowFPOpFusion = FPOpFusion::Fast;
    break;
  }

  Options.LessPreciseFPMADOption = CodeGenOpts.LessPreciseFPMAD;
  Options.NoInfsFPMath = CodeGenOpts.NoInfsFPMath;
  Options.NoNaNsFPMath = CodeGenOpts.NoNaNsFPMath;
  Options.NoZerosInBSS = CodeGenOpts.NoZeroInitializedInBSS;
  Options.UnsafeFPMath = CodeGenOpts.UnsafeFPMath;
  Options.StackAlignmentOverride = CodeGenOpts.StackAlignment;
  Options.PositionIndependentExecutable = LangOpts.PIELevel != 0;
  Options.FunctionSections = CodeGenOpts.FunctionSections;
  Options.DataSections = CodeGenOpts.DataSections;
  Options.UniqueSectionNames = CodeGenOpts.UniqueSectionNames;
  Options.EmulatedTLS = CodeGenOpts.EmulatedTLS;

  Options.MCOptions.MCRelaxAll = CodeGenOpts.RelaxAll;
  Options.MCOptions.MCSaveTempLabels = CodeGenOpts.SaveTempLabels;
  Options.MCOptions.MCUseDwarfDirectory = !CodeGenOpts.NoDwarfDirectoryAsm;
  Options.MCOptions.MCNoExecStack = CodeGenOpts.NoExecStack;
  Options.MCOptions.MCFatalWarnings = CodeGenOpts.FatalWarnings;
  Options.MCOptions.AsmVerbose = CodeGenOpts.AsmVerbose;
  Options.MCOptions.ABIName = TargetOpts.ABI;

  return TheTarget->createTargetMachine(TargetOpts.Triple, TargetOpts.CPU,
                                        FeaturesStr, Options, RM, CM, OptLevel);
}

static Pass *createTTI(TargetMachine *TM) {
  assert(TM);
  return createTargetTransformInfoWrapperPass(TM->getTargetIRAnalysis());
}

void LLVMOptimizerBase::run(Module *M) {
  if (!M)
    return;

  TargetMachine *TM = createTargetMachine(Params);

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
