#include "opencrun/Util/ModuleInfo.h"
#include "llvm/IR/Constants.h"

using namespace opencrun;

bool KernelArgInfo::IsGenericMD() const {
  if (llvm::MDString *S = llvm::dyn_cast<llvm::MDString>(MD->getOperand(0)))
    return S->getString().startswith("kernel_arg_") ||
      S->getString().equals("vec_type_hint") ||
      S->getString().equals("work_group_size_hint") ||
      S->getString().equals("reqd_work_group_size");
  return false;
}

bool KernelSignature::checkValidity() const {
  if (llvm::MDString *S = llvm::dyn_cast<llvm::MDString>(MD->getOperand(0)))
    return S->getString().startswith("signature");
  return false;
}

bool KernelSignature::operator==(const KernelSignature &S) const {
  if (getNumArguments() != S.getNumArguments())
    return false;

  opencl::TypeComparator C;
  for (unsigned I = 0, E = getNumArguments(); I != E; ++I)
    if (!C.equals(getArgument(I), S.getArgument(I)))
      return false;
  return true;
}

bool KernelInfo::checkValidity() const {
  if (MD) return Kernel;

  return !Kernel || (!Kernel->isDeclaration() &&
    Kernel->getCallingConv() == llvm::CallingConv::SPIR_KERNEL);
}

llvm::MDNode *KernelInfo::getCustomInfo(llvm::StringRef Name) const {
  assert(CustomInfoMD);

  unsigned StartIdx = 0;
  // For old SPIR format the first argument of the MDNode is represented
  // by the "custom_info" string itself.
  if (MD) StartIdx = 1;

  for (unsigned I = StartIdx, E = CustomInfoMD->getNumOperands(); I != E; ++I) {
    llvm::MDNode *N = llvm::cast<llvm::MDNode>(CustomInfoMD->getOperand(I));
    llvm::MDString *S = llvm::cast<llvm::MDString>(N->getOperand(0));
    if (S->getString() == Name)
      return N;
  }
  return nullptr;
}

llvm::MDNode *KernelInfo::retrieveCustomInfo() const  {
  if (MD) {
    for (unsigned I = 1, E = MD->getNumOperands(); I != E; ++I)
      if (llvm::MDNode *N = llvm::dyn_cast<llvm::MDNode>(MD->getOperand(I)))
        if (llvm::MDString *S = llvm::dyn_cast<llvm::MDString>(N->getOperand(0)))
          if (S->getString() == "custom_info")
            return N;
    return nullptr;
  }

  return Kernel ? Kernel->getMetadata("custom_info") : nullptr;
}

llvm::MDNode *KernelInfo::getKernelArgInfo(llvm::StringRef Name) const {
  if (MD) {
    if (Name.startswith("kernel_arg_") ||
        Name.equals("vec_type_hint") ||
        Name.equals("work_group_size_hint") ||
        Name.equals("reqd_work_group_size"))
      for (unsigned I = 1, E = MD->getNumOperands(); I != E; ++I) {
        llvm::MDNode *N = llvm::cast<llvm::MDNode>(MD->getOperand(I));
        llvm::MDString *S = llvm::cast<llvm::MDString>(N->getOperand(0));
        if (S->getString() == Name)
          return N;
      }
    return nullptr;
  }
  
  return Kernel->getMetadata(Name);
}

void KernelInfo::updateCustomInfo(llvm::MDNode *CMD) {
  assert(CMD);

  if (MD) {
    unsigned Idx = 0;
    for (; Idx < MD->getNumOperands(); ++Idx)
      if (MD->getOperand(Idx).get() == CustomInfoMD)
        break;

    assert(Idx < MD->getNumOperands());
    MD->replaceOperandWith(Idx, CMD);
  } else
    Kernel->setMetadata("custom_info", CMD);

  CustomInfoMD = CMD;
}

unsigned ModuleInfo::getNumKernels() const {
  unsigned Num = 0;

  if (IRisSPIR()) {
    auto *Kernels = Mod.getNamedMetadata("opencl.kernels"); 
    assert(Kernels && "No 'opencl.kernels' metadata!");

    Num = Kernels->getNumOperands();
  } else {
    for (auto &F : Mod)
      if (!F.isDeclaration() &&
          (F.getCallingConv() == llvm::CallingConv::SPIR_KERNEL))
        Num++;
  }

  return Num;
}

ModuleInfo::iterator ModuleInfo::find(llvm::StringRef Name) const {
  for (auto I = begin(), E = end(); I != E; ++I)
    if (I->getName() == Name)
      return I;
  return end();
}
