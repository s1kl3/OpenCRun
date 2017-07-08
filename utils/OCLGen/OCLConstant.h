#ifndef  OCLCONSTANT_H
#define  OCLCONSTANT_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"

namespace opencrun {

//===----------------------------------------------------------------------===//
// OCLConstant
//===----------------------------------------------------------------------===//

class OCLConstant {
public:
  OCLConstant(llvm::StringRef name, llvm::StringRef group, 
              llvm::StringRef value, bool isTarget)
   : Name(name), Group(group), Value(value), IsTarget(isTarget) {}

  llvm::StringRef getName() const { return Name; }
  llvm::StringRef getGroup() const { return Group; }
  llvm::StringRef getValue() const { return Value; }
  bool isTarget() const { return IsTarget; }
private:
  std::string Name;
  std::string Group;
  std::string Value;
  bool IsTarget;
};

//===----------------------------------------------------------------------===//
// OCLConstants Loader
//===----------------------------------------------------------------------===//

typedef std::vector<const OCLConstant *> OCLConstantsContainer;
void LoadOCLConstants(const llvm::RecordKeeper &R, OCLConstantsContainer &T);

//===----------------------------------------------------------------------===//
// Constants table singleton
//===----------------------------------------------------------------------===//

class OCLConstantsTableImpl;

class OCLConstantsTable {
public:
  static const OCLConstant &get(llvm::Record &R);

private:
  static std::unique_ptr<OCLConstantsTableImpl> Impl;
};

}

#endif
