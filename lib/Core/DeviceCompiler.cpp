#include "opencrun/Core/DeviceCompiler.h"
#include "opencrun/System/Env.h"
#include "opencrun/Util/LLVMCodeGenAction.h"

#include "clang/Basic/Version.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/Path.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <sstream>

using namespace opencrun;

//
// DeviceCompiler implementation
//

DeviceCompiler::DeviceCompiler(llvm::StringRef Name) : Name(Name) {
  // Library name depends on the device name.
  llvm::SmallString<20> LibName("opencrun");
  LibName += Name;
  LibName += "Lib.bc";

  llvm::SmallString<32> Path;
  if (sys::HasEnv("OPENCRUN_PREFIX"))
    llvm::sys::path::append(Path, sys::GetEnv("OPENCRUN_PREFIX"));
  else
    llvm::sys::path::append(Path, LLVM_PREFIX);
  llvm::sys::path::append(Path, "lib", LibName.str());

  if (auto MBOrErr = llvm::MemoryBuffer::getFile(Path.str())) {
    auto MB = std::move(MBOrErr.get());
    if (auto BCOrErr = llvm::parseBitcodeFile(MB->getMemBufferRef(), Ctx))
      Builtins = std::move(BCOrErr.get());
  }

  if (!Builtins)
    llvm::report_fatal_error("Unable to find class library " + LibName +
                             " for device " + Name);
}

DeviceCompiler::~DeviceCompiler() {}

void DeviceCompiler::addInitialLoweringPasses(llvm::legacy::PassManager &PM) {}

std::unique_ptr<clang::CompilerInvocation>
DeviceCompiler::createInvocation(llvm::MemoryBuffer &Src,
                                 llvm::StringRef UserOpts,
                                 clang::DiagnosticsEngine &Diags) {
  llvm::StringRef DefaultOpts = "-cl-std=CL ";
  llvm::StringRef EnvOpts = sys::GetEnv("OPENCRUN_COMPILER_OPTIONS");
  std::istringstream ISS(DefaultOpts.str() + EnvOpts.str() + UserOpts.str());
  std::string Token;
  llvm::SmallVector<const char *, 16> Argv;

  // Build command line.
  while(ISS >> Token) {
    char *CurArg = new char[Token.size() + 1];

    std::strcpy(CurArg, Token.c_str());
    Argv.push_back(CurArg);
  }
  Argv.push_back("<opencl-sources.cl>");

  using CompilerInvocation = clang::CompilerInvocation;
  auto Invocation = llvm::make_unique<CompilerInvocation>();

  // Create invocation.
  CompilerInvocation::CreateFromArgs(*Invocation, Argv.data(),
                                     Argv.data() + Argv.size(), Diags);

  // Remap file to in-memory buffer.
  auto &PreprocOpts = Invocation->getPreprocessorOpts();
  PreprocOpts.addRemappedFile("<opencl-sources.cl>", &Src);
  PreprocOpts.RetainRemappedFileBuffers = true;

  // Implicit target include
  llvm::SmallString<16> InclName("ocltarget.");
  InclName += Name;
  InclName += ".h";
  PreprocOpts.Includes.push_back(InclName.str());

  // Add include paths.
  auto &HdrSearchOpts = Invocation->getHeaderSearchOpts();
  
  llvm::SmallString<32> Path;
  if (sys::HasEnv("OPENCRUN_PREFIX_LLVM"))
    llvm::sys::path::append(Path, sys::GetEnv("OPENCRUN_PREFIX_LLVM"));
  else
    llvm::sys::path::append(Path, LLVM_PREFIX);
  llvm::sys::path::append(Path, "lib", "clang", CLANG_VERSION_STRING, "include");
  HdrSearchOpts.AddPath(Path.str(), clang::frontend::System, false, false);
  HdrSearchOpts.AddSystemHeaderPrefix(Path.str(), true);

  Path.clear();
  if (sys::HasEnv("OPENCRUN_PREFIX"))
    llvm::sys::path::append(Path, sys::GetEnv("OPENCRUN_PREFIX"));
  else
    llvm::sys::path::append(Path, LLVM_PREFIX);
  llvm::sys::path::append(Path, "lib", "opencrun", "include");
  HdrSearchOpts.AddPath(Path.str(), clang::frontend::System, false, false);
  HdrSearchOpts.AddSystemHeaderPrefix(Path.str(), true);

  // Set target triple.
  auto &TargetOpts = Invocation->getTargetOpts();
  TargetOpts.Triple = TM->getTargetTriple().str();
  TargetOpts.CPU = TM->getTargetCPU();
  TargetOpts.Features = TargetFeatures;

  // Code generation options: emit kernel arg metadata + no optimizations
  auto &CodeGenOpts = Invocation->getCodeGenOpts();
  CodeGenOpts.EmitOpenCLArgMetadata = true;
  CodeGenOpts.StackRealignment = true;

  return Invocation;
}

