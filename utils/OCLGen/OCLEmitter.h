#ifndef OCLEMITTER_H
#define OCLEMITTER_H

#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"

namespace opencrun {
  bool EmitOCLBuiltinDefs(llvm::raw_ostream &OS, llvm::RecordKeeper &R);
  bool EmitOCLBuiltinImpls(llvm::raw_ostream &OS, llvm::RecordKeeper &R);
  bool EmitOCLConstantDefs(llvm::raw_ostream &OS, llvm::RecordKeeper &R);
  bool EmitOCLConstantDefsTarget(llvm::raw_ostream &OS, llvm::RecordKeeper &R);
  bool EmitOCLTargetDefs(llvm::raw_ostream &OS, llvm::RecordKeeper &R);
  bool EmitOCLTypeDefs(llvm::raw_ostream &OS, llvm::RecordKeeper &R);
  bool EmitOCLTypeDefsTarget(llvm::raw_ostream &OS, llvm::RecordKeeper &R);
}

#endif
