#include "opencrun/Core/Device.h"
#include "opencrun/Device/CPU/CPUDevice.h"
#include "opencrun/Util/LLVMCodeGenAction.h"
#include "opencrun/Util/LLVMOptimizer.h"

#include "clang/Basic/Version.h"
#include "clang/Parse/ParseAST.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/system_error.h"

// OpenCL extensions defines macro that clashes with clang internal symbols.
// This is a workaround to include clang needed headers. Is this the right
// solution?
#define cl_khr_gl_sharing_save cl_khr_gl_sharing
#undef cl_khr_gl_sharing
#include "clang/Frontend/CompilerInstance.h"
#define cl_khr_gl_sharing cl_khr_gl_sharing_save

#include <sstream>

using namespace opencrun;

namespace opencrun {

template<>
class LLVMOptimizerInterfaceTraits<Device> {
public:
  static void registerExtensions(Device &Dev,
                                 llvm::PassManagerBuilder &PMB,
                                 LLVMOptimizerParams &Params) {
    Dev.addOptimizerExtensions(PMB, Params);
  }
};

}

//
// Device implementation.
//

Device::Device(llvm::StringRef Name, llvm::StringRef Triple) :
  Parent(NULL),
  BitCodeLibrary(NULL),
  EnvCompilerOpts(sys::GetEnv("OPENCRUN_COMPILER_OPTIONS")),
  Triple(Triple.str()) {
  // Force initialization here, I do not want to pass explicit parameter to
  // DeviceInfo.
  this->Name = Name;

  // Initialize the device.
  InitLibrary();
  InitCompiler();
}

Device::Device(Device &Parent, const PartitionPropertiesContainer &PartProps) :
  Parent(&Parent),
  PartProps(const_cast<PartitionPropertiesContainer &>(PartProps)),
  BitCodeLibrary(Parent.GetBitCodeLibrary()),
  EnvCompilerOpts(Parent.GetEnvCompilerOpts()),
  Triple(Parent.GetTriple()) {
    this->Name = Parent.GetName();  
    
    InitCompiler();
}

Device::~Device() {
  if(IsSubDevice()) {
    Device *Parent = GetParent();
    if(Parent->IsSubDevice())
      Parent->Release();
  }
}

bool Device::TranslateToBitCode(llvm::StringRef Opts,
                                clang::DiagnosticConsumer &Diag,
                                llvm::MemoryBuffer &Src,
                                llvm::Module *&Mod) {
  llvm::sys::ScopedLock Lock(ThisLock);

  // Create the compiler.
  clang::CompilerInstance Compiler;
 
  // Create default DiagnosticsEngine and setup client.
  Compiler.createDiagnostics(&Diag, false);

  // Configure compiler invocation.
  clang::CompilerInvocation *Invocation = new clang::CompilerInvocation();
  BuildCompilerInvocation(Opts, Src, *Invocation, Compiler.getDiagnostics());
  Compiler.setInvocation(Invocation);

  // Launch compiler.
  LLVMCodeGenAction ToBitCode(&LLVMCtx);
  bool Success = Compiler.ExecuteAction(ToBitCode);

  Mod = ToBitCode.takeModule();
  Success = Success && !Mod->MaterializeAll();

  LLVMOptimizer<Device> Opt(Compiler.getLangOpts(), Compiler.getCodeGenOpts(),
                            Compiler.getTargetOpts(), *this);
  Opt.run(Mod);

  return Success;
}

void Device::InitLibrary() {
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

  llvm::OwningPtr<llvm::MemoryBuffer> File;
  if (!llvm::MemoryBuffer::getFile(Path.str(), File))
    BitCodeLibrary.reset(llvm::ParseBitcodeFile(File.get(), LLVMCtx));

  if (!BitCodeLibrary)
    llvm::report_fatal_error("Unable to find class library " + LibName +
                             " for device " + Name);
}

