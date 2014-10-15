#include "opencrun/Util/LLVMOptimizer.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/IPO.h"

using namespace opencrun;

LLVMOptimizerBase::LLVMOptimizerBase(const clang::LangOptions &langopts,
                                     const clang::CodeGenOptions &cgopts,
                                     const clang::TargetOptions &targetopts)
 : Params(langopts, cgopts, targetopts) {
}

void LLVMOptimizerBase::init() {
  // Setup code copied from clang/lib/CodeGen/BackendUtil.cpp
  const clang::CodeGenOptions &CGOpts = Params.CodeGenOpts;
  unsigned OptLevel = CGOpts.OptimizationLevel;
  clang::CodeGenOptions::InliningMethod Inlining = CGOpts.getInlining();

  if (CGOpts.DisableLLVMOpts) {
    OptLevel = 0;
    Inlining = CGOpts.NoInlining;
  }

  PMBuilder.OptLevel = OptLevel;
  PMBuilder.SizeLevel = CGOpts.OptimizeSize;
  PMBuilder.BBVectorize = CGOpts.VectorizeBB;
  PMBuilder.SLPVectorize = CGOpts.VectorizeSLP;
  PMBuilder.LoopVectorize = CGOpts.VectorizeLoop;

  PMBuilder.DisableUnitAtATime = !CGOpts.UnitAtATime;
  PMBuilder.DisableUnrollLoops = !CGOpts.UnrollLoops;
  PMBuilder.RerollLoops = CGOpts.RerollLoops;

  llvm::Triple Triple(Params.TargetOpts.Triple);

  PMBuilder.LibraryInfo = new llvm::TargetLibraryInfo(Triple);
  if (!CGOpts.SimplifyLibCalls)
    PMBuilder.LibraryInfo->disableAllFunctions();

  switch (Inlining) {
  case clang::CodeGenOptions::NoInlining: break;
  case clang::CodeGenOptions::NormalInlining: {
    unsigned Threshold = 225;
    if (CGOpts.OptimizeSize == 1) Threshold = 75;
    else if (CGOpts.OptimizeSize == 2) Threshold = 25;
    else if (OptLevel > 2) Threshold = 275;
    PMBuilder.Inliner = llvm::createFunctionInliningPass(Threshold);
    break;
  }
  case clang::CodeGenOptions::OnlyAlwaysInlining:
    if (OptLevel == 0)
      PMBuilder.Inliner = llvm::createAlwaysInlinerPass(false);
    else
      PMBuilder.Inliner = llvm::createAlwaysInlinerPass();
    break;
  }
}

