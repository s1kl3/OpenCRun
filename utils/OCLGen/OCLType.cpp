#include "OCLType.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Casting.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"

#include <algorithm>
#include <map>
#include <set>
#include <utility>

using namespace opencrun;

OCLType::~OCLType() {
}

std::string OCLScalarType::BuildName(llvm::StringRef Name) {
  if (!Name.startswith("ocl_"))
    llvm::PrintFatalError("Scalar type names must begin with 'ocl_': " +
                          Name.str());

  llvm::StringRef StrippedName = Name.substr(4);
  if (StrippedName.empty())
    llvm::PrintFatalError("Scalar type names must have a non-empty suffix: " +
                          Name.str());

  return StrippedName;
}

std::string OCLVectorType::BuildName(const OCLScalarType &Base, 
                                     unsigned Width) {
  return (Base.getName() + llvm::Twine(Width)).str();
}

std::string OCLPointerType::BuildName(const OCLType &Base, 
                                      AddressSpaceKind AS,
                                      unsigned Modifiers) {
  return ("@ptr<as:" + llvm::Twine(AS) +", m:" + 
         llvm::Twine(Modifiers) + ">{ " + Base.getName() + " }").str();
}

bool OCLIntegerType::compareLess(const OCLType *T) const {
  if (const OCLIntegerType *I = llvm::dyn_cast<OCLIntegerType>(T))
    return (isSigned() && I->isUnsigned()) ||
           (isSigned() && isSigned() == I->isSigned() && 
            getBitWidth() < I->getBitWidth()) || 
           (isUnsigned() && isUnsigned() == I->isUnsigned() && 
            getBitWidth() < I->getBitWidth());
  if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(T))
    return this == &V->getBaseType() || compareLess(&V->getBaseType());

  // Integer < !Integer
  return true;
}

bool OCLRealType::compareLess(const OCLType *T) const {
  if (llvm::isa<OCLIntegerType>(T)) return false;
  if (const OCLRealType *R = llvm::dyn_cast<OCLRealType>(T))
    return getBitWidth() < R->getBitWidth();
  if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(T))
    return this == &V->getBaseType() || compareLess(&V->getBaseType());

  // Real < !Integer && !Real
  return true;
}

bool OCLVectorType::compareLess(const OCLType *T) const {
  if (llvm::isa<OCLScalarType>(T))
    return getBaseType().compareLess(T);
  if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(T))
    return getBaseType().compareLess(&(V->getBaseType())) ||
           (&getBaseType() == &V->getBaseType() && getWidth() < V->getWidth());

  // Vector < !Scalar && !Opaque && !Vector
  return true;
}

bool OCLOpaqueType::compareLess(const OCLType *T) const {
  if (llvm::isa<OCLScalarType>(T) || 
      llvm::isa<OCLVectorType>(T)) return false;
  if (const OCLOpaqueType *O = llvm::dyn_cast<OCLOpaqueType>(T))
    return getName() < O->getName();

  // Opaque < !Scalar && !Opaque
  return true;
}

bool OCLPointerType::compareLess(const OCLType *T) const {
  if (llvm::isa<OCLScalarType>(T) ||
      llvm::isa<OCLOpaqueType>(T) ||
      llvm::isa<OCLVectorType>(T)) return false;
  if (const OCLPointerType *P = llvm::dyn_cast<OCLPointerType>(T))
    return getBaseType().compareLess(&(P->getBaseType())) ||
           (&getBaseType() == &P->getBaseType() && 
            getAddressSpace() < P->getAddressSpace());

  // Pointer < !Scalar && !Opaque && !Vector && !Pointer
  return true;
}

bool OCLGroupType::compareLess(const OCLType *T) const {
  if (const OCLGroupType *G = llvm::dyn_cast<OCLGroupType>(T))
    return Elements < G->Elements;

  return false;
}

//===----------------------------------------------------------------------===//
// Types table implementation
//===----------------------------------------------------------------------===//

