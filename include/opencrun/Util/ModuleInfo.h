#ifndef OPENCRUN_UTIL_MODULEINFO_H
#define OPENCRUN_UTIL_MODULEINFO_H

#include "opencrun/Util/OpenCLTypeSystem.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

namespace opencrun {

class KernelArgInfo {
public:
  KernelArgInfo(llvm::MDNode *InfoMD) : MD(InfoMD) {
    assert(checkValidity());
  }

  unsigned getNumArguments() const {
    return MD->getNumOperands() - 1;
  }

  llvm::Metadata *getArgument(unsigned I) const {
    assert(I < getNumArguments());
    return MD->getOperand(I + 1).get();
  }

  template<typename Ty>
  Ty *getArgumentAs(unsigned I) const {
    return llvm::mdconst::extract<Ty>(getArgument(I));
  }

  llvm::StringRef getArgumentAsString(unsigned I) const {
    return llvm::cast<llvm::MDString>(getArgument(I))->getString();
  }

private:
  bool checkValidity() const;

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
  KernelInfo(llvm::MDNode *KernMD = 0) : MD(KernMD) {
    assert(checkValidity());
    CustomInfoMD = retrieveCustomInfo();
  }
  KernelInfo(const KernelInfo &I) : MD(I.MD), CustomInfoMD(I.CustomInfoMD) {}
  KernelInfo &operator=(const KernelInfo &I) {
    MD = I.MD;
    CustomInfoMD = I.CustomInfoMD;
    return *this;
  }
  KernelInfo &operator=(llvm::MDNode *KernMD) {
    MD = KernMD;
    assert(checkValidity());
    CustomInfoMD = retrieveCustomInfo();
    return *this;
  }

  llvm::Function *getFunction() const {
    assert(MD);
    return llvm::mdconst::extract<llvm::Function>(MD->getOperand(0));
  }

  llvm::StringRef getName() const {
    return getFunction()->getName();
  }

  bool hasRequiredWorkGroupSizes() const {
    // This metadata is added to the module only if its corresponding
    // __attribute_ has been specified for the kernel.
    return getKernelArgInfo("reqd_work_group_size") ? true : false;
  }

  KernelSignature getSignature() const {
    return getCustomInfo("signature");
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

  llvm::MDNode *getKernelInfo() const { return MD; }
  llvm::MDNode *getCustomInfo() const { return CustomInfoMD; }

protected:
  llvm::MDNode *getCustomInfo(llvm::StringRef Name) const;

private:
  bool checkValidity() const;
  llvm::MDNode *retrieveCustomInfo() const;

  llvm::MDNode *getKernelArgInfo(llvm::StringRef Name) const;

private:
  llvm::MDNode *MD;
  llvm::MDNode *CustomInfoMD;
};

class ModuleInfo {
public:
  class kernel_info_iterator {
  public:
    typedef const KernelInfo value_type;
    typedef ptrdiff_t difference_type;
    typedef value_type *pointer;
    typedef value_type &reference;
    typedef std::forward_iterator_tag iterator_category;
  
  public:
    kernel_info_iterator(llvm::NamedMDNode *Kernels, bool Sentinel = false)
     : KernelsMD(Kernels),
       CurIdx(Kernels && Sentinel ? Kernels->getNumOperands() : 0) {
      updateKernelInfo();
    }

    bool operator==(const kernel_info_iterator &I) const {
      return KernelsMD == I.KernelsMD && CurIdx == I.CurIdx;
    }
    bool operator!=(const kernel_info_iterator &I) const {
      return !(*this == I);
    }

    const KernelInfo &operator*() const {
      return CurInfo;
    }

    const KernelInfo *operator->() const {
      return &CurInfo;
    }

    kernel_info_iterator &operator++() {
      advance();
      return *this;
    }

    kernel_info_iterator operator++(int Ign) {
      kernel_info_iterator That = *this;
      ++*this;
      return That;
    }

  private:
    void updateKernelInfo() {
      if (KernelsMD && CurIdx < KernelsMD->getNumOperands())
        CurInfo = KernelsMD->getOperand(CurIdx);
    }

    void advance() {
      if (KernelsMD && CurIdx < KernelsMD->getNumOperands())
        ++CurIdx;
      updateKernelInfo();
    }

  private:
    llvm::NamedMDNode *KernelsMD;
    unsigned CurIdx;
    KernelInfo CurInfo;
  };

public:
  ModuleInfo(llvm::Module &M) : Mod(M) {}

  kernel_info_iterator kernel_info_begin() const {
    return kernel_info_iterator(Mod.getNamedMetadata("opencl.kernels"));
  }

  kernel_info_iterator kernel_info_end() const {
    return kernel_info_iterator(Mod.getNamedMetadata("opencl.kernels"), true);
  }

  kernel_info_iterator findKernel(llvm::StringRef Name) const;

  bool hasKernel(llvm::StringRef Name) const {
    return findKernel(Name) != kernel_info_end();
  }

  KernelInfo getKernelInfo(llvm::StringRef Name) const {
    kernel_info_iterator I = findKernel(Name);
    assert(I != kernel_info_end());
    return *I;
  }

private:
  llvm::Module &Mod;
};

}

#endif