static llvm::TargetMachine *createTargetMachine(LLVMOptimizerParams &P) {
  std::string Error;
  std::string Triple(P.TargetOpts.Triple);
  const llvm::Target *TheTarget =
    llvm::TargetRegistry::lookupTarget(Triple, Error);

  if (!TheTarget) return 0;

  unsigned CodeModel =
    llvm::StringSwitch<unsigned>(P.CodeGenOpts.CodeModel)
      .Case("small", llvm::CodeModel::Small)
      .Case("kernel", llvm::CodeModel::Kernel)
      .Case("medium", llvm::CodeModel::Medium)
      .Case("large", llvm::CodeModel::Large)
      .Case("default", llvm::CodeModel::Default)
      .Default(~0u);
  assert(CodeModel != ~0u && "invalid code model!");
  llvm::CodeModel::Model CM = static_cast<llvm::CodeModel::Model>(CodeModel);

  std::string FeaturesStr;
  if (P.TargetOpts.Features.size()) {
    llvm::SubtargetFeatures Features;
    for (std::vector<std::string>::const_iterator
           it = P.TargetOpts.Features.begin(),
           ie = P.TargetOpts.Features.end(); it != ie; ++it)
      Features.AddFeature(*it);
    FeaturesStr = Features.getString();
  }

  llvm::Reloc::Model RM = llvm::Reloc::Default;
  if (P.CodeGenOpts.RelocationModel == "static") {
    RM = llvm::Reloc::Static;
  } else if (P.CodeGenOpts.RelocationModel == "pic") {
    RM = llvm::Reloc::PIC_;
  } else {
    assert(P.CodeGenOpts.RelocationModel == "dynamic-no-pic" &&
           "Invalid PIC model!");
    RM = llvm::Reloc::DynamicNoPIC;
  }

  llvm::CodeGenOpt::Level OptLevel = llvm::CodeGenOpt::Default;
  switch (P.CodeGenOpts.OptimizationLevel) {
  default: break;
  case 0: OptLevel = llvm::CodeGenOpt::None; break;
  case 1: OptLevel = llvm::CodeGenOpt::Less; break;
  case 3: OptLevel = llvm::CodeGenOpt::Aggressive; break;
  }

  llvm::TargetOptions Options;

  // Set frame pointer elimination mode.
  if (!P.CodeGenOpts.DisableFPElim) {
    Options.NoFramePointerElim = false;
  } else if (P.CodeGenOpts.OmitLeafFramePointer) {
    Options.NoFramePointerElim = false;
  } else {
    Options.NoFramePointerElim = true;
  }

  if (P.CodeGenOpts.UseInitArray)
    Options.UseInitArray = true;

  // Set float ABI type.
  if (P.CodeGenOpts.FloatABI == "soft" || P.CodeGenOpts.FloatABI == "softfp")
    Options.FloatABIType = llvm::FloatABI::Soft;
  else if (P.CodeGenOpts.FloatABI == "hard")
    Options.FloatABIType = llvm::FloatABI::Hard;
  else {
    assert(P.CodeGenOpts.FloatABI.empty() && "Invalid float abi!");
    Options.FloatABIType = llvm::FloatABI::Default;
  }

  // Set FP fusion mode.
  switch (P.CodeGenOpts.getFPContractMode()) {
  case clang::CodeGenOptions::FPC_Off:
    Options.AllowFPOpFusion = llvm::FPOpFusion::Strict;
    break;
  case clang::CodeGenOptions::FPC_On:
    Options.AllowFPOpFusion = llvm::FPOpFusion::Standard;
    break;
  case clang::CodeGenOptions::FPC_Fast:
    Options.AllowFPOpFusion = llvm::FPOpFusion::Fast;
    break;
  }

  Options.LessPreciseFPMADOption = P.CodeGenOpts.LessPreciseFPMAD;
  Options.NoInfsFPMath = P.CodeGenOpts.NoInfsFPMath;
  Options.NoNaNsFPMath = P.CodeGenOpts.NoNaNsFPMath;
  Options.NoZerosInBSS = P.CodeGenOpts.NoZeroInitializedInBSS;
  Options.UnsafeFPMath = P.CodeGenOpts.UnsafeFPMath;
  Options.UseSoftFloat = P.CodeGenOpts.SoftFloat;
  Options.StackAlignmentOverride = P.CodeGenOpts.StackAlignment;
  Options.DisableTailCalls = P.CodeGenOpts.DisableTailCalls;
  Options.TrapFuncName = P.CodeGenOpts.TrapFuncName;
  Options.PositionIndependentExecutable = P.LangOpts.PIELevel != 0;
  Options.FunctionSections = P.CodeGenOpts.FunctionSections;
  Options.DataSections = P.CodeGenOpts.DataSections;

  Options.MCOptions.MCRelaxAll = P.CodeGenOpts.RelaxAll;
  Options.MCOptions.MCSaveTempLabels = P.CodeGenOpts.SaveTempLabels;
  Options.MCOptions.MCUseDwarfDirectory = !P.CodeGenOpts.NoDwarfDirectoryAsm;
  Options.MCOptions.MCNoExecStack = P.CodeGenOpts.NoExecStack;
  Options.MCOptions.AsmVerbose = P.CodeGenOpts.AsmVerbose;

  llvm::TargetMachine *TM =
    TheTarget->createTargetMachine(Triple, P.TargetOpts.CPU, FeaturesStr,
                                   Options, RM, CM, OptLevel);

  return TM;
}

void LLVMOptimizerBase::run(llvm::Module *M) {
  if (!M) return;

  llvm::TargetMachine *TM = createTargetMachine(Params);

  FPM.reset(new llvm::FunctionPassManager(M));
  FPM->add(new llvm::DataLayoutPass(M));
  if (TM) TM->addAnalysisPasses(*FPM);
  PMBuilder.populateFunctionPassManager(*FPM);

  MPM.reset(new llvm::PassManager());
  MPM->add(new llvm::DataLayoutPass(M));
  if (TM) TM->addAnalysisPasses(*MPM);
  PMBuilder.populateModulePassManager(*MPM);

  if (FPM) {
    FPM->doInitialization();
    for (llvm::Module::iterator I = M->begin(), E = M->end(); I != E; ++I)
      if (!I->isDeclaration())
        FPM->run(*I);
    FPM->doFinalization();
  }
  if (MPM)
    MPM->run(*M);
}
