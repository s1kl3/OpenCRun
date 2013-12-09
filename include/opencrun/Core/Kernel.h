
#ifndef OPENCRUN_CORE_KERNEL_H
#define OPENCRUN_CORE_KERNEL_H

#include "CL/cl.h"

#include "opencrun/Core/MemoryObj.h"
#include "opencrun/Core/Sampler.h"
#include "opencrun/Core/Program.h"
#include "opencrun/Passes/FootprintEstimate.h"
#include "opencrun/System/OS.h"
#include "opencrun/Util/MTRefCounted.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/ErrorHandling.h"

struct _cl_kernel { };

namespace opencrun {

class KernelArg {
public:
  enum Type {
    BufferArg,
    ImageArg,
    SamplerArg,
    ByValueArg
  };

protected:
  KernelArg(Type Ty, unsigned Position) : Ty(Ty), Position(Position) { }

public:
  Type GetType() const { return Ty; }
  unsigned GetPosition() const { return Position; }

private:
  Type Ty;
  unsigned Position;
};

class BufferKernelArg : public KernelArg {
public:
  static bool classof(const KernelArg *Arg) {
    return Arg->GetType() == KernelArg::BufferArg;
  }

public:
  BufferKernelArg(unsigned Position, Buffer *Buf, clang::LangAS::ID AddrSpace) :
    KernelArg(KernelArg::BufferArg, Position),
    Buf(Buf),
    AddrSpace(AddrSpace) { }

public:
  Buffer *GetBuffer() { return Buf; }

  bool OnGlobalAddressSpace() const {
    return AddrSpace == clang::LangAS::opencl_global;
  }

  bool OnConstantAddressSpace() const {
    return AddrSpace == clang::LangAS::opencl_constant;
  }

  bool OnLocalAddressSpace() const {
    return AddrSpace == clang::LangAS::opencl_local;
  }

private:
  Buffer *Buf;
  clang::LangAS::ID AddrSpace;
};

class ImageKernelArg : public KernelArg {
public:
  static bool classof(const KernelArg *Arg) {
    return Arg->GetType() == KernelArg::ImageArg;
  }

public:
  ImageKernelArg(unsigned Position, Image *Img, clang::OpenCLImageAccess ImgAccess) :
    KernelArg(KernelArg::ImageArg, Position),
    Img(Img),
    ImgAccess(ImgAccess) { }

public:
  Image *GetImage() { return Img; }

  bool IsReadOnly() const {
    return ImgAccess == clang::CLIA_read_only;
  }

  bool IsWriteOnly() const {
    return ImgAccess == clang::CLIA_write_only;
  }

private:
  Image *Img;
  clang::OpenCLImageAccess ImgAccess;
};

class SamplerKernelArg : public KernelArg {
public:
  static bool classof(const KernelArg *Arg) {
    return Arg->GetType() == KernelArg::SamplerArg;
  }

public:
  SamplerKernelArg(unsigned Position, const void *Arg) :
    KernelArg(KernelArg::SamplerArg, Position) { }

public:
  Sampler *GetSampler() { return Smplr; }

private:
  Sampler *Smplr;
};

class ByValueKernelArg : public KernelArg {
public:
  static bool classof(const KernelArg *Arg) {
    return Arg->GetType() == KernelArg::ByValueArg;
  }

public:
  ByValueKernelArg(unsigned Position, const void *Arg, size_t Size) :
    KernelArg(KernelArg::ByValueArg, Position),
    Size(Size) {
    // We need to allocate a chunk of memory for holding the parameter -- it is
    // passed by copy. Generally speaking, we do not have alignment
    // requirements. However, the CPU backed generates loads from this memory
    // location, and since this parameter may be a vector type we can incur into
    // alignment hazards! Force using the maximum natural alignment.
    this->Arg = sys::NaturalAlignedAlloc(Size);

    // Copy the parameter.
    std::memcpy(this->Arg, Arg, Size);
  }

public:
  void *GetArg() { return Arg; }

private:
  void *Arg;
  size_t Size;
};

class Kernel : public _cl_kernel, public MTRefCountedBase<Kernel> {
public:
  static bool classof(const _cl_kernel *Kern) { return true; }

public:
  typedef std::map<Device *, llvm::Function *> CodesContainer;

  typedef llvm::SmallVector<KernelArg *, 8> ArgumentsContainer;
  
