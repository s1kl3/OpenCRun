#include "OCLTarget.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"

using namespace opencrun;

class opencrun::OCLPredicatesTableImpl {
public:
  typedef std::map<llvm::Record *, const OCLPredicate *> OCLPredicatesMap;
  typedef std::map<AddressSpaceKind, const OCLAddressSpace*> OCLAddrSpaceMap;

public:
  ~OCLPredicatesTableImpl() {
    llvm::DeleteContainerSeconds(Predicates);
  }

public:
  const OCLPredicate &get(llvm::Record &R) {
    if (!Predicates.count(&R))
      BuildPredicate(R);

    return *Predicates[&R];
  }

  const OCLAddressSpace *getAddressSpace(AddressSpaceKind K) {
    if (!AddrSpaces.count(K)) return 0;

    return AddrSpaces[K];
  }

private:
  void BuildPredicate(llvm::Record &R) {
    OCLPredicate *P = 0;

    if (R.isSubClassOf("OCLPredicate")) {
      llvm::StringRef Prefix = R.getValueAsString("Prefix");
      llvm::StringRef Name = R.getValueAsString("Name");

      if (R.isSubClassOf("OCLExtension")) {
        P = new OCLExtension(Prefix, Name);
      } else if (R.isSubClassOf("OCLMacro")) { 
        P = new OCLMacro(Prefix, Name);
      } else if (R.isSubClassOf("OCLAddressSpace")) {
        AddressSpaceKind AS = 
          llvm::StringSwitch<AddressSpaceKind>(Name)
            .Case("private", AS_Private)
            .Case("global", AS_Global)
            .Case("local", AS_Local)
            .Case("constant", AS_Constant)
            .Default(AS_Unknown);

        if (AS == AS_Unknown)
          llvm::PrintFatalError("Illegal address space!");

        OCLAddressSpace *PAS = new OCLAddressSpace(Prefix, Name, AS);
        AddrSpaces[AS] = PAS;
        P = PAS;
      } else
        llvm::PrintFatalError("Unknown predicate kind: " + R.getName());
    }
    else
      llvm::PrintFatalError("Unknown predicate: " + R.getName());

    Predicates[&R] = P;
  }

private:
  OCLPredicatesMap Predicates;
  OCLAddrSpaceMap AddrSpaces;
};

std::unique_ptr<OCLPredicatesTableImpl> OCLPredicatesTable::Impl;

const OCLPredicate &OCLPredicatesTable::get(llvm::Record &R) {
  if (!Impl) Impl.reset(new OCLPredicatesTableImpl());
  return Impl->get(R);
}

const OCLAddressSpace *OCLPredicatesTable::getAddressSpace(AddressSpaceKind K) {
  if (!Impl) Impl.reset(new OCLPredicatesTableImpl());
  return Impl->getAddressSpace(K);
}

void opencrun::LoadOCLPredicates(const llvm::RecordKeeper &R, 
                                 OCLPredicatesContainer &P) {
  P.clear();

  std::vector<llvm::Record *> RawVects = 
    R.getAllDerivedDefinitions("OCLPredicate");

  for(unsigned I = 0, E = RawVects.size(); I < E; ++I)
    P.push_back(&OCLPredicatesTable::get(*RawVects[I]));
}
