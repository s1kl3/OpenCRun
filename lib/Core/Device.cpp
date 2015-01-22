#include "opencrun/Core/Device.h"
#include "opencrun/Device/CPU/CPUDevice.h"
#include "opencrun/Util/LLVMCodeGenAction.h"
#include "opencrun/Util/LLVMOptimizer.h"

#include "clang/Basic/Version.h"
#include "clang/Parse/ParseAST.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/ErrorHandling.h"

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

DeviceInfo::DeviceInfo(DeviceType Ty) : Type(Ty) {
  Vendor = "";
  Name = "";
  Version = "";
  DriverVersion = "";
  OpenCLCVersion = "";
  Profile = "";
  Extensions = "";
  BuiltInKernels = "";

  AddressBits = 0;
  ExecutionCapabilities = CanExecKernel;
  HostUnifiedMemory = false;
  LittleEndian = true;
  SupportErrorCorrection = false;

  MemoryBaseAddressAlignment = 0;
  MinimumDataTypeAlignment = 0;

  SinglePrecisionFPCapabilities = FPRoundToNearest | FPInfNaN;
  DoublePrecisionFPCapabilities = 0;
  HalfPrecisionFPCapabilities = 0;

  CompilerAvailable = false;
  LinkerAvailable = false;

  GlobalMemoryCacheType = NoCache;
  GlobalMemoryCachelineSize = 0;
  GlobalMemoryCacheSize = 0;
  GlobalMemorySize = 0;

  LocalMemoryMapping = PrivateLocal;
  LocalMemorySize = 32 * 1024;

  MaxClockFrequency = 0;
  MaxComputeUnits = 0;
  MaxConstantBufferSize = 64 * 1024;
  MaxConstantArguments = 8;
  MaxParameterSize = 0;

  // Native vector widths
  NativeCharVectorWidth = 0;
  NativeShortVectorWidth = 0;
  NativeIntVectorWidth = 0;
  NativeLongVectorWidth = 0;
  NativeFloatVectorWidth = 0;
  NativeDoubleVectorWidth = 0;
  NativeHalfVectorWidth = 0;

  // Preferred vector widths
  PreferredCharVectorWidth = 0;
  PreferredShortVectorWidth = 0;
  PreferredIntVectorWidth = 0;
  PreferredLongVectorWidth = 0;
  PreferredFloatVectorWidth = 0;
  PreferredDoubleVectorWidth = 0;
  PreferredHalfVectorWidth = 0;

  PrintfBufferSize = 0;

  PreferredInteropUserSync = true;

  ProfilingTimerResolution = 0;

  QueueProperties = ProfilingEnabled;

  // Images support
  SupportImages = false;
  MaxReadableImages = 0;
  MaxWriteableImages = 0;
  Image2DMaxWidth = 0;
  Image2DMaxHeight = 0;
  Image3DMaxWidth = 0;
  Image3DMaxHeight = 0;
  Image3DMaxDepth = 0;
  ImageMaxBufferSize = 0;
  ImageMaxArraySize = 0;
  MaxSamplers = 0;
  ImgFmts = nullptr;
  NumImgFmts = 0;

  // SubDevices support
  MaxSubDevices = 0;
  AffinityDomains = 0;

  // Other, non OpenCL specific properties.
  SizeTypeMax = (1ull << 32) - 1;
  PrivateMemorySize = 0;
  PreferredWorkGroupSizeMultiple = 1;
}

