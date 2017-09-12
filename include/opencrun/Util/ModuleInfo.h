#ifndef OPENCRUN_UTIL_MODULEINFO_H
#define OPENCRUN_UTIL_MODULEINFO_H

#include "opencrun/Util/OpenCLTypeSystem.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

#include <algorithm>

namespace opencrun {
  
class KernelArgInfo {
public:
  KernelArgInfo(llvm::MDNode *InfoMD) : MD(InfoMD) { }

  unsigned getNumArguments() const {
    if (IsGenericMD())
      return MD->getNumOperands() - 1;

    return MD->getNumOperands();
  }

  llvm::Metadata *getArgument(unsigned I) const {
    assert(I < getNumArguments());
    if (IsGenericMD())
      return MD->getOperand(I + 1).get();

    return MD->getOperand(I).get();
  }

  template<typename Ty>
  Ty *getArgumentAs(unsigned I) const {
    return llvm::mdconst::extract<Ty>(getArgument(I));
  }

  llvm::StringRef getArgumentAsString(unsigned I) const {
    return llvm::cast<llvm::MDString>(getArgument(I))->getString();
  }

private:
  bool IsGenericMD() const;

private:
  llvm::MDNode *MD;
};

class KernelSignature {
public:
  KernelSignature(llvm::MDNode *SignMD) : MD(SignMD) {
    assert(checkValidity());
  }

  unsigned getNumArguments() const {
    return MD->getNumOperands() - 1;
  }

  opencl::Type getArgument(unsigned I) const {
    assert(I < getNumArguments());
    return llvm::cast<llvm::MDNode>(MD->getOperand(I + 1).get());
  }

  bool operator==(const KernelSignature &S) const;
  bool operator!=(const KernelSignature &S) const { return !(*this == S); }

private:
  bool checkValidity() const;

private:
  llvm::MDNode *MD;
};

class KernelInfo {
public:
  KernelInfo() : Kernel(nullptr), MD(nullptr) {}

  KernelInfo(llvm::MDNode *KernMD) : Kernel(nullptr), MD(KernMD) {
    assert(checkValidity());
    CustomInfoMD = retrieveCustomInfo();
    Kernel = llvm::mdconst::extract<llvm::Function>(MD->getOperand(0));
  }

  KernelInfo(llvm::Function *Kernel) : Kernel(Kernel), MD(nullptr) {
    assert(checkValidity());
    CustomInfoMD = retrieveCustomInfo();
  }

  KernelInfo(const KernelInfo &KI)
    : Kernel(KI.Kernel),
      MD(KI.MD),
      CustomInfoMD(KI.CustomInfoMD) {}

  KernelInfo &operator=(const KernelInfo &KI) {
    MD = KI.MD;
    Kernel = KI.Kernel;
    CustomInfoMD = KI.CustomInfoMD;
    return *this;
  }

  KernelInfo &operator=(llvm::MDNode *KernMD) {
    MD = KernMD;
    assert(checkValidity());
    CustomInfoMD = retrieveCustomInfo();
    return *this;
  }

  KernelInfo &operator=(llvm::Function *Kernel) {
    this->Kernel = Kernel;
    assert(checkValidity());
    CustomInfoMD = retrieveCustomInfo();
    return *this;
  }

  llvm::MDNode *getKernelInfo() const { return MD; }

  llvm::Function *getFunction() const { return Kernel; }

  llvm::MDNode *getCustomInfo() const { return CustomInfoMD; }

  llvm::StringRef getName() const { return Kernel->getName(); }

  bool hasVectorTypeHint() const {
    // This metadata is added to the module only if its corresponding
    // __attribute_((vec_type_hint(<typen>))) has been specified for
    // the kernel.
    return getKernelArgInfo("vector_type_hint") ? true : false;
  }

  bool hasWorkGroupSizesHint() const {
    // This metadata is added to the module only if its corresponding
    // __attribute_((work_group_size_hint(X, Y, Z))) has been specified
    // for the kernel.
    return getKernelArgInfo("work_group_size_hint") ? true : false;
  }

  bool hasRequiredWorkGroupSizes() const {
    // This metadata is added to the module only if its corresponding
    // __attribute_((reqd_work_group_size(X, Y, Z))) has been specified
    // for the kernel.
    return getKernelArgInfo("reqd_work_group_size") ? true : false;
  }

  KernelSignature getSignature() const {
    return getCustomInfo("signature");
  }

  KernelArgInfo getVectorTypeHint() const {
    assert(hasVectorTypeHint());
    return getKernelArgInfo("vector_type_hint");
  }

  KernelArgInfo getWorkGroupSizesHint() const {
    assert(hasWorkGroupSizesHint());
    return getKernelArgInfo("work_group_size_hint");
  }

  KernelArgInfo getRequiredWorkGroupSizes() const {
    assert(hasRequiredWorkGroupSizes());
    return getKernelArgInfo("reqd_work_group_size");
  }

  KernelArgInfo getArgsAddrSpace() const {
    return getKernelArgInfo("kernel_arg_addr_space");
  }

  KernelArgInfo getArgsAccessQual() const {
    return getKernelArgInfo("kernel_arg_access_qual");
  }

