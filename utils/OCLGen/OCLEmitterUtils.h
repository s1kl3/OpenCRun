#ifndef OCLBUILTIN_EMITTER_UTILS_H
#define OCLBUILTIN_EMITTER_UTILS_H

#include "OCLBuiltin.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

namespace opencrun {

class OCLType;

const char *AddressSpaceName(OCLPointerType::AddressSpace AS);

void EmitOCLTypeSignature(llvm::raw_ostream &OS, const OCLType &T,
                          std::string Name = "");

void ComputeRequiredExt(const BuiltinSignature &S, llvm::BitVector &Req);

void EmitRequiredExtBegin(llvm::raw_ostream &OS, const llvm::BitVector &Req);
void EmitRequiredExtEnd(llvm::raw_ostream &OS, const llvm::BitVector &Req);

void EmitBuiltinGroupBegin(llvm::raw_ostream &OS, llvm::StringRef Group);
void EmitBuiltinGroupEnd(llvm::raw_ostream &OS, llvm::StringRef Group);

bool IsScalarAlternative(BuiltinSignature &Sign);

}

#endif
