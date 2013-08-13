#ifndef OCLBUILTIN_EMITTER_UTILS_H
#define OCLBUILTIN_EMITTER_UTILS_H

#include "OCLBuiltin.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include <iterator>
#include <string>

namespace opencrun {

class OCLType;

const char *AddressSpaceQualifier(AddressSpaceKind AS);

PredicateSet ComputePredicates(const OCLBuiltin &B, const BuiltinSign &S, 
                               bool IgnoreAS = false);

void EmitOCLTypeSignature(llvm::raw_ostream &OS, const OCLType &T,
                          std::string Name = "");

template<typename K, typename V>
class MapKeyIterator {
  typedef typename std::map<K, V>::const_iterator base_iterator;

public:
  MapKeyIterator(base_iterator I) : Iter(I) {}
  MapKeyIterator(const MapKeyIterator &I) : Iter(I.Iter) {}

  const K &operator*() const {
    return Iter->first;
  }

  const K *operator->() const {
    return &(Iter->first);
  }

  MapKeyIterator &operator++() {
    ++Iter;
    return *this;
  }

  MapKeyIterator &operator--() {
    --Iter;
    return *this;
  }

  bool operator==(const MapKeyIterator &I) const {
    return Iter == I.Iter;
  }

  bool operator!=(const MapKeyIterator &I) const {
    return !(*this == I);
  }

private:
  base_iterator Iter;
};

template<typename IterTy>
void ComputeSignsRanges(IterTy Begin, IterTy End, 
                        std::list<std::pair<unsigned, IterTy> > &R) {
  unsigned Count = 0;
  R.push_back(std::make_pair(Count, Begin));
  for (IterTy I = Begin; I != End; ++I, ++Count)
    if ((*I)[0] != (*Begin)[0]) {
      R.push_back(std::make_pair(Count, I));
      Begin = I;
      Count = 0;
    }
  R.push_back(std::make_pair(Count, End));
}

struct OCLBuiltinDecorator {
  const OCLBuiltin &Builtin;

  OCLBuiltinDecorator(const OCLBuiltin &B) : Builtin(B) {}

  std::string getExternalName(const BuiltinSign *S = 0, bool NoRound = 0) const;
  std::string getInternalName(const BuiltinSign *S = 0, bool NoRound = 0) const;
};

template<typename IteratorTy>
unsigned CountSignaturesWithSameRetTy(IteratorTy I, IteratorTy E) {
  if (I == E) return 0;

  const OCLBasicType *RetTy = (*I)[0];

  unsigned Count = 0;

  for (; I != E && (*I)[0] == RetTy; ++Count, ++I);

  return Count;
}

class GroupGuardEmitter {
  llvm::StringRef Old;
  llvm::StringRef Current;
  llvm::raw_ostream &OS;

public:
  GroupGuardEmitter(llvm::raw_ostream &os) : OS(os) {}
  bool Push(llvm::StringRef G);
  void Finalize();
};

class PredicatesGuardEmitter {
  PredicateSet Old;
  PredicateSet Current;
  llvm::raw_ostream &OS;

public:
  PredicatesGuardEmitter(llvm::raw_ostream &os) : OS(os) {}
  bool Push(const PredicateSet &S);
  void Finalize();
};

}

#endif
