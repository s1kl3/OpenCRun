#ifndef OCLTYPES_H
#define OCLTYPES_H

#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"
#include <set>
#include <string>
#include <vector>

namespace opencrun {

//===----------------------------------------------------------------------===//
// OCLType hierarchy
//===----------------------------------------------------------------------===//

class OCLType {
public:
  enum TypeKind {
    TK_Basic,
    TK_Opaque,
    TK_Scalar,
    TK_Integer,
    TK_Real,
    TK_Vector,
    TK_Pointer,
    TK_Group
  };

protected:
  OCLType(TypeKind ty, llvm::StringRef name) : Kind(ty), Name(name) {}

public:
  virtual ~OCLType();

  TypeKind getKind() const { return Kind; }
  std::string getName() const { return Name; }

private:
  TypeKind Kind;
  std::string Name;
};


class OCLBasicType : public OCLType {
public:
  static bool classof(const OCLType *Ty) {
    switch (Ty->getKind()) {
    case TK_Basic: 
    case TK_Opaque:
    case TK_Scalar: case TK_Integer: case TK_Real: 
    case TK_Vector: 
    case  TK_Pointer: 
      return true;
    default: 
      return false;
    }
  }

protected:
  OCLBasicType(TypeKind SubType, llvm::StringRef name) 
   : OCLType(SubType, name) {}
};

class OCLOpaqueType : public OCLBasicType {
public:
  static bool classof(const OCLBasicType *Ty) {
    switch (Ty->getKind()) {
    case TK_Basic: case TK_Opaque: return true;
    default: return false;                               
    }
  }

public:
  OCLOpaqueType(llvm::StringRef name)
    : OCLBasicType(TK_Opaque, name) {}
};

class OCLScalarType : public OCLBasicType {
public:
  static bool classof(const OCLType *Ty) {
    switch (Ty->getKind()) {
    case TK_Scalar: case TK_Integer: case TK_Real: return true;
    default: return false;
    }
  }

protected:
  OCLScalarType(TypeKind SubType, llvm::StringRef name, unsigned bits) 
   : OCLBasicType(SubType, BuildName(name)), BitWidth(bits) {}

public:
  unsigned getBitWidth() const { return BitWidth; }

private:
  std::string BuildName(llvm::StringRef);

private:
  unsigned BitWidth;
};

class OCLIntegerType : public OCLScalarType {
public:
  static bool classof(const OCLType *Ty) {
    return Ty->getKind() == TK_Integer;
  }

public:
  OCLIntegerType(llvm::StringRef name, unsigned bits, bool unsign)
   : OCLScalarType(TK_Integer, name, bits), Unsigned(unsign) {}

  bool isSigned() const { return !Unsigned; }
  bool isUnsigned() const { return Unsigned; }

private:
  bool Unsigned;
};

class OCLRealType : public OCLScalarType {
public:
  static bool classof(const OCLType *Ty) {
    return Ty->getKind() == TK_Real;
  }

public:
  OCLRealType(llvm::StringRef name, unsigned bits)
   : OCLScalarType(TK_Real, name, bits) {}
};

class OCLVectorType : public OCLBasicType {
public:
  static bool classof(const OCLType *Ty) {
    return Ty->getKind() == TK_Vector;
  }

public:
  OCLVectorType(const OCLScalarType &base, unsigned w)
   : OCLBasicType(OCLType::TK_Vector, BuildName(base, w)), 
     BaseType(base), Width(w) {}

  const OCLScalarType &getBaseType() const { return BaseType; }
  unsigned getWidth() const { return Width; }

private:
  std::string BuildName(const OCLScalarType &, unsigned);

private:
  const OCLScalarType &BaseType;
  unsigned Width;
};

class OCLPointerType : public OCLBasicType {
public:
  enum Modifier {
    M_Const = 1 << 0
  };
  enum AddressSpace {
    AS_Private = 0,
    AS_Global = 1,
    AS_Local = 2,
    AS_Constant = 3
  };

public:
  static bool classof(const  OCLType *Ty) {
    return Ty->getKind() == TK_Pointer;
  }

public:
  OCLPointerType(const OCLType &base, AddressSpace as, unsigned mods = 0)
   : OCLBasicType(TK_Pointer, BuildName(base, as, mods)), 
     BaseType(base), AddrSpace(as), Modifiers(mods) {}