  typedef ArgumentsContainer::iterator arg_iterator;

public:
  arg_iterator arg_begin() { return Arguments.begin(); }
  arg_iterator arg_end() { return Arguments.end(); }

public:
  Kernel(Program &Prog, CodesContainer &Codes) : Prog(&Prog),
                                                 Codes(Codes),
                                                 Arguments(GetArgCount()) { }

  ~Kernel();

public:
  cl_int SetArg(unsigned I, size_t Size, const void *Arg);

public:
  llvm::Function *GetFunction(Device &Dev) {
    return Codes.count(&Dev) ? Codes[&Dev] : NULL;
  }

  llvm::FunctionType &GetType() const {
    // All stored functions share the same signature, just return the first.
    CodesContainer::const_iterator I = Codes.begin();
    llvm::Function &Func = *I->second;

    return *Func.getFunctionType();
  }

  llvm::StringRef GetName() const {
    // All stored functions share the same signature, just return the first.
    CodesContainer::const_iterator I = Codes.begin();
    const llvm::Function &Func = *I->second;

    return Func.getName();
  }

  unsigned GetArgCount() const {
    const llvm::FunctionType &FuncTy = GetType();

    return FuncTy.getNumParams();
  }

  llvm::Module *GetModule(Device &Dev) {
    return Codes.count(&Dev) ? Codes[&Dev]->getParent() : NULL;
  }

  Context &GetContext() const { return Prog->GetContext(); }
  Program &GetProgram() const { return *Prog; }

  // TODO: implement. Attribute must be gathered from metadata.
  llvm::SmallVector<size_t, 4> &GetRequiredWorkGroupSizes() const {
    return *(new llvm::SmallVector<size_t, 4>(3, size_t(0)));
  }

  bool IsBuiltFor(Device &Dev) const { return Prog->IsBuiltFor(Dev); }
  bool IsBuiltForOnlyADevice() const { return Codes.size() == 1; }

  // TODO: implement. Depends on kernel argument setting code, not well written.
  bool AreAllArgsSpecified() const { return true; }

  // TODO: implement. Attribute must be gathered from metadata.
  bool RequireWorkGroupSizes() const { return false; }

  bool GetMaxWorkGroupSize(size_t &Size, Device *Dev = NULL);
  bool GetMinLocalMemoryUsage(size_t &Size, Device *Dev = NULL);
  bool GetMinPrivateMemoryUsage(size_t &Size, Device *Dev = NULL);

private:
  llvm::Type *GetArgType(unsigned I) const {
    llvm::FunctionType &KernTy = GetType();

    return I < KernTy.getNumParams() ? KernTy.getParamType(I) : NULL;
  }

  cl_int SetBufferArg(unsigned I, size_t Size, const void *Arg);
  cl_int SetImageArg(unsigned I, size_t Size, const void *Arg);
  cl_int SetSamplerArg(unsigned I, size_t Size, const void *Arg);
  cl_int SetByValueArg(unsigned I, size_t Size, const void *Arg);

  clang::LangAS::ID GetArgAddressSpace(unsigned I) {
    // All stored functions share the same signature, use the first.
    CodesContainer::iterator J = Codes.begin();
    llvm::Function &Kern = *J->second;

    OpenCLMetadataHandler OpenCLMDHandler(*Kern.getParent());

    return OpenCLMDHandler.GetArgAddressSpace(Kern, I);
  }

  clang::OpenCLImageAccess GetArgAccessQual(unsigned I) {
    // All stored functions share the same signature, use the first.
    CodesContainer::iterator J = Codes.begin();
    llvm::Function &Kern = *J->second;

    OpenCLMetadataHandler OpenCLMDHandler(*Kern.getParent());

    return OpenCLMDHandler.GetArgAccessQual(Kern, I);
  }

  llvm::StringRef GetArgTypeName(unsigned I) {
    // All stored functions share the same signature, use the first.
    CodesContainer::iterator J = Codes.begin();
    llvm::Function &Kern = *J->second;

    OpenCLMetadataHandler OpenCLMDHandler(*Kern.getParent());

    return OpenCLMDHandler.GetArgTypeName(Kern, I);
  }

  bool IsSampler(unsigned I);
  bool IsImage(unsigned I);
  bool IsBuffer(llvm::Type &Ty);
  bool IsByValue(llvm::Type &Ty);

  Device *RequireEstimates(Device *Dev = NULL);

private:
  llvm::sys::Mutex ThisLock;

  llvm::IntrusiveRefCntPtr<Program> Prog;
  CodesContainer Codes;

  ArgumentsContainer Arguments;

  llvm::OwningPtr<Footprint> Estimates;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_KERNEL_H