class opencrun::OCLTypesTableImpl {
public:
  typedef std::map<llvm::Record *, const OCLType *> OCLTypesMap;
  typedef std::map<const OCLPointerType *, 
                   std::map<OCLGroupType::const_iterator, 
                            const OCLPointerType *> > OCLGroupPointersMap;
  typedef std::pair<const OCLScalarType *, unsigned> OCLVectorId;
  typedef std::map<OCLVectorId, const OCLVectorType *> OCLVectorsMap;
  typedef std::pair<const OCLBasicType *, OCLPtrStructure> OCLPointerId;
  typedef std::map<OCLPointerId, const OCLPointerType *> OCLPointersMap;
  typedef std::map<llvm::Record*, const OCLOpaqueTypeDef*> OCLOpaqueTypeDefsMap;

public:
  ~OCLTypesTableImpl() {
    llvm::DeleteContainerSeconds(Types);
    llvm::DeleteContainerSeconds(Ptrs);
  }

public:
  const OCLType &get(llvm::Record &R) {
    if (!Types.count(&R))
      BuildType(R);

    return *Types[&R];
  }

  const OCLPointerType &get(const OCLPointerGroupIterator &I) {
    if (!GroupPtrs.count(&I.Ptr) || !GroupPtrs[&I.Ptr].count(I.Iter))
      ExpandPointerType(I);

    return *GroupPtrs[&I.Ptr][I.Iter];
  }

  const OCLVectorType *getVectorType(const OCLScalarType &Base,
                                     unsigned Width) const {
    OCLVectorsMap::const_iterator I = Vects.find(OCLVectorId(&Base, Width));
    if (I != Vects.end()) return I->second;
    return 0;
  }

  const OCLPointerType &getPointerType(const OCLBasicType &Base,
                                       const OCLPtrStructure &PtrS) {
    if (!Ptrs.count(OCLPointerId(&Base, PtrS)))
      BuildPointerType(Base, PtrS);

    return *Ptrs[OCLPointerId(&Base, PtrS)];
  }

  const OCLOpaqueTypeDef &getOpaqueTypeDef(llvm::Record &R) {
    if (!OpaqueTypeDefs.count(&R))
      BuildOpaqueTypeDef(R);

    return *OpaqueTypeDefs[&R];
  }

private:
  PredicateSet FetchPredicates(llvm::Record &R) {
    PredicateSet Preds;
    std::vector<llvm::Record*> Predicates =
      R.getValueAsListOfDefs("Predicates");

    for (unsigned i = 0, e = Predicates.size(); i != e; ++i) {
      const OCLPredicate &P = OCLPredicatesTable::get(*Predicates[i]);

      if (!llvm::isa<OCLAddressSpace>(&P))
        Preds.insert(&P);
    }
    return Preds;
  }

