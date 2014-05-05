
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
  KernelArg(Type Ty, unsigned Position) : Ty(Ty), Position(Position) {}
  KernelArg(const KernelArg &Arg) : Ty(Arg.Ty), Position(Arg.Position) {}

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
  BufferKernelArg(unsigned Position, Buffer *Buf,
                  opencl::AddressSpace AddrSpace)
   : KernelArg(KernelArg::BufferArg, Position), Buf(Buf),
     AddrSpace(AddrSpace) {}

  BufferKernelArg(const BufferKernelArg &Arg)
   : KernelArg(Arg), Buf(Arg.Buf), AddrSpace(Arg.AddrSpace) {}

public:
  Buffer *GetBuffer() { return Buf.getPtr(); }

  bool OnGlobalAddressSpace() const {
    return AddrSpace == opencl::AS_Global;
  }

  bool OnConstantAddressSpace() const {
    return AddrSpace == opencl::AS_Constant;
  }

  bool OnLocalAddressSpace() const {
    return AddrSpace == opencl::AS_Local;
  }

private:
  llvm::IntrusiveRefCntPtr<Buffer> Buf;
  opencl::AddressSpace AddrSpace;
};

class ImageKernelArg : public KernelArg {
public:
  static bool classof(const KernelArg *Arg) {
    return Arg->GetType() == KernelArg::ImageArg;
  }

public:
  ImageKernelArg(unsigned Position, Image *Img, opencl::ImageAccess ImgAccess)
   : KernelArg(KernelArg::ImageArg, Position), Img(Img),
     ImgAccess(ImgAccess) {}

  ImageKernelArg(const ImageKernelArg &Arg)
   : KernelArg(Arg), Img(Arg.Img), ImgAccess(Arg.ImgAccess) {}

public:
  Image *GetImage() { return Img.getPtr(); }

  bool IsReadOnly() const {
    return ImgAccess == opencl::IA_ReadOnly;
  }

  bool IsWriteOnly() const {
    return ImgAccess == opencl::IA_WriteOnly;
  }

private:
  llvm::IntrusiveRefCntPtr<Image> Img;
  opencl::ImageAccess ImgAccess;
};

class SamplerKernelArg : public KernelArg {
public:
  static bool classof(const KernelArg *Arg) {
    return Arg->GetType() == KernelArg::SamplerArg;
  }

public:
  SamplerKernelArg(unsigned Position, Sampler *Smplr)
   : KernelArg(KernelArg::SamplerArg, Position),
     Smplr(Smplr) {}

  SamplerKernelArg(const SamplerKernelArg &Arg)
   : KernelArg(Arg), Smplr(Arg.Smplr) {}

public:
  Sampler *GetSampler() { return Smplr.getPtr(); }

private:
  llvm::IntrusiveRefCntPtr<Sampler> Smplr;
};

class ByValueKernelArg : public KernelArg {
public:
  static bool classof(const KernelArg *Arg) {
    return Arg->GetType() == KernelArg::ByValueArg;
  }

public:
  ByValueKernelArg(unsigned Position, const void *Arg, size_t Size)
   : KernelArg(KernelArg::ByValueArg, Position), Size(Size) {
    // We need to allocate a chunk of memory for holding the parameter -- it is
    // passed by copy. Generally speaking, we do not have alignment
    // requirements. However, the CPU backed generates loads from this memory
    // location, and since this parameter may be a vector type we can incur into
    // alignment hazards! Force using the maximum natural alignment.
    this->Arg = sys::NaturalAlignedAlloc(Size);

    // Copy the parameter.
    std::memcpy(this->Arg, Arg, Size);
  }

  ByValueKernelArg(const ByValueKernelArg &A)
   : KernelArg(A), Arg(A.Arg), Size(A.Size) {}

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
  class LifetimeHandler : public MTRefCountedBase<LifetimeHandler> {
  public:
    LifetimeHandler(Kernel &KernHandle);
    ~LifetimeHandler();

  private:
    Kernel &KernHandle;
  };

  typedef std::map<Device *, llvm::Function *> CodesContainer;

  typedef llvm::SmallVector<KernelArg *, 8> ArgumentsContainer;
  
  typedef ArgumentsContainer::iterator arg_iterator;

public:
  arg_iterator arg_begin() { return Arguments.begin(); }
  arg_iterator arg_end() { return Arguments.end(); }

public:
  Kernel(Program &Prog, CodesContainer &Codes)
   : Prog(&Prog), Codes(Codes), Arguments(GetArgCount()), 
     Lifetime(new LifetimeHandler(*this)) {}

  Kernel(const Kernel &K);

  ~Kernel();

public:
  cl_int SetArg(unsigned I, size_t Size, const void *Arg);

public:
  llvm::Function *GetFunction(Device &Dev) const {
    CodesContainer::const_iterator I = Codes.find(&Dev);
    return (I != Codes.end()) ? I->second : NULL;
  }

  llvm::StringRef GetName() const {
    // All stored functions share the same signature, just return the first.
    CodesContainer::const_iterator I = Codes.begin();
    const llvm::Function &Func = *I->second;

    return Func.getName();
  }

  unsigned GetArgCount() const {
    return GetSignature().getNumArguments();
  }

  llvm::StringRef GetArgName(unsigned I) const;

  llvm::Module *GetModule(Device &Dev) const {
    CodesContainer::const_iterator I = Codes.find(&Dev);
    return (I != Codes.end()) ? I->second->getParent() : NULL;
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

  void RegisterToDevices();
  void UnregisterFromDevices();

public:
  cl_kernel_arg_address_qualifier GetArgAddressQualifier(unsigned I) const;

  cl_kernel_arg_access_qualifier GetArgAccessQualifier(unsigned I) const;

  llvm::StringRef GetArgTypeName(unsigned I) const;

  cl_kernel_arg_type_qualifier GetArgTypeQualifier(unsigned I) const;

private:
  KernelSignature GetSignature() const;
  KernelInfo GetInfo() const;

  cl_int SetBufferArg(unsigned I, size_t Size, const void *Arg);
  cl_int SetImageArg(unsigned I, size_t Size, const void *Arg);
  cl_int SetSamplerArg(unsigned I, size_t Size, const void *Arg);
  cl_int SetByValueArg(unsigned I, size_t Size, const void *Arg);

private:
  llvm::sys::Mutex ThisLock;

  llvm::IntrusiveRefCntPtr<Program> Prog;
  CodesContainer Codes;
  ArgumentsContainer Arguments;
  llvm::IntrusiveRefCntPtr<LifetimeHandler> Lifetime;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_KERNEL_H
