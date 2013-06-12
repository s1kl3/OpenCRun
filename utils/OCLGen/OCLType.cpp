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
  return Base.getName() + llvm::Twine(Width).str();
}

std::string OCLPointerType::BuildName(const OCLType &Base, 
                                      OCLPointerType::AddressSpace AS,
                                      unsigned Modifiers) {
  return "@ptr<as:" + llvm::Twine(AS).str() +", m:" + 
         llvm::Twine(Modifiers).str() + ">{ " + Base.getName() + " }";
}

bool OCLIntegerType::compareLess(const OCLType *T) const {
  if (const OCLIntegerType *I = llvm::dyn_cast<OCLIntegerType>(T))
    return (isSigned() && I->isUnsigned()) ||
           (isSigned() && isSigned() == I->isSigned() && 
            getBitWidth() < I->getBitWidth()) || 
           (isUnsigned() && isUnsigned() == I->isUnsigned() && 
            getBitWidth() < I->getBitWidth());

  // Integer < !Integer
  return true;
}

bool OCLRealType::compareLess(const OCLType *T) const {
  if (llvm::isa<OCLIntegerType>(T)) return false;
  if (const OCLRealType *R = llvm::dyn_cast<OCLRealType>(T))
    return getBitWidth() < R->getBitWidth();
  if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(T))
    return compareLess(&V->getBaseType());

  // Real < !Integer && !Real
  return true;
}

bool OCLVectorType::compareLess(const OCLType *T) const {
  if (llvm::isa<OCLScalarType>(T))
    return getBaseType().compareLess(T);
  if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(T))
    //return getWidth() < V->getWidth() ||
    //       (getWidth() == V->getWidth() && 
    //        getBaseType().compareLess(&(V->getBaseType())));
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
  typedef std::map<const OCLPointerType*, 
                   std::map<OCLGroupType::const_iterator, 
                            const OCLPointerType *> > ExpandedPointerMap;

public:
  ~OCLTypesTableImpl() {
    llvm::DeleteContainerPointers(PtrTracker);
    llvm::DeleteContainerSeconds(Types);
  }

public:
  const OCLType &get(llvm::Record &R) {
    if (!Types.count(&R))
      BuildType(R);

    return *Types[&R];
  }

  const OCLPointerType &get(const OCLPointerGroupIterator &I) {
    if (!Ptrs.count(&I.Ptr) || !Ptrs[&I.Ptr].count(I.Iter))
      ExpandPointerType(I);

    return *Ptrs[&I.Ptr][I.Iter];
  }

private:
  void LoadRequiredTypeExt(OCLScalarType *B, llvm::Record &R) {
    if (!R.getValue("Requires")) return;

    std::vector<llvm::Record*> Requires = R.getValueAsListOfDefs("Requires");

    for (unsigned i = 0, e = Requires.size(); i != e; ++i) {
      llvm::StringRef Name = Requires[i]->getName();

      OCLExtension Ext =
        llvm::StringSwitch<OCLExtension>(Name)
          .Case("ocl_ext_cl_khr_fp16", Ext_cl_khr_fp16)
          .Case("ocl_ext_cl_khr_fp64", Ext_cl_khr_fp64)
          .Default(Ext_MaxValue);

      if (Ext == Ext_MaxValue)
        llvm::PrintFatalError("Invalid opencl extension: " + Name.str());

      B->getRequiredTypeExt().set(Ext);
    }
  }

  void BuildType(llvm::Record &R) {
    OCLType *Type = 0;

    if (R.isSubClassOf("OCLOpaqueType")) {
      llvm::StringRef OpaqueName = R.getName();
      if (OpaqueName.equals("ocl_size_t") || OpaqueName.equals("ocl_void"))
        Type = new OCLOpaqueType(R.getValueAsString("Name"));    
    }
    else if (R.isSubClassOf("OCLIntegerType")) {
      unsigned BitWidth = R.getValueAsInt("BitWidth");
      bool Unsigned = R.getValueAsBit("Unsigned");
      Type = new OCLIntegerType(R.getName(), BitWidth, Unsigned);
    }
    else if (R.isSubClassOf("OCLRealType")) {
      unsigned BitWidth = R.getValueAsInt("BitWidth");
      Type = new OCLRealType(R.getName(), BitWidth);
      LoadRequiredTypeExt(static_cast<OCLScalarType*>(Type), R);
    }
    else if (R.isSubClassOf("OCLVectorType")) {
      unsigned Width = R.getValueAsInt("Width");
      const OCLType &Base = get(*R.getValueAsDef("BaseType"));
      if (const OCLScalarType *B = llvm::dyn_cast<OCLScalarType>(&Base)) {
        OCLVectorType *V = new OCLVectorType(*B, Width);
        V->getRequiredTypeExt() = B->getRequiredTypeExt();
        Type = V;
      } else
        llvm::PrintFatalError("Invalid base type: " + 
                              R.getValueAsDef("BaseType")->getName());
    }
    else if (R.isSubClassOf("OCLPointerType")) {
      llvm::StringRef ASName = R.getValueAsDef("AddressSpace")->getName();

      OCLPointerType::AddressSpace AS = 
        llvm::StringSwitch<OCLPointerType::AddressSpace>(ASName)
          .Case("ocl_as_private", OCLPointerType::AS_Private)
          .Case("ocl_as_global", OCLPointerType::AS_Global)
          .Case("ocl_as_local", OCLPointerType::AS_Local)
          .Case("ocl_as_constant", OCLPointerType::AS_Constant)
          .Default(OCLPointerType::AS_Unknown);

      if (AS == OCLPointerType::AS_Unknown)
        llvm::PrintFatalError("Invalid address space: " + ASName.str());

      const OCLType &Base = get(*R.getValueAsDef("BaseType"));
      // TODO: modifiers
      Type = new OCLPointerType(Base, AS);
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

    Types[&R] = Type;
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
    
    do {
      const OCLPointerType *P = Chain.pop_back_val();
      T = new OCLPointerType(*T, P->getAddressSpace(), P->getModifierFlags());
      PtrTracker.push_back(llvm::cast<OCLPointerType>(T));
    } while (!Chain.empty()); 
    
    Ptrs[&I.Ptr][I.Iter] = llvm::cast<OCLPointerType>(T);
  }

private:
  OCLTypesMap Types;
  ExpandedPointerMap Ptrs;
  std::vector<const OCLPointerType*> PtrTracker;
};

llvm::OwningPtr<OCLTypesTableImpl> OCLTypesTable::Impl;

const OCLType &OCLTypesTable::get(llvm::Record &R) {
  if (!Impl) Impl.reset(new OCLTypesTableImpl());
  return Impl->get(R);
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