  void BuildType(llvm::Record &R) {
    OCLType *Type = 0;

    if (R.isSubClassOf("OCLOpaqueType")) {
      auto Name = R.getValueAsString("Name");
      Type = new OCLOpaqueType(Name);
    }
    else if (R.isSubClassOf("OCLIntegerType")) {
      unsigned BitWidth = R.getValueAsInt("BitWidth");
      bool Unsigned = R.getValueAsBit("Unsigned");
      Type = new OCLIntegerType(R.getName(), BitWidth, Unsigned);
    }
    else if (R.isSubClassOf("OCLRealType")) {
      unsigned BitWidth = R.getValueAsInt("BitWidth");
      Type = new OCLRealType(R.getName(), BitWidth);
    }
    else if (R.isSubClassOf("OCLVectorType")) {
      unsigned Width = R.getValueAsInt("Width");
      const OCLType &T = get(*R.getValueAsDef("BaseType"));
      const OCLScalarType &Base = *llvm::cast<OCLScalarType>(&T);
      
      OCLVectorType *V = new OCLVectorType(Base, Width);

      OCLVectorId K(&Base, Width);
      if (Vects.find(K) == Vects.end()) Vects[K] = V;

      Type = V;
    }
    else if (R.isSubClassOf("OCLPointerType")) {
      llvm::Record *ASRecord = R.getValueAsDef("AddressSpace");

      const OCLAddressSpace *AS = 
        llvm::cast<OCLAddressSpace>(&OCLPredicatesTable::get(*ASRecord));

      const OCLType &Base = get(*R.getValueAsDef("BaseType"));
	  
	  std::vector<llvm::Record *> Modifiers =
		R.getValueAsListOfDefs("Modifiers");
	  unsigned mods = 0;
	  for (unsigned i = 0, e = Modifiers.size(); i != e; ++i) {
		    llvm::StringRef ModName = Modifiers[i]->getName();
        mods |= 
          llvm::StringSwitch<unsigned>(ModName)
            .Case("ocl_mod_const", OCLPointerType::M_Const)
            .Case("ocl_mod_restrict", OCLPointerType::M_Restrict)
            .Case("ocl_mod_volatile", OCLPointerType::M_Volatile)
            .Default(OCLPointerType::M_Unknown);

        if (mods == OCLPointerType::M_Unknown)
          llvm::PrintFatalError("Illegal modifier!");
	  }
	  
      Type = new OCLPointerType(Base, AS->getAddressSpace(), mods);
    }
    else if (R.isSubClassOf("OCLGroupType")) {
      std::vector<llvm::Record *> Members = R.getValueAsListOfDefs("Members");
      std::set<const OCLBasicType *> Elems;
      for (unsigned i = 0, e = Members.size(); i != e; ++i) {
        const OCLType &T = get(*Members[i]);
        if (const OCLBasicType *BT = llvm::dyn_cast<OCLBasicType>(&T))
          Elems.insert(BT);
        else if (const OCLGroupType *GT = llvm::dyn_cast<OCLGroupType>(&T))
          for (OCLGroupType::const_iterator I = GT->begin(), E = GT->end(); 
               I != E; ++I)
            Elems.insert(*I);
        else
          llvm::PrintFatalError("Illegal group type!");
      }
      Type = new OCLGroupType(R.getName(), Elems);
    }
    else llvm::PrintFatalError("Unknown type: " + R.getName());

    if (OCLBasicType *B = llvm::dyn_cast<OCLBasicType>(Type))
      B->setPredicates(FetchPredicates(R));

    Types[&R] = Type;
  }

  void BuildPointerType(const OCLBasicType &Base, const OCLPtrStructure &PtrS) {
    OCLPtrStructure ChildPtrS(PtrS.begin() + 1, PtrS.end());

    const OCLBasicType &B = PtrS.size() > 1 ? getPointerType(Base, ChildPtrS)
                                            : Base;

    OCLPointerType *P = new OCLPointerType(B, PtrS[0].first, PtrS[0].second);
    Ptrs[OCLPointerId(&Base, PtrS)] = P;
  }

  void ExpandPointerType(const OCLPointerGroupIterator &I) {
    llvm::SmallVector<const OCLPointerType*, 4> Chain;
    const OCLType *T = &I.Ptr;
    do {
      const OCLPointerType *P = llvm::cast<OCLPointerType>(T);
      Chain.push_back(P);
      T = &P->getBaseType();
    } while (llvm::isa<OCLPointerType>(T));

    assert(llvm::isa<OCLGroupType>(T) && "Bad pointer group iterator!");

    T = *I.Iter;

    OCLPtrStructure PtrS;
    PtrS.reserve(Chain.size());
    do {
      const OCLPointerType *P = Chain.pop_back_val();
      std::reverse(PtrS.begin(), PtrS.end());
      PtrS.push_back(OCLPtrDesc(P->getAddressSpace(), P->getModifierFlags()));
      std::reverse(PtrS.begin(), PtrS.end());
      T = &getPointerType(*llvm::cast<OCLBasicType>(T), PtrS);
    } while (!Chain.empty()); 
    
    GroupPtrs[&I.Ptr][I.Iter] = llvm::cast<OCLPointerType>(T);
  }

