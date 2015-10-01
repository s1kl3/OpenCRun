
#ifndef OPENCRUN_CORE_PROGRAM_H
#define OPENCRUN_CORE_PROGRAM_H

#include "CL/opencl.h"

#include "opencrun/Util/ModuleInfo.h"
#include "opencrun/Util/MTRefCounted.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Mutex.h"

#include <map>

// OpenCL extensions defines macro that clashes with clang internal symbols.
// This is a workaround to include clang needed headers. Is this the right
// solution?
#define cl_khr_gl_sharing_save cl_khr_gl_sharing
#undef cl_khr_gl_sharing
#include "clang/Frontend/CompilerInvocation.h"
#define cl_khr_gl_sharing cl_khr_gl_sharing_save

struct _cl_program { };

namespace opencrun {

class Context;
class Device;
class Kernel;
class KernelDescriptor;
class Program;

class CompilerCallbackClojure {
public:
  typedef void (CL_CALLBACK *Signature)(cl_program, void *);
  typedef void *UserDataSignature; 

public:
  CompilerCallbackClojure(Signature Callback, UserDataSignature UserData) :
    Callback(Callback),
    UserData(UserData) { }

public:
  void CompilerDone(Program &Prog);

private:
  const Signature Callback;
  const UserDataSignature UserData;
};

class BuildInformation {
public:
  typedef ModuleInfo::iterator kernel_info_iterator;

public:
  kernel_info_iterator kernel_begin() {
    return ModuleInfo(*BitCode).begin();
  }

  kernel_info_iterator kernel_end() {
    return ModuleInfo(*BitCode).end();
  }

public:
  BuildInformation() : BuildStatus(CL_BUILD_NONE) { }
  BuildInformation(const BuildInformation &That); // Do not implement.
  void operator=(const BuildInformation &That); // Do not implement.

public:
  void RegisterBuildInProgress() { BuildStatus = CL_BUILD_IN_PROGRESS; }

  void RegisterBuildDone(bool Success = false) {
    BuildStatus = Success ? CL_BUILD_SUCCESS : CL_BUILD_ERROR;
  }

public:
  void SetBitCode(llvm::Module *BitCode) {
    this->BitCode.reset(BitCode);
  }

  void SetBuildOptions(llvm::StringRef Opts) {
    BuildOpts = Opts;
  }

  void SetBinary(llvm::MemoryBuffer *Binary) {
    this->Binary.reset(Binary);
  }

public:
  std::string &GetBuildLog() { return BuildLog; }

  llvm::StringRef GetBuildOptions() { return BuildOpts.str(); }

  cl_build_status GetBuildStatus() { return BuildStatus; }

  KernelInfo GetKernel(llvm::StringRef KernName) {
    if (!BitCode)
      return NULL;

    ModuleInfo Info(*BitCode); 
    auto I = Info.find(KernName);

    return I != Info.end() ? *I : NULL;
  }

  KernelSignature GetKernelSignature(llvm::StringRef KernName) {
    if (!BitCode)
      return NULL;

    ModuleInfo Info(*BitCode); 
    auto I = Info.find(KernName);

    return I != Info.end() ? I->getSignature() : NULL;
  }

  llvm::StringRef GetBinary() { return Binary->getBuffer(); }

  size_t GetBinarySize() { return Binary->getBufferSize(); }

public:
  bool IsBuilt() const { return BuildStatus == CL_BUILD_SUCCESS; }
  bool BuildInProgress() const { return BuildStatus == CL_BUILD_IN_PROGRESS; }
  bool HasBitCode() const { return bool(BitCode); }

  bool DefineKernel(llvm::StringRef KernName) {
    return ModuleInfo(*BitCode).contains(KernName);
  }

private:
  cl_build_status BuildStatus;
  std::string BuildLog;
  llvm::SmallString<32> BuildOpts;

