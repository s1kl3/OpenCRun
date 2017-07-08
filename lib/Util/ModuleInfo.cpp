#include "opencrun/Util/ModuleInfo.h"
#include "llvm/IR/Constants.h"

using namespace opencrun;

bool KernelArgInfo::checkValidity() const {
  if (llvm::MDString *S = llvm::dyn_cast<llvm::MDString>(MD->getOperand(0)))
    return S->getString().startswith("kernel_arg_") ||
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
  return !MD || llvm::isa<llvm::Function>(MD->getOperand(0));
}

llvm::MDNode *KernelInfo::retrieveCustomInfo() const {
  if (!MD) return 0;

  for (unsigned I = 1, E = MD->getNumOperands(); I != E; ++I)
    if (llvm::MDNode *N = llvm::dyn_cast<llvm::MDNode>(MD->getOperand(I)))
      if (llvm::MDString *S = llvm::dyn_cast<llvm::MDString>(N->getOperand(0)))
        if (S->getString() == "custom_info")
          return N;
  return 0;
}

llvm::MDNode *KernelInfo::getCustomInfo(llvm::StringRef Name) const {
  assert(MD && CustomInfoMD);

  for (unsigned I = 1, E = CustomInfoMD->getNumOperands(); I != E; ++I) {
    llvm::MDNode *N = llvm::cast<llvm::MDNode>(CustomInfoMD->getOperand(I));
    llvm::MDString *S = llvm::cast<llvm::MDString>(N->getOperand(0));
    if (S->getString() == Name)
      return N;
  }
  return 0;
}

llvm::MDNode *KernelInfo::getKernelArgInfo(llvm::StringRef Name) const {
  assert(MD);
  if (Name.startswith("kernel_arg_") || Name.equals("reqd_work_group_size"))
    for (unsigned I = 1, E = MD->getNumOperands(); I != E; ++I) {
      llvm::MDNode *N = llvm::cast<llvm::MDNode>(MD->getOperand(I));
      llvm::MDString *S = llvm::cast<llvm::MDString>(N->getOperand(0));
      if (S->getString() == Name)
        return N;
    }
  return 0;
}

ModuleInfo::kernel_info_iterator
ModuleInfo::findKernel(llvm::StringRef Name) const {
  kernel_info_iterator I = kernel_info_begin(),
                       E = kernel_info_end();
  for (; I != E; ++I)
    if (I->getName() == Name)
      return I;
  return E;
}
