#include "opencrun/Util/LLVMOptimizer.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "llvm/ADT/SmallVector.h"
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
#ifndef NDEBUG
  llvm::SmallVector<const char *, 16> Args;
  Args.push_back("opencrun");
  Args.push_back("-debug-pass");
  Args.push_back("Structure");
  Args.push_back(0);
  llvm::cl::ParseCommandLineOptions(Args.size() - 1, Args.data());
#endif

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

  llvm::TargetMachine::setAsmVerbosityDefault(P.CodeGenOpts.AsmVerbose);

  llvm::TargetMachine::setFunctionSections(P.CodeGenOpts.FunctionSections);
  llvm::TargetMachine::setDataSections(P.CodeGenOpts.DataSections);

  // FIXME: Parse this earlier.
  llvm::CodeModel::Model CM;
  if (P.CodeGenOpts.CodeModel == "small") {
    CM = llvm::CodeModel::Small;
  } else if (P.CodeGenOpts.CodeModel == "kernel") {
    CM = llvm::CodeModel::Kernel;
  } else if (P.CodeGenOpts.CodeModel == "medium") {
    CM = llvm::CodeModel::Medium;
  } else if (P.CodeGenOpts.CodeModel == "large") {
    CM = llvm::CodeModel::Large;
  } else {
    assert(P.CodeGenOpts.CodeModel.empty() && "Invalid code model!");
    CM = llvm::CodeModel::Default;
  }

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
  Options.EnableSegmentedStacks = P.CodeGenOpts.EnableSegmentedStacks;

  llvm::TargetMachine *TM =
    TheTarget->createTargetMachine(Triple, P.TargetOpts.CPU, FeaturesStr,
                                   Options, RM, CM, OptLevel);

  if (P.CodeGenOpts.RelaxAll)
    TM->setMCRelaxAll(true);
  if (P.CodeGenOpts.SaveTempLabels)
    TM->setMCSaveTempLabels(true);
  if (P.CodeGenOpts.NoDwarf2CFIAsm)
    TM->setMCUseCFI(false);
  if (!P.CodeGenOpts.NoDwarfDirectoryAsm)
    TM->setMCUseDwarfDirectory(true);
  if (P.CodeGenOpts.NoExecStack)
    TM->setMCNoExecStack(true);

  return TM;
}

void LLVMOptimizerBase::run(llvm::Module *M) {
  if (!M) return;

  llvm::TargetMachine *TM = createTargetMachine(Params);

  FPM.reset(new llvm::FunctionPassManager(M));
  FPM->add(new llvm::DataLayout(M));
  if (TM) TM->addAnalysisPasses(*FPM);
  PMBuilder.populateFunctionPassManager(*FPM);

  MPM.reset(new llvm::PassManager());
  MPM->add(new llvm::DataLayout(M));
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
