#ifndef OCLTARGET_H
#define OCLTARGET_H

#include "llvm/ADT/StringRef.h"

namespace opencrun {

enum OCLPredicate {
  Pred_InitValue,

  // Address Space predicates
  Pred_AS_InitValue = Pred_InitValue,
  Pred_AS_Private = Pred_AS_InitValue,
  Pred_AS_Global,
  Pred_AS_Local,
  Pred_AS_Constant,
  Pred_AS_MaxValue,

  // Extension predicates
  Pred_Ext_InitValue = Pred_AS_MaxValue,
  Pred_Ext_cl_khr_fp16 = Pred_Ext_InitValue,
  Pred_Ext_cl_khr_fp64,
  Pred_Ext_MaxValue,

  Pred_MaxValue = Pred_Ext_MaxValue
};

bool IsAddressSpacePredicate(unsigned P);

bool IsExtensionPredicate(unsigned P);

const char *PredicateName(unsigned P);

OCLPredicate ParsePredicateName(llvm::StringRef Name);

}

#endif