std::unique_ptr<llvm::Module>
DeviceCompiler::compileSource(llvm::MemoryBuffer &Src, llvm::StringRef Opts,
                              llvm::raw_ostream &OS) {
  if (!TM)
    return nullptr;

  llvm::sys::ScopedLock Lock(ThisLock);

  // Create the compiler.
  clang::CompilerInstance Compiler;

  // Create default DiagnosticsEngine and setup client.
  llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts;
  DiagOpts = new clang::DiagnosticOptions();
  Compiler.createDiagnostics(
    new clang::TextDiagnosticPrinter(OS, DiagOpts.get()));

  // Configure compiler invocation.
  auto Invocation = createInvocation(Src, Opts, Compiler.getDiagnostics());
  Compiler.setInvocation(Invocation.release());

  // Launch compiler.
  LLVMCodeGenAction GenBitCode(Ctx);
  if (!Compiler.ExecuteAction(GenBitCode))
    return nullptr;

  auto Mod = GenBitCode.takeModule();
  llvm::legacy::PassManager PM;
  addInitialLoweringPasses(PM);
  PM.run(*Mod);

  return Mod;
}

static llvm::Pass *createTTI(llvm::TargetMachine &TM) {
  return llvm::createTargetTransformInfoWrapperPass(TM.getTargetIRAnalysis());
}

bool DeviceCompiler::optimize(llvm::Module &M) {
  if (!TM)
    return false;

  llvm::sys::ScopedLock Lock(ThisLock);

  llvm::Triple TargetTriple(TM->getTargetTriple());

  llvm::PassManagerBuilder PMB;
  PMB.OptLevel = 2;
  PMB.SizeLevel = 0;
  PMB.BBVectorize = true;
  PMB.SLPVectorize = true;
  PMB.LoopVectorize = true;

  PMB.DisableUnitAtATime = false;
  PMB.DisableUnrollLoops = false;
  PMB.MergeFunctions = true;
  PMB.PrepareForLTO = false;
  PMB.RerollLoops = true;

  PMB.LibraryInfo = new llvm::TargetLibraryInfoImpl(TargetTriple);
  PMB.Inliner = llvm::createFunctionInliningPass(2, 0);

  // Set up the per-function pass manager.
  auto FPM = llvm::make_unique<llvm::legacy::FunctionPassManager>(&M);
  FPM->add(createTTI(*TM));
  PMB.populateFunctionPassManager(*FPM);

  // Set up the per-module pass manager.
  auto MPM = llvm::make_unique<llvm::legacy::PassManager>();
  MPM->add(createTTI(*TM));
  PMB.populateModulePassManager(*MPM);

  // Run per-function passes.
  FPM->doInitialization();
  for (auto &F : M)
    if (!F.isDeclaration())
      FPM->run(F);
  FPM->doFinalization();

  // Run per-module passes.
  MPM->run(M);

  return true;
}

std::unique_ptr<llvm::Module>
DeviceCompiler::linkModules(llvm::ArrayRef<llvm::Module*> Modules) {
  llvm_unreachable("Not implemented yet!");
}

bool DeviceCompiler::linkInBuiltins(llvm::Module &M) {
  auto BuiltinsCopy =
    std::unique_ptr<llvm::Module>{llvm::CloneModule(Builtins.get())};

  bool Failed =
    llvm::Linker::LinkModules(&M, BuiltinsCopy.get(),
                              llvm::Linker::LinkOnlyNeeded |
                              llvm::Linker::InternalizeLinkedSymbols);

  llvm::legacy::PassManager PM;
  PM.add(createTTI(*TM));
  PM.add(llvm::createFunctionInliningPass(100));
  PM.add(llvm::createConstantPropagationPass());
  PM.add(llvm::createInstructionCombiningPass());
  PM.add(llvm::createReassociatePass());
  PM.add(llvm::createEarlyCSEPass());
  PM.add(llvm::createCFGSimplificationPass());
  PM.add(llvm::createDeadCodeEliminationPass());
  PM.run(M);

  return !Failed;
}

std::unique_ptr<llvm::Module>
DeviceCompiler::loadBitcode(llvm::MemoryBufferRef Src) {
  if (auto BCOrErr = llvm::parseBitcodeFile(Src, Ctx))
    return std::move(BCOrErr.get());

  return nullptr;
}
