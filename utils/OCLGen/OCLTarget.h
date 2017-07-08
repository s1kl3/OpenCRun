#ifndef OCLTARGET_H
#define OCLTARGET_H

#include "llvm/ADT/StringRef.h"
#include "llvm/TableGen/Record.h"

#include <set>
#include <vector>

namespace opencrun {

class OCLPredicate {
public:
  enum PredicateKind {
    PK_Extension,
    PK_AddressSpace,
    PK_Macro
  };

protected:
  OCLPredicate(PredicateKind K, llvm::StringRef P, llvm::StringRef N)
   : Kind(K), Prefix(P), Name(N), FullName(Prefix + Name) {}

public:
  PredicateKind getKind() const { return Kind; }
  llvm::StringRef getPrefix() const { return Prefix; }
  llvm::StringRef getName() const { return Name; }
  llvm::StringRef getFullName() const { return FullName; }

private:
  PredicateKind Kind;
  std::string Prefix;
  std::string Name;
  std::string FullName;
};

class OCLExtension : public OCLPredicate {
public:
  static bool classof(const OCLPredicate *P) {
    return P->getKind() == PK_Extension;
  }

  OCLExtension(llvm::StringRef Prefix, llvm::StringRef Name)
   : OCLPredicate(PK_Extension, Prefix, Name) {}
};

enum AddressSpaceKind {
  AS_Begin = 0,
  AS_Private = AS_Begin,
  AS_Global,
  AS_Local,
  AS_Constant,

  AS_End,
  AS_Unknown
};

class OCLAddressSpace : public OCLPredicate {
public:
  static bool classof(const OCLPredicate *P) {
    return P->getKind() == PK_AddressSpace;
  }

public:
  OCLAddressSpace(llvm::StringRef Prefix, llvm::StringRef Name, 
                  AddressSpaceKind AS)
   : OCLPredicate(PK_AddressSpace, Prefix, Name), AddressSpace(AS) {}

  AddressSpaceKind getAddressSpace() const { return AddressSpace; }

private:
  AddressSpaceKind AddressSpace;
};

class OCLMacro : public OCLPredicate {
public:
  static bool classof(const OCLPredicate *P) {
    return P->getKind() == PK_Macro;
  }

public:
  OCLMacro(llvm::StringRef Prefix, llvm::StringRef Name)
   : OCLPredicate(PK_Macro, Prefix, Name) {}
};

typedef std::set<const OCLPredicate*> PredicateSet;

//===----------------------------------------------------------------------===//
// OCLPredicates Loader
//===----------------------------------------------------------------------===//

typedef std::vector<const OCLPredicate *> OCLPredicatesContainer;
void LoadOCLPredicates(const llvm::RecordKeeper &R, OCLPredicatesContainer &P);

//===----------------------------------------------------------------------===//
// Predicates table singleton
//===----------------------------------------------------------------------===//

class OCLPredicatesTableImpl;

class OCLPredicatesTable {
public:
  static const OCLPredicate &get(llvm::Record &R);

  static const OCLAddressSpace *getAddressSpace(AddressSpaceKind K);

private:
  static std::unique_ptr<OCLPredicatesTableImpl> Impl;

  friend void LoadOCLPredicates(const llvm::RecordKeeper &R,
                                OCLPredicatesContainer &P);
};

}

#endif