  std::unique_ptr<llvm::Module> BitCode;
  std::unique_ptr<llvm::MemoryBuffer> Binary;
};

class Program : public _cl_program, public MTRefCountedBase<Program> {
public:
  static bool classof(const _cl_program *Prog) { return true; }

public:
  typedef llvm::SmallVector<Device *, 2> DevicesContainer;
  typedef std::map<Device *, BuildInformation *> BuildInformationContainer;
  typedef std::map<Device *, llvm::MemoryBuffer *> BinariesContainer;
  typedef llvm::SmallPtrSet<const KernelDescriptor*, 4> AttachedKernelsContainer;

  typedef DevicesContainer::iterator device_iterator;
  typedef BuildInformationContainer::iterator buildinfo_iterator;
  typedef BinariesContainer::iterator binary_iterator;
  typedef AttachedKernelsContainer::iterator kernel_iterator;

public:
  device_iterator device_begin() { return Devices.begin(); }
  device_iterator device_end() { return Devices.end(); }

  buildinfo_iterator buildinfo_begin() { return BuildInfo.begin(); }
  buildinfo_iterator buildinfo_end() { return BuildInfo.end(); }

public:
  DevicesContainer::size_type device_size() const { return Devices.size(); }
  BuildInformationContainer::size_type buildinfo_size() const { return BuildInfo.size(); }

public:
  ~Program() { llvm::DeleteContainerSeconds(BuildInfo); }

private:
  Program(Context &Ctx, llvm::MemoryBuffer &Src);
  Program(Context &Ctx, BinariesContainer &Binaries);

public:
  cl_int Build(DevicesContainer &Devs,
               llvm::StringRef Opts,
               CompilerCallbackClojure &CompilerNotify);

  Kernel *CreateKernel(llvm::StringRef KernName, cl_int *ErrCode);

  cl_int CreateKernelsInProgram(cl_uint NumKernels, 
                                cl_kernel *Kernels, 
                                cl_uint *NumKernelsRet);

public:
  Context &GetContext() { return *Ctx; }

  llvm::StringRef GetSource() { return Src->getBuffer(); }

  BuildInformation &GetBuildInformation(Device &Dev) { return *BuildInfo[&Dev]; }

  AttachedKernelsContainer &GetAttachedKernels() { return AttachedKernels; }

public:
  bool IsAssociatedWith(Device &Dev) {
    return std::count(Devices.begin(), Devices.end(), &Dev);
  }

  bool IsBuiltFor(Device &Dev) {
    llvm::sys::ScopedLock Lock(ThisLock);

    return BuildInfo[&Dev]->IsBuilt();
  }

  bool HasAttachedKernels() {
    llvm::sys::ScopedLock Lock(ThisLock);

    return !AttachedKernels.empty();
  }

  void DetachKernel(const KernelDescriptor &Kern) {
    llvm::sys::ScopedLock Lock(ThisLock);

    AttachedKernels.erase(&Kern);
  }

private: 
  cl_int Build(Device &Dev, llvm::StringRef Opts);

private:
  llvm::sys::Mutex ThisLock;

  llvm::IntrusiveRefCntPtr<Context> Ctx;
  std::unique_ptr<llvm::MemoryBuffer> Src;

  // Default diagnostic: don't colorize or format the output, it must write to a
  // memory buffer.
  clang::DiagnosticOptions DiagOptions;

  DevicesContainer Devices;
  BuildInformationContainer BuildInfo;
  AttachedKernelsContainer AttachedKernels;

  friend class ProgramBuilder;
};

class ProgramBuilder {
public:
  ProgramBuilder(Context &Ctx);

public:
  ProgramBuilder &SetSources(unsigned Count,
                             const char *Srcs[],
                             const size_t Lengths[] = NULL);

  ProgramBuilder &SetBinaries(const cl_device_id *DevList,
                              unsigned NumDevs,
                              const unsigned char **Binaries,
                              const size_t *Lengths,
                              cl_int *BinStatus);

  Program *Create(cl_int *ErrCode);

private:
  ProgramBuilder &NotifyError(cl_int ErrCode, const char *Msg = "");

private:
  Context &Ctx;
  llvm::MemoryBuffer *Srcs;
  Program::BinariesContainer Binaries;

  cl_int ErrCode;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_PROGRAM_H
