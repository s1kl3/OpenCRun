#ifndef OCLTYPEDEF_EMITTER_H
#define OCLTYPEDEF_EMITTER_H

#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"

namespace opencrun {
  bool EmitOCLTypeDef(llvm::raw_ostream &OS, llvm::RecordKeeper &R);
}

#endif
