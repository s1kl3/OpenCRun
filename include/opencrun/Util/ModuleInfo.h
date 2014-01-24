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

  llvm::Value *getArgument(unsigned I) const {
    assert(I < getNumArguments());
    return MD->getOperand(I + 1);
  }

  template<typename Ty>
  Ty *getArgumentAs(unsigned I) const {
    return llvm::cast<Ty>(getArgument(I));
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
    return llvm::cast<llvm::MDNode>(MD->getOperand(I + 1));
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
    InfoMD = retrieveKernelInfo();
  }
  KernelInfo(const KernelInfo &I) : MD(I.MD), InfoMD(I.InfoMD) {}
  KernelInfo &operator=(const KernelInfo &I) {
    MD = I.MD;
    InfoMD = I.InfoMD;
    return *this;
  }
  KernelInfo &operator=(llvm::MDNode *KernMD) {
    MD = KernMD;
    assert(checkValidity());
    InfoMD = retrieveKernelInfo();
    return *this;
  }

  llvm::Function *getFunction() const {
    assert(MD);
    return llvm::cast<llvm::Function>(MD->getOperand(0));
  }

  llvm::StringRef getName() const {
    return getFunction()->getName();
  }

  KernelSignature getSignature() const {
    return getKernelInfo("signature");
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

  llvm::MDNode *getInfo() const { return InfoMD; }

  uint64_t getStaticLocalSize() const;

private:
  bool checkValidity() const;
  llvm::MDNode *retrieveKernelInfo() const;

  llvm::MDNode *getKernelArgInfo(llvm::StringRef Name) const;
  llvm::MDNode *getKernelInfo(llvm::StringRef Name) const;

private:
  llvm::MDNode *MD;
  llvm::MDNode *InfoMD;
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
        CurInfo = llvm::cast<llvm::MDNode>(KernelsMD->getOperand(CurIdx));
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
