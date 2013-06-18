#include "OCLTarget.h"

#include "llvm/ADT/StringSwitch.h"

using namespace opencrun;

bool opencrun::IsAddressSpacePredicate(unsigned P) {
  switch (P) {
  default: return false;
  case Pred_AS_Global:
  case Pred_AS_Local:
  case Pred_AS_Constant:
    return true;
  }
}

bool opencrun::IsExtensionPredicate(unsigned P) {
  switch (P) {
  default: return false;
  case Pred_Ext_cl_khr_fp16:
  case Pred_Ext_cl_khr_fp64:
    return true;
  }
}

const char *opencrun::PredicateName(unsigned P) {
  switch (P) {
  case Pred_Ext_cl_khr_fp16: return "cl_khr_fp16";
  case Pred_Ext_cl_khr_fp64: return "cl_khr_fp64";
  case Pred_AS_Global: return "addrspace_global";
  case Pred_AS_Local: return "addrspace_local";
  case Pred_AS_Constant: return "addrspace_constant";
  default: break;
  }
  return 0;
}

OCLPredicate opencrun::ParsePredicateName(llvm::StringRef Name) {
  return llvm::StringSwitch<OCLPredicate>(Name)
          .Case("ocl_ext_cl_khr_fp16", Pred_Ext_cl_khr_fp16)
          .Case("ocl_ext_cl_khr_fp64", Pred_Ext_cl_khr_fp64)
          .Case("ocl_as_private", Pred_AS_Private)
          .Case("ocl_as_global", Pred_AS_Global)
          .Case("ocl_as_local", Pred_AS_Local)
          .Case("ocl_as_constant", Pred_AS_Constant)
          .Default(Pred_MaxValue);
}