  const OCLType &getBaseType() const { return BaseType; }
  AddressSpace getAddressSpace() const { return AddrSpace; }
  unsigned getModifierFlags() const { return Modifiers; }
  bool hasModifier(Modifier m) const { return Modifiers & m; }

private:
  std::string BuildName(const OCLType &, AddressSpace, unsigned);

private:
  const OCLType &BaseType;
  AddressSpace AddrSpace;
  unsigned Modifiers;
};

class OCLGroupType : public OCLType {
public:
  static bool classof(const OCLType *Ty) {
    return Ty->getKind() == TK_Group;
  }
public:
  typedef std::vector<const OCLBasicType*> ElementContainer;
  typedef ElementContainer::const_iterator const_iterator;
  typedef ElementContainer::iterator iterator;

public:
  OCLGroupType(llvm::StringRef name, const std::set<const OCLBasicType*> &elems)
   : OCLType(TK_Group, name), Elements(elems.begin(), elems.end()) {}

  const_iterator begin() const { return Elements.begin(); }  
  const_iterator end() const { return Elements.end(); }  
  iterator begin() { return Elements.begin(); }  
  iterator end() { return Elements.end(); }

  size_t size() const { return Elements.size(); }

private:
  ElementContainer Elements;
};


inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const OCLType &Ty) {
  if (const OCLGroupType *GT = llvm::dyn_cast<OCLGroupType>(&Ty)) {
    OS << "@group(" << Ty.getName() << ") { ";
    for (OCLGroupType::const_iterator I = GT->begin(), E = GT->end();
         I != E; ++I)
      OS << **I << " ";
    return OS << "}";
  }
  return OS << Ty.getName();
}

//===----------------------------------------------------------------------===//
// OCLTypes Loader
//===----------------------------------------------------------------------===//

typedef std::vector<const OCLType *> OCLTypesContainer;
void LoadOCLTypes(const llvm::RecordKeeper &R, OCLTypesContainer &T);

//===----------------------------------------------------------------------===//
// Types table singleton
//===----------------------------------------------------------------------===//

class OCLTypesTableImpl;

class OCLTypesTable {
public:
  static const OCLType &get(llvm::Record &R);
  

private:
  static llvm::OwningPtr<OCLTypesTableImpl> Impl;

  friend class OCLPointerGroupIterator;
  friend void LoadOCLTypes(const llvm::RecordKeeper &R, OCLTypesContainer &T);
};

//===----------------------------------------------------------------------===//
// PointerGroup Iterator
//===----------------------------------------------------------------------===//

class OCLPointerGroupIterator {
public:
  OCLPointerGroupIterator(const OCLPointerType &p, bool end = false)
   : Ptr(p), End(end) {
    FindInnerGroup();
  }

  bool operator==(const OCLPointerGroupIterator &J) const {
    if (&Ptr != &J.Ptr) return false;
    if (!End && !J.End) return Iter == J.Iter;
    return End == J.End;
  }

  bool operator!=(const OCLPointerGroupIterator &J) const {
    return !(*this == J);
  }

  const OCLPointerType &operator*() const;
  const OCLPointerType *operator->() const;

  OCLPointerGroupIterator &operator++() {
    assert(!End && "Cannot increment 'end' iterator!");
    if (Singleton) End = true;
    else End = ++Iter == IterEnd;
    return *this;
  }

private:
  void FindInnerGroup();

private:
  const OCLPointerType &Ptr;
  OCLGroupType::const_iterator Iter;
  OCLGroupType::const_iterator IterEnd;
  bool End;
  bool Singleton;

  friend class OCLTypesTableImpl;
};

}

#endif
