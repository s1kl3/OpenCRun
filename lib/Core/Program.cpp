
#include "opencrun/Core/Program.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/Device.h"
#include "opencrun/Core/DeviceCompiler.h"
#include "opencrun/Core/Kernel.h"

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/SourceMgr.h"

#include "clang/Frontend/ChainedDiagnosticConsumer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"

using namespace opencrun;

//
// CompilerCallbackClojure implementation.
//

void CompilerCallbackClojure::CompilerDone(Program &Prog) {
  if(Callback)
    Callback(&Prog, UserData);
}

//
// Program implementation.
//

#define RETURN_WITH_ERROR(ERR, MSG) \
  {                                 \
  Ctx->ReportDiagnostic(MSG);       \
  return ERR;                       \
  }

Program::Program(Context &Ctx, llvm::MemoryBuffer &Src) : Ctx(&Ctx),
                                                          Src(&Src) {
  Devices.resize(Ctx.device_size());
  std::copy(Ctx.device_begin(), Ctx.device_end(), Devices.begin());
}

Program::Program(Context &Ctx, BinariesContainer &Binaries) : Ctx(&Ctx) {
  for(binary_iterator I = Binaries.begin(), E = Binaries.end(); I!= E; ++I) {
    Device &Dev = *I->first;
    llvm::MemoryBuffer *Binary = I->second;

    Devices.push_back(&Dev);

    BuildInfo[&Dev] = new BuildInformation();
    BuildInformation &Info = *BuildInfo[&Dev];

    Info.SetBinary(Binary);

    auto BC = Dev.getCompiler()->loadBitcode(Binary->getMemBufferRef());
    if (BC) {
      Info.SetBitCode(BC.release());
      Info.RegisterBuildDone(true);
    }
  }
}

cl_int Program::Build(DevicesContainer &Devs,
                      llvm::StringRef Opts,
                      CompilerCallbackClojure &CompilerNotify) {
  if(HasAttachedKernels())
    RETURN_WITH_ERROR(CL_INVALID_OPERATION,
                      "cannot build a program with attached kernel[s]");

  device_iterator I, E;

  if(Devs.empty()) {
    I = device_begin();
    E = device_end();
  } else {
    I = Devs.begin();
    E = Devs.end();
  }

  cl_int ErrCode = CL_SUCCESS;

  for(; I != E && ErrCode == CL_SUCCESS; ++I)
    ErrCode = Build(**I, Opts);

  // No concurrent builds.
  CompilerNotify.CompilerDone(*this);

  return ErrCode;
}

// Program requires two version of this macro.

#undef RETURN_WITH_ERROR

#define RETURN_WITH_ERROR(VAR, ERRCODE, MSG) \
  {                                          \
  Ctx->ReportDiagnostic(MSG);                \
  if(VAR)                                    \
    *VAR = ERRCODE;                          \
  return NULL;                               \
  }

Kernel *Program::CreateKernel(llvm::StringRef KernName, cl_int *ErrCode) {
  llvm::sys::ScopedLock Lock(ThisLock);

  if(BuildInfo.empty())
    RETURN_WITH_ERROR(ErrCode,
                      CL_INVALID_PROGRAM_EXECUTABLE,
                      "no program has been built");

  buildinfo_iterator I = BuildInfo.begin(),
                     E = BuildInfo.end();

  for(; I != E; ++I) {
    BuildInformation &Fst = *I->second;

    if(Fst.DefineKernel(KernName))
      break;
  }

  if(I == E)
    RETURN_WITH_ERROR(ErrCode, CL_INVALID_KERNEL_NAME, "no kernel definition");

  BuildInformation &Fst = *I->second;

  KernelDescriptor::KernelInfoContainer Infos;
  Infos[I->first] = Fst.GetKernel(KernName);

  for(buildinfo_iterator J = ++I; J != E; ++J) {
    Device &Dev = *J->first;
    BuildInformation &Cur = *J->second;

    if(!Cur.IsBuilt())
      continue;

    if(Cur.GetKernelSignature(KernName) != Fst.GetKernelSignature(KernName))
      RETURN_WITH_ERROR(ErrCode,
                        CL_INVALID_KERNEL_DEFINITION,
                        "kernel signatures do not match");

    Infos[&Dev] = Cur.GetKernel(KernName);
  }

  std::unique_ptr<KernelDescriptor> Desc;
  Desc.reset(new KernelDescriptor(KernName, *this, std::move(Infos)));

  std::unique_ptr<Kernel> Kern(new Kernel(Desc.get()));
  AttachedKernels.insert(Desc.release());

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return Kern.release();
}