  KernelArgInfo getArgsType() const {
    return getKernelArgInfo("kernel_arg_type");
  }

  KernelArgInfo getArgsTypeQual() const {
    return getKernelArgInfo("kernel_arg_type_qual");
  }

  KernelArgInfo getArgsName() const {
    return getKernelArgInfo("kernel_arg_name");
  }

  void updateCustomInfo(llvm::MDNode *CMD);

protected:
  llvm::MDNode *getCustomInfo(llvm::StringRef Name) const;

private:
  bool checkValidity() const;
  
  llvm::MDNode *retrieveCustomInfo() const;

  llvm::MDNode *getKernelArgInfo(llvm::StringRef Name) const;

private:
  // New SPIR format that uses kernel metaata.
  llvm::Function *Kernel;

  // Old SPIR format that uses generic metadata for kernels.
  llvm::MDNode *MD;

  llvm::MDNode *CustomInfoMD;
};

class ModuleInfo {
public:
  class iterator {
  public:
    typedef const KernelInfo value_type;
    typedef ptrdiff_t difference_type;
    typedef value_type *pointer;
    typedef value_type &reference;
    typedef std::forward_iterator_tag iterator_category;
  
  public:
    iterator(llvm::Module *Mod, bool Sentinel = false)
      : Mod(Mod),
        HasKernelsMD(Mod->getTargetTriple().find("spir") == 0),
        KernelsMD(nullptr) {
      assert(Mod && "No llvm::Module");
     
      if (HasKernelsMD) {
        KernelsMD = Mod->getNamedMetadata("opencl.kernels");
        assert(KernelsMD && "No opencl.kernels MDNode");

        CurIdx = KernelsMD && Sentinel ? KernelsMD->getNumOperands() : 0;
      } else {
        StartIt = Mod->begin();
        EndIt = Mod->end();

        if (Sentinel)
          CurIt = Mod->end();
        else
          CurIt = std::find_if(StartIt, EndIt, [](const llvm::Function &F) {
              return !F.isDeclaration() &&
                  (F.getCallingConv() == llvm::CallingConv::SPIR_KERNEL);
                });
      }

      updateKernelInfo();
    }

    bool operator==(const iterator &I) const { 
      if (HasKernelsMD) {
        return I.HasKernelsMD && Mod == I.Mod &&
          KernelsMD == I.KernelsMD && CurIdx == I.CurIdx;
      } else {
        return !I.HasKernelsMD && Mod == I.Mod && 
          StartIt == I.StartIt &&
          CurIt == I.CurIt &&
          EndIt == I.EndIt;
      }
    }

    bool operator!=(const iterator &I) const {
      return !(*this == I);
    }

    const KernelInfo &operator*() const {
      return CurInfo;
    }

    const KernelInfo *operator->() const {
      return &CurInfo;
    }

    iterator &operator++() {
      advance();
      return *this;
    }

    iterator operator++(int Ign) {
      iterator That = *this;
      ++*this;
      return That;
    }

  private:
    void updateKernelInfo() {
      if (HasKernelsMD) {
        if (KernelsMD && CurIdx < KernelsMD->getNumOperands())
          CurInfo = KernelsMD->getOperand(CurIdx);
      } else {
        if (Mod && CurIt != EndIt)
          CurInfo = &*CurIt;
      }
    }

    void advance() {
      if (HasKernelsMD) {
        if (KernelsMD && CurIdx < KernelsMD->getNumOperands())
          ++CurIdx;
      } else {
        if (Mod && CurIt != EndIt)
          CurIt = std::find_if(++CurIt, EndIt, [](const llvm::Function &F) {
              return !F.isDeclaration() &&
              (F.getCallingConv() == llvm::CallingConv::SPIR_KERNEL);
              });
      }

      updateKernelInfo();
    }

  private:
    llvm::Module *Mod;
    KernelInfo CurInfo;
    bool HasKernelsMD;

    // For old SPIR format with "opencl.kernels" MDNode listing all
    // kernel functions.
    llvm::NamedMDNode *KernelsMD;
    unsigned CurIdx;

    // For new SPIR format using kernel function metadata.
    llvm::Module::iterator StartIt, CurIt, EndIt;
  };

public:
  ModuleInfo(llvm::Module &M)
    : Mod(M), Has_SPIR_MDs(M.getTargetTriple().find("spir") == 0) {}

  // Check if the LLVM module is in the old SPIR format that uses generic
  // metadata instead of function metadata for kernels.
  bool IRisSPIR() const {
    return Has_SPIR_MDs;
  }

  iterator begin() const {
    return iterator(&Mod);
  }

  iterator end() const {
    return iterator(&Mod, true);
  }

  unsigned getNumKernels() const;

  iterator find(llvm::StringRef Name) const;

  bool contains(llvm::StringRef Name) const {
    return find(Name) != end();
  }

  KernelInfo get(llvm::StringRef Name) const {
    assert(contains(Name));
    return *find(Name);
  }

protected:
  llvm::Module &Mod;

private:
  bool Has_SPIR_MDs;
};

}

#endif
