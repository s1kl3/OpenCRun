#ifndef OCLBUILTINDEF_EMITTER_H
#define OCLBUILTINDEF_EMITTER_H

#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"

namespace opencrun {
  bool EmitOCLBuiltinDef(llvm::raw_ostream &OS, llvm::RecordKeeper &R);
  bool EmitOCLBuiltinDefTarget(llvm::raw_ostream &OS, llvm::RecordKeeper &R);
}

#endif