cl_int Program::CreateKernelsInProgram(cl_uint num_kernels, 
                                       cl_kernel *kernels, 
                                       cl_uint *num_kernels_ret) {
  llvm::sys::ScopedLock Lock(ThisLock);

  if(BuildInfo.empty())
    return CL_INVALID_PROGRAM_EXECUTABLE;

  typedef llvm::SmallVector<std::pair<Device *, KernelInfo>,
                            4> DeviceFunctionVector;

  typedef std::map<llvm::StringRef,
                   DeviceFunctionVector> SignaturesMap;
      
  SignaturesMap SignMap;

  buildinfo_iterator BI = BuildInfo.begin(),
                     BE = BuildInfo.end();

  for(; BI != BE; ++BI) {
    Device &Dev = *BI->first;
    BuildInformation &Cur = *BI->second;

    if(!Cur.IsBuilt())
      continue;

    BuildInformation::kernel_info_iterator KI = Cur.kernel_begin(),
                                           KE = Cur.kernel_end();

    for(; KI != KE; ++KI) {
      llvm::StringRef KernName = KI->getName();
      SignMap[KernName].push_back(std::make_pair(&Dev, *KI));
    }
  }

  if(kernels) {
    if(num_kernels < SignMap.size())
      return CL_INVALID_VALUE;

    // This will contain the created Kernel objects.
    llvm::SmallVector<opencrun::Kernel *, 4> Kernels;

    SignaturesMap::iterator SI = SignMap.begin(),
      SE = SignMap.end();

    for(; SI != SE; ++SI) {
      KernelDescriptor::KernelInfoContainer Infos;

      DeviceFunctionVector &DevFun = SI->second;

      for(unsigned I = 0; I < DevFun.size(); ++I)
        Infos[DevFun[I].first] = DevFun[I].second;

      std::unique_ptr<KernelDescriptor> Desc;
      Desc.reset(new KernelDescriptor(SI->first, *this, std::move(Infos)));
      Kernels.push_back(new Kernel(Desc.get()));
      AttachedKernels.insert(Desc.release());
    }

    for(unsigned I = 0; I < num_kernels && I < Kernels.size(); ++I)
      *kernels++ = Kernels[I];
  }

  if(num_kernels_ret)
    *num_kernels_ret = SignMap.size();

  return CL_SUCCESS;
}

// Switch again, only to keep naming convention consistent.

#undef RETURN_WITH_ERROR

#define RETURN_WITH_ERROR(ERR, MSG) \
  {                                 \
  Ctx->ReportDiagnostic(MSG);       \
  return ERR;                       \
  }