void Device::InitCompiler() {
  llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();

  llvm::initializeCore(Registry);
  llvm::initializeScalarOpts(Registry);
  llvm::initializeIPO(Registry);
  llvm::initializeAnalysis(Registry);
  llvm::initializeIPA(Registry);
  llvm::initializeTransformUtils(Registry);
  llvm::initializeInstCombine(Registry);
  llvm::initializeInstrumentation(Registry);
}

void Device::BuildCompilerInvocation(llvm::StringRef UserOpts,
                                     llvm::MemoryBuffer &Src,
                                     clang::CompilerInvocation &Invocation,
                                     clang::DiagnosticsEngine &Diags) {
  std::istringstream ISS(EnvCompilerOpts + UserOpts.str());
  std::string Token;
  llvm::SmallVector<const char *, 16> Argv;

  // Build command line.
  while(ISS >> Token) {
    char *CurArg = new char[Token.size() + 1];

    std::strcpy(CurArg, Token.c_str());
    Argv.push_back(CurArg);
  }
  Argv.push_back("<opencl-sources.cl>");

  // Create invocation.
  clang::CompilerInvocation::CreateFromArgs(Invocation,
                                            Argv.data(),
                                            Argv.data() + Argv.size(),
                                            Diags);
  clang::CompilerInvocation::setLangDefaults(*Invocation.getLangOpts(),
                                             clang::IK_OpenCL,
                                            clang::LangStandard::lang_opencl11);

  // Remap file to in-memory buffer.
  clang::PreprocessorOptions &PreprocOpts = Invocation.getPreprocessorOpts();
  PreprocOpts.addRemappedFile("<opencl-sources.cl>", &Src);
  PreprocOpts.RetainRemappedFileBuffers = true;

  // Implicit target include
  llvm::SmallString<16> InclName("ocltarget.");
  InclName += Name;
  InclName += ".h";
  PreprocOpts.Includes.push_back(InclName.str());


  // Add include paths.
  clang::HeaderSearchOptions &HdrSearchOpts = Invocation.getHeaderSearchOpts();
  
  llvm::SmallString<32> Path;
  if (sys::HasEnv("OPENCRUN_PREFIX_LLVM"))
    llvm::sys::path::append(Path, sys::GetEnv("OPENCRUN_PREFIX_LLVM"));
  else
    llvm::sys::path::append(Path, LLVM_PREFIX);
  llvm::sys::path::append(Path, "lib", "clang", CLANG_VERSION_STRING, "include");
  HdrSearchOpts.AddPath(Path.str(), clang::frontend::Angled, false, false);

  Path.clear();
  if (sys::HasEnv("OPENCRUN_PREFIX"))
    llvm::sys::path::append(Path, sys::GetEnv("OPENCRUN_PREFIX"));
  else
    llvm::sys::path::append(Path, LLVM_PREFIX);
  llvm::sys::path::append(Path, "lib", "opencrun", "include");
  HdrSearchOpts.AddPath(Path.str(), clang::frontend::Quoted, false, false);
  HdrSearchOpts.AddPath(Path.str(), clang::frontend::Angled, false, false);

  // Set target triple.
  clang::TargetOptions &TargetOpts = Invocation.getTargetOpts();
  TargetOpts.Triple = Triple;

  // Code generation options: emit kernel arg metadata + no optimizations
  clang::CodeGenOptions &CodeGenOpts = Invocation.getCodeGenOpts();
  CodeGenOpts.EmitOpenCLArgMetadata = true;
  CodeGenOpts.DisableLLVMOpts = true;
}

sys::Time ProfilerTraits<Device>::ReadTime(Device &Profilable) {
  if(CPUDevice *CPU = llvm::dyn_cast<CPUDevice>(&Profilable))
    return ProfilerTraits<CPUDevice>::ReadTime(*CPU);

  llvm_unreachable("Unknown device type");
}

//
// SubDevicesBuilder implementation.
//

unsigned SubDevicesBuilder::Create(SubDevicesContainer *SubDevs, cl_int &ErrCode) {
  // As default device partitioning is not supported and no sub-device is
  // created.
  ErrCode = CL_INVALID_VALUE;

  return 0;
}
