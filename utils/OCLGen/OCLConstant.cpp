#include "OCLConstant.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"

#include <algorithm>
#include <map>

using namespace opencrun;

class opencrun::OCLConstantsTableImpl {
public:
  typedef std::map<llvm::Record *, const OCLConstant *> OCLConstantsMap;

public:
  ~OCLConstantsTableImpl() {
    llvm::DeleteContainerSeconds(Constants);
  }

public:
  const OCLConstant &get(llvm::Record &R) {
    if (!Constants.count(&R))
      BuildConstant(R);

    return *Constants[&R];
  }

private:
  void BuildConstant(llvm::Record &R) {
    OCLConstant *C = 0;

    if (R.isSubClassOf("OCLConstant")) {
      llvm::StringRef Name = R.getValueAsString("Name");
      llvm::StringRef Group = R.getValueAsString("Group");
      llvm::StringRef Value = R.getValueAsString("Value");
      bool IsTarget = R.getValueAsBit("isTarget");
      C = new OCLConstant(Name, Group, Value, IsTarget);
    }
    else
      llvm::PrintFatalError("Invalid OCLConstant: " + R.getName());

    Constants[&R] = C;
  }

private:
  OCLConstantsMap Constants;
};

llvm::OwningPtr<OCLConstantsTableImpl> OCLConstantsTable::Impl;

const OCLConstant &OCLConstantsTable::get(llvm::Record &R) {
  if (!Impl) Impl.reset(new OCLConstantsTableImpl());
  return Impl->get(R);
}

static bool CompareLess(const OCLConstant *C1, const OCLConstant *C2) {
  return (C1->getGroup() < C2->getGroup()) || 
         (C1->getGroup() == C2->getGroup() && C1->getName() == C2->getName());
}

void opencrun::LoadOCLConstants(const llvm::RecordKeeper &R, 
                                OCLConstantsContainer &T) {
  T.clear();

  std::vector<llvm::Record *> RawVects = 
    R.getAllDerivedDefinitions("OCLConstant");

  for(unsigned I = 0, E = RawVects.size(); I < E; ++I)
    T.push_back(&OCLConstantsTable::get(*RawVects[I]));

  std::sort(T.begin(), T.end(), CompareLess);
}
