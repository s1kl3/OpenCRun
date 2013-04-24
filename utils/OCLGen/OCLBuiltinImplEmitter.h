#ifndef OCLBUILTINIMPL_EMITTER_H
#define OCLBUILTINIMPL_EMITTER_H

#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"

namespace opencrun {
  bool EmitOCLBuiltinImpl(llvm::raw_ostream &OS, llvm::RecordKeeper &R);
}

#endif
