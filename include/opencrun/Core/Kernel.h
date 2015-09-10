
#ifndef OPENCRUN_CORE_KERNEL_H
#define OPENCRUN_CORE_KERNEL_H

#include "CL/cl.h"

#include "opencrun/Core/MemoryObject.h"
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
  enum KernelArgKind {
    NoArg,
    BufferArg,
    LocalBufferArg,
    ImageArg,
    SamplerArg,
    ByValueArg
  };

public:
  KernelArg() : Kind(NoArg) {}
  KernelArg(unsigned Index, Buffer *Buf);
  KernelArg(unsigned Index, Image *Img);
  KernelArg(unsigned Index, Sampler *Smplr);
  KernelArg(unsigned Index, const void *ByValPtr, size_t ByValSize);
  KernelArg(unsigned Index, size_t LocalSize);

  KernelArg(const KernelArg &Arg);
  KernelArg(KernelArg &&Arg) noexcept;

  ~KernelArg();

  KernelArg &operator=(const KernelArg &Arg) {
    KernelArg CopyArg(Arg);
    *this = std::move(CopyArg);
    return *this;
  }
  KernelArg &operator=(KernelArg &&Arg) noexcept;

  bool isValid() const { return Kind != NoArg; }
  KernelArgKind getKind() const { return Kind; }
  unsigned getIndex() const { return Index; }

  Buffer *getBuffer() const {
    assert(Kind == BufferArg);
    return Buf.get();
  }
  Image *getImage() const {
    assert(Kind == ImageArg);
    return Img.get();
  }
  Sampler *getSampler() const {
    assert(Kind == SamplerArg);
    return Smplr.get();
  }
  void *getByValPtr() const {
    assert(Kind == ByValueArg);
    return ByVal.Ptr;
  }
  size_t getByValSize() const {
    assert(Kind == ByValueArg);
    return ByVal.Size;
  }
  size_t getLocalSize() const {
    assert(Kind == LocalBufferArg);
    return LocalSize;
  }

private:
  KernelArgKind Kind;
  unsigned Index;
  union {
    llvm::IntrusiveRefCntPtr<Buffer> Buf;
    llvm::IntrusiveRefCntPtr<Image> Img;
    llvm::IntrusiveRefCntPtr<Sampler> Smplr;
    struct { 
      void *Ptr;
      size_t Size;
    } ByVal;
    size_t LocalSize;
  };
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
  typedef std::vector<KernelArg> ArgumentsContainer;
  typedef ArgumentsContainer::const_iterator arg_iterator;

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
