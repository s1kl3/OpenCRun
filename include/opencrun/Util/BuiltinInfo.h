#ifndef OPENCRUN_UTIL_BUILTININFO_H
#define OPENCRUN_UTIL_BUILTININFO_H

#include "llvm/ADT/StringRef.h"

namespace llvm {

class Function;
class FunctionType;
class Module;

}

namespace opencrun {

class Device;

class DeviceBuiltinInfo {
public:
  static llvm::Function *getPrototype(llvm::Module &Mod,
                                      llvm::StringRef Name,
                                      llvm::StringRef Format);
};

}

#endif