DeviceInfo::DeviceInfo(const DeviceInfo &DI)
 : Type(DI.Type),
   VendorID(DI.VendorID),

   MaxComputeUnits(DI.MaxComputeUnits),
   MaxWorkItemDimensions(DI.MaxWorkItemDimensions),
   MaxWorkItemSizes(DI.MaxWorkItemSizes),
   MaxWorkGroupSize(DI.MaxWorkGroupSize),

   PreferredCharVectorWidth(DI.PreferredCharVectorWidth),
   PreferredShortVectorWidth(DI.PreferredShortVectorWidth),
   PreferredIntVectorWidth(DI.PreferredIntVectorWidth),
   PreferredLongVectorWidth(DI.PreferredLongVectorWidth),
   PreferredFloatVectorWidth(DI.PreferredFloatVectorWidth),
   PreferredDoubleVectorWidth(DI.PreferredDoubleVectorWidth),
   PreferredHalfVectorWidth(DI.PreferredHalfVectorWidth),

   NativeCharVectorWidth(DI.NativeCharVectorWidth),
   NativeShortVectorWidth(DI.NativeShortVectorWidth),
   NativeIntVectorWidth(DI.NativeIntVectorWidth),
   NativeLongVectorWidth(DI.NativeLongVectorWidth),
   NativeFloatVectorWidth(DI.NativeFloatVectorWidth),
   NativeDoubleVectorWidth(DI.NativeDoubleVectorWidth),
   NativeHalfVectorWidth(DI.NativeHalfVectorWidth),

   MaxClockFrequency(DI.MaxClockFrequency),
   AddressBits(DI.AddressBits),
   MaxMemoryAllocSize(DI.MaxMemoryAllocSize),

   SupportImages(DI.SupportImages),
   MaxReadableImages(DI.MaxReadableImages),
   MaxWriteableImages(DI.MaxWriteableImages),
   Image2DMaxWidth(DI.Image2DMaxWidth),
   Image2DMaxHeight(DI.Image2DMaxHeight),
   Image3DMaxWidth(DI.Image3DMaxWidth),
   Image3DMaxHeight(DI.Image3DMaxHeight),
   Image3DMaxDepth(DI.Image3DMaxDepth),
   ImageMaxBufferSize(DI.ImageMaxBufferSize),
   ImageMaxArraySize(DI.ImageMaxArraySize),
   MaxSamplers(DI.MaxSamplers),
   ImgFmts(DI.ImgFmts),
   NumImgFmts(DI.NumImgFmts),

   MaxParameterSize(DI.MaxParameterSize),

   PrintfBufferSize(DI.PrintfBufferSize),

   MemoryBaseAddressAlignment(DI.MemoryBaseAddressAlignment),
   MinimumDataTypeAlignment(DI.MinimumDataTypeAlignment),

   SinglePrecisionFPCapabilities(DI.SinglePrecisionFPCapabilities),
   DoublePrecisionFPCapabilities(DI.DoublePrecisionFPCapabilities),
   HalfPrecisionFPCapabilities(DI.HalfPrecisionFPCapabilities),

   GlobalMemoryCacheType(DI.GlobalMemoryCacheType),
   GlobalMemoryCachelineSize(DI.GlobalMemoryCachelineSize),
   GlobalMemoryCacheSize(DI.GlobalMemoryCacheSize),
   GlobalMemorySize(DI.GlobalMemorySize),

   MaxConstantBufferSize(DI.MaxConstantBufferSize),
   MaxConstantArguments(DI.MaxConstantArguments),

   LocalMemoryMapping(DI.LocalMemoryMapping),
   LocalMemorySize(DI.LocalMemorySize),
   SupportErrorCorrection(DI.SupportErrorCorrection),

   HostUnifiedMemory(DI.HostUnifiedMemory),

   PreferredInteropUserSync(DI.PreferredInteropUserSync),

   ProfilingTimerResolution(DI.ProfilingTimerResolution),

   LittleEndian(DI.LittleEndian),

   CompilerAvailable(DI.CompilerAvailable),
   LinkerAvailable(DI.LinkerAvailable),

   ExecutionCapabilities(DI.ExecutionCapabilities),

   QueueProperties(DI.QueueProperties),

   MaxSubDevices(DI.MaxSubDevices),
   AffinityDomains(DI.AffinityDomains),

   Vendor(DI.Vendor),
   Name(DI.Name),
   Version(DI.Version),
   DriverVersion(DI.DriverVersion),
   OpenCLCVersion(DI.OpenCLCVersion),
   Profile(DI.Profile),
   Extensions(DI.Extensions),
   BuiltInKernels(DI.BuiltInKernels),

   SizeTypeMax(DI.SizeTypeMax),
   PrivateMemorySize(DI.PrivateMemorySize),
   PreferredWorkGroupSizeMultiple(DI.PreferredWorkGroupSizeMultiple)
{
}
//
// Device implementation.
//

Device::Device(DeviceType Ty, llvm::StringRef Name, llvm::StringRef Triple)
 : DeviceInfo(Ty), Parent(nullptr),
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
  DeviceInfo(Parent),
  Parent(&Parent),
  PartProps(const_cast<PartitionPropertiesContainer &>(PartProps)),
  EnvCompilerOpts(Parent.GetEnvCompilerOpts()),
  Triple(Parent.GetTriple()) {
    this->Name = Parent.GetName();  
  
    InitLibrary();
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
  Success = Success && !Mod->materializeAll();

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

  std::unique_ptr<llvm::MemoryBuffer> File;
  if (!llvm::MemoryBuffer::getFile(Path.str(), File))
    BitCodeLibrary.reset(llvm::parseBitcodeFile(File.get(), LLVMCtx).get());

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
  CodeGenOpts.StackRealignment = true;
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
