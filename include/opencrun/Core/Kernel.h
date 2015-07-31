
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
    LocalBufferArg,
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

class LocalBufferKernelArg : public KernelArg {
public:
  static bool classof(const KernelArg *Arg) {
    return Arg->GetType() == KernelArg::LocalBufferArg;
  }

public:
  LocalBufferKernelArg(unsigned Position, size_t Size)
   : KernelArg(KernelArg::LocalBufferArg, Position), Size(Size) {}


public:
  size_t getSize() const { return Size; }

private:
  size_t Size;
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

class KernelDescriptor : public MTRefCountedBase<KernelDescriptor> {
public:
  typedef std::map<Device *, KernelInfo> KernelInfoContainer;

public:
  KernelDescriptor(llvm::StringRef Name, Program &P, KernelInfoContainer &&I);
  ~KernelDescriptor();

  Context &getContext() const { return Prog->GetContext(); }
  Program &getProgram() const { return *Prog; }
  llvm::StringRef getName() const { return Name; }

  llvm::Function *getFunction(const Device *Dev) const {
    auto I = Infos.find(const_cast<Device*>(Dev));
    return I != Infos.end() ? I->second.getFunction() : nullptr;
  }

  bool isBuiltForDevice(const Device *Dev) const {
    return Infos.count(const_cast<Device*>(Dev));
  }

  bool isBuiltForSingleDevice() const {
    return Infos.size() == 1;
  }

  KernelInfo getKernelInfo() const {
    assert(!Infos.empty());
    return Infos.begin()->second;
  }

  bool hasRequiredWorkGroupSizes(const Device *Dev) const;
  bool getRequiredWorkGroupSizes(llvm::SmallVectorImpl<size_t> &Sizes,
                                 const Device *Dev) const;

  bool getMaxWorkGroupSize(size_t &Size, const Device *Dev) const;
  bool getMinLocalMemoryUsage(size_t &Size, const Device *Dev) const;
  bool getMinPrivateMemoryUsage(size_t &Size, const Device *Dev) const;

  void addLocalArgsSize(size_t Size) {
    LocalArgsSz += Size;
  }

  size_t getLocalArgsSize() const {
    return LocalArgsSz;
  }

private:
  llvm::SmallString<64> Name;
  KernelInfoContainer Infos;
  llvm::IntrusiveRefCntPtr<Program> Prog;
  size_t LocalArgsSz; 
};

class Kernel : public _cl_kernel, public MTRefCountedBase<Kernel> {
public:
  typedef llvm::SmallVector<KernelArg *, 8> ArgumentsContainer;
  typedef ArgumentsContainer::iterator arg_iterator;

public:
  Kernel(KernelDescriptor *KD) : Desc(KD), Arguments(GetArgCount()) {}
  Kernel(const Kernel &K);
  ~Kernel();

  const KernelDescriptor &getDescriptor() const { return *Desc; }

  Context &GetContext() const { return Desc->getContext(); }
  Program &GetProgram() const { return Desc->getProgram(); }
  llvm::StringRef GetName() const { return Desc->getName(); }

  arg_iterator arg_begin() { return Arguments.begin(); }
  arg_iterator arg_end() { return Arguments.end(); }

  bool AreAllArgsSpecified() const;

  unsigned GetArgCount() const;

  llvm::StringRef GetArgName(unsigned I) const;
  llvm::StringRef GetArgTypeName(unsigned I) const;

  cl_kernel_arg_address_qualifier GetArgAddressQualifier(unsigned I) const;
  cl_kernel_arg_access_qualifier GetArgAccessQualifier(unsigned I) const;
  cl_kernel_arg_type_qualifier GetArgTypeQualifier(unsigned I) const;

  cl_int SetArg(unsigned I, size_t Size, const void *Arg);

public:
  static bool classof(const _cl_kernel *Kern) { return true; }

private:
  cl_int SetBufferArg(unsigned I, size_t Size, const void *Arg);
  cl_int SetLocalBufferArg(unsigned I, size_t Size, const void *Arg);
  cl_int SetImageArg(unsigned I, size_t Size, const void *Arg);
  cl_int SetSamplerArg(unsigned I, size_t Size, const void *Arg);
  cl_int SetByValueArg(unsigned I, size_t Size, const void *Arg);

private:
  llvm::sys::Mutex ThisLock;

  llvm::IntrusiveRefCntPtr<KernelDescriptor> Desc;
  ArgumentsContainer Arguments;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_KERNEL_H