  void BuildOpaqueTypeDef(llvm::Record &R) {
    OCLOpaqueTypeDef *D = 0;

    if (R.isSubClassOf("OCLOpaqueTypeDef")) {
      const OCLType *T = &get(*R.getValueAsDef("Type"));

      auto Def = R.getValueAsString("Define");
      bool IsTarget = R.getValueAsBit("isTarget");

      D = new OCLOpaqueTypeDef(*llvm::cast<OCLOpaqueType>(T), Def, IsTarget);

      PredicateSet Preds = FetchPredicates(R);
      D->setPredicates(Preds);
    }
    else llvm::PrintFatalError("Invalid OCLOpaqueTypeDef: " + R.getName());

    OpaqueTypeDefs[&R] = D;
  }

private:
  OCLTypesMap Types;
  OCLVectorsMap Vects;
  OCLPointersMap Ptrs;
  OCLGroupPointersMap GroupPtrs;
  OCLOpaqueTypeDefsMap OpaqueTypeDefs;
};

std::unique_ptr<OCLTypesTableImpl> OCLTypesTable::Impl;

const OCLType &OCLTypesTable::get(llvm::Record &R) {
  if (!Impl) Impl.reset(new OCLTypesTableImpl());
  return Impl->get(R);
}

const OCLOpaqueTypeDef &OCLTypesTable::getOpaqueTypeDef(llvm::Record &R) {
  if (!Impl) Impl.reset(new OCLTypesTableImpl());
  return Impl->getOpaqueTypeDef(R);
}

const OCLVectorType *OCLTypesTable::getVectorType(const OCLScalarType &Base,
                                                  unsigned Width) {
  if (!Impl) Impl.reset(new OCLTypesTableImpl());
  return Impl->getVectorType(Base, Width);
}

const OCLPointerType &
OCLTypesTable::getPointerType(const OCLBasicType &Base,
                              const OCLPtrStructure &PtrS) {
  if (!Impl) Impl.reset(new OCLTypesTableImpl());
  return Impl->getPointerType(Base, PtrS);
}

static bool CompareLess(const OCLType *T, const OCLType *V) {
  return T->compareLess(V);
}

void opencrun::LoadOCLTypes(const llvm::RecordKeeper &R, OCLTypesContainer &T) {
  T.clear();

  std::vector<llvm::Record *> RawVects = R.getAllDerivedDefinitions("OCLType");

  for(unsigned I = 0, E = RawVects.size(); I < E; ++I)
    T.push_back(&OCLTypesTable::get(*RawVects[I]));

  std::sort(T.begin(), T.end(), CompareLess);
}

void opencrun::LoadOCLOpaqueTypeDefs(const llvm::RecordKeeper &R, 
                                     OCLOpaqueTypeDefsContainer &T) {
  T.clear();

  std::vector<llvm::Record *> RawVects = 
    R.getAllDerivedDefinitions("OCLOpaqueTypeDef");

  for(unsigned I = 0, E = RawVects.size(); I < E; ++I)
    T.push_back(&OCLTypesTable::getOpaqueTypeDef(*RawVects[I]));
}

//===----------------------------------------------------------------------===//
// PointerGroup iterator implementation
//===----------------------------------------------------------------------===//

const OCLPointerType &OCLPointerGroupIterator::operator*() const {
  assert(!End && "Cannot derefence 'end' iterator!");

  if (Singleton) return Ptr;
  return OCLTypesTable::Impl->get(*this);
}

const OCLPointerType *OCLPointerGroupIterator::operator->() const {
  assert(!End && "Cannot derefence 'end' iterator!");

  if (Singleton) return &Ptr;
  return &OCLTypesTable::Impl->get(*this);
}

void OCLPointerGroupIterator::FindInnerGroup() {
  const OCLType *T = &Ptr;
  do {
    T = &llvm::cast<OCLPointerType>(T)->getBaseType();
  } while (llvm::isa<OCLPointerType>(T));

  Singleton = false;

  if (!llvm::isa<OCLGroupType>(T)) {
    Singleton = true;
  } else {
    const OCLGroupType *GT = llvm::cast<OCLGroupType>(T);
    
    Iter = End ? GT->end() : GT->begin();
    IterEnd = GT->end();
    End = Iter == IterEnd;
  }
}