cl_int Program::Build(Device &Dev, llvm::StringRef Opts) {
  llvm::sys::ScopedLock Lock(ThisLock);

  if(!BuildInfo.count(&Dev))
    BuildInfo[&Dev] = new BuildInformation();

  BuildInformation &Info = *BuildInfo[&Dev];

  if(!IsAssociatedWith(Dev))
    RETURN_WITH_ERROR(CL_INVALID_DEVICE,
                      "device not associated with program");

  if(!Src) {
    // No bit-code available, cannot build anything.
    if(!Info.HasBitCode())
      RETURN_WITH_ERROR(CL_INVALID_BINARY,
                        "no program source/binary available");

    // No sources, but bit-code available.
    return CL_SUCCESS;
  }

  if(Info.BuildInProgress())
    RETURN_WITH_ERROR(CL_INVALID_OPERATION,
                      "previously started build not yet terminated");

  // Needed to log compilation results into the program structure.
  llvm::raw_string_ostream Log(Info.GetBuildLog());

  // Build in progress.
  Info.SetBuildOptions(Opts);
  Info.RegisterBuildInProgress();

  // Invoke the compiler.
  auto BitCode = Dev.getCompiler()->compileSource(*Src, Opts, Log);
  if (Opts.find("-cl-opt-disable") == llvm::StringRef::npos)
    Dev.getCompiler()->optimize(*BitCode);

  if (BitCode) {
    // TODO: The generated LLVM bitcode is stored as the binary code for every
    // device but, in some cases, a device may require a shared object as a binary.
    std::string BitCodeDump;
    llvm::raw_string_ostream BitCodeOS(BitCodeDump);
    llvm::WriteBitcodeToFile(BitCode.get(), BitCodeOS);
    BitCodeOS.flush();

    auto Binary = llvm::MemoryBuffer::getMemBufferCopy(BitCodeDump);

    Info.SetBinary(Binary.release());
  }

  bool Success = BitCode != nullptr;

  Info.SetBitCode(BitCode.release());

  // Build done.
  Info.RegisterBuildDone(Success);
  return Success ? CL_SUCCESS : CL_BUILD_PROGRAM_FAILURE;
}

// Both Program and ProgramBuilder need this macro.

#undef RETURN_WITH_ERROR

//
// ProgramBuilder implementation.
//

#define RETURN_WITH_ERROR(VAR) \
  {                            \
  if(VAR)                      \
    *VAR = this->ErrCode;      \
  return NULL;                 \
  }

ProgramBuilder::ProgramBuilder(Context &Ctx) : Ctx(Ctx),
                                               ErrCode(CL_SUCCESS) { }

ProgramBuilder &ProgramBuilder::SetSources(unsigned Count,
                                           const char *Srcs[],
                                           const size_t Lengths[]) {
  if(!Count || !Srcs)
    return NotifyError(CL_INVALID_VALUE, "no program given");

  std::string Buf;

  for(unsigned I = 0; I < Count; ++I) {
    if(!Srcs[I])
      return NotifyError(CL_INVALID_VALUE, "null program given");

    llvm::StringRef Src;

    if(Lengths && Lengths[I])
      Src = llvm::StringRef(Srcs[I], Lengths[I]);
    else
      Src = llvm::StringRef(Srcs[I]);

    Buf += Src;
  }

  this->Srcs = llvm::MemoryBuffer::getMemBufferCopy(Buf).release();

  return *this;
}

ProgramBuilder &ProgramBuilder::SetBinaries(const cl_device_id *DevList,
                                            unsigned NumDevs,
                                            const unsigned char** Binaries,
                                            const size_t *Lengths,
                                            cl_int *BinStatus) {
  for(unsigned I = 0; I < NumDevs; ++I) {
    if(!DevList[I])
      return NotifyError(CL_INVALID_DEVICE, "null device given");

    Device *Dev = llvm::cast<Device>(DevList[I]);
    if(!Ctx.IsAssociatedWith(*Dev))
      return NotifyError(CL_INVALID_DEVICE, "device not associated with contex");

    if(!Binaries[I]) {
      if(BinStatus) BinStatus[I] = CL_INVALID_VALUE;
      return NotifyError(CL_INVALID_VALUE, "null binary given");
    }

    if(!Lengths[I]) {
      if(BinStatus) BinStatus[I] = CL_INVALID_VALUE;
      return NotifyError(CL_INVALID_VALUE, "null length given");
    }

    // TODO: Check if the program binary is valid for the device,
    // otherwise return CL_INVALID_BINARY.

    auto Ref = llvm::StringRef{reinterpret_cast<const char*>(Binaries[I]),
                               Lengths[I]};

    this->Binaries[Dev] = llvm::MemoryBuffer::getMemBufferCopy(Ref).release();

    BinStatus[I] = CL_SUCCESS;
  }

  return *this;
}

Program *ProgramBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  if(!Binaries.empty())
    return new Program(Ctx, Binaries);

  return new Program(Ctx, *Srcs);
}

ProgramBuilder &ProgramBuilder::NotifyError(cl_int ErrCode, const char *Msg) {
  Ctx.ReportDiagnostic(Msg);
  this->ErrCode = ErrCode;

  return *this;
}
