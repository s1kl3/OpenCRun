#ifndef OCLTYPES_H
#define OCLTYPES_H

#include "OCLTarget.h"

#include "llvm/ADT/StringRef.h"
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
    TK_Qualified,
    TK_Pointer,
    TK_Group
  };

protected:
  OCLType(TypeKind ty, llvm::StringRef name) : Kind(ty), Name(name) {}

public:
  virtual ~OCLType();

  TypeKind getKind() const { return Kind; }
  llvm::StringRef getName() const { return Name; }

  virtual bool compareLess(const OCLType *T) const = 0;

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
    case TK_Qualified:
    case TK_Pointer: 
      return true;
    default: 
      return false;
    }
  }

public:
  virtual bool compareLess(const OCLType *T) const = 0;
  const PredicateSet &getPredicates() const { return Predicates; }
  void setPredicates(const PredicateSet &preds) { Predicates = preds; }

protected:
  OCLBasicType(TypeKind SubType, llvm::StringRef name) 
   : OCLType(SubType, name) {}

private:
  PredicateSet Predicates;
};

class OCLOpaqueType : public OCLBasicType {
public:
  static bool classof(const OCLType *Ty) {
    switch (Ty->getKind()) {
    case TK_Basic: case TK_Opaque: return true;
    default: return false;                               
    }
  }

public:
  OCLOpaqueType(llvm::StringRef name)
    : OCLBasicType(TK_Opaque, name) {}

  virtual bool compareLess(const OCLType *T) const;
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
  virtual bool compareLess(const OCLType *T) const = 0;
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
  OCLIntegerType(llvm::StringRef name, unsigned bits, bool nosign)
   : OCLScalarType(TK_Integer, name, bits), Unsigned(nosign) {}

  virtual bool compareLess(const OCLType *T) const;
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

  virtual bool compareLess(const OCLType *T) const;
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

  virtual bool compareLess(const OCLType *T) const;
  const OCLScalarType &getBaseType() const { return BaseType; }
  unsigned getWidth() const { return Width; }

private:
  std::string BuildName(const OCLScalarType &, unsigned);

private:
  const OCLScalarType &BaseType;
  unsigned Width;
};

class OCLQualifiedType : public OCLBasicType {
public:
  enum Qualifier {
    Q_ReadOnly = 1 << 0,
    Q_WriteOnly = 1 << 1,
    Q_ReadWrite = 1 << 2, // Only for OpenCL > 2.0
    Q_Unknown
  };

public:
  static bool classof(const OCLType *Ty) {
    return Ty->getKind() == TK_Qualified;
  }

public:
  OCLQualifiedType(const OCLType &unqual, const Qualifier qual)
    : OCLBasicType(TK_Qualified, BuildName(unqual, qual)),
      UnQualType(unqual), Qual(qual) {}

  virtual bool compareLess(const OCLType *T) const;
  const OCLType &getUnQualType() const { return UnQualType; }
  Qualifier getQualifier() const { return Qual; }
  bool hasQualifier(Qualifier q) const { return Qual == q; }

private:
  std::string BuildName(const OCLType &UnQualTy, Qualifier Qual);

private:
  const OCLType &UnQualType;
  const Qualifier Qual;
};

class OCLPointerType : public OCLBasicType {
public:
  enum Modifier {
    M_Const = 1 << 0,
	M_Restrict = 1 << 1,
	M_Volatile = 1 << 2,
	M_Unknown
  };

public:
  static bool classof(const OCLType *Ty) {
    return Ty->getKind() == TK_Pointer;
  }

public:
  OCLPointerType(const OCLType &base, AddressSpaceKind as, unsigned mods = 0)
   : OCLBasicType(TK_Pointer, BuildName(base, as, mods)), 
     BaseType(base), AddressSpace(as), Modifiers(mods) {}

  virtual bool compareLess(const OCLType *T) const;
  const OCLType &getBaseType() const { return BaseType; }
  AddressSpaceKind getAddressSpace() const { return AddressSpace; }
  unsigned getModifierFlags() const { return Modifiers; }
  bool hasModifier(Modifier m) const { return Modifiers & m; }

private:
  std::string BuildName(const OCLType &, AddressSpaceKind, unsigned);

private:
  const OCLType &BaseType;
  AddressSpaceKind AddressSpace;
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

  virtual bool compareLess(const OCLType *T) const;
  const_iterator begin() const { return Elements.begin(); }  
  const_iterator end() const { return Elements.end(); }  
  iterator begin() { return Elements.begin(); }  
  iterator end() { return Elements.end(); }
  size_t size() const { return Elements.size(); }

private:
  ElementContainer Elements;
};

class OCLOpaqueTypeDef {
public:
  OCLOpaqueTypeDef(const OCLOpaqueType &Ty, llvm::StringRef def, bool isTarget)
   : Type(Ty), Def(def), IsTarget(isTarget) {}

  llvm::StringRef getDef() const { return Def; }
  const OCLOpaqueType &getType() const { return Type; }
  bool isTarget() const { return IsTarget; }
  const PredicateSet &getPredicates() const { return Predicates; }
  void setPredicates(const PredicateSet &preds) { Predicates = preds; }

private:
  const OCLOpaqueType &Type;
  std::string Def;
  bool IsTarget;
  PredicateSet Predicates;
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
typedef std::vector<const OCLOpaqueTypeDef *> OCLOpaqueTypeDefsContainer;
void LoadOCLTypes(const llvm::RecordKeeper &R, OCLTypesContainer &T);
void LoadOCLOpaqueTypeDefs(const llvm::RecordKeeper &R, 
                           OCLOpaqueTypeDefsContainer &T);

//===----------------------------------------------------------------------===//
// Types table singleton
//===----------------------------------------------------------------------===//

typedef std::pair<AddressSpaceKind, unsigned> OCLPtrDesc;
typedef std::vector<OCLPtrDesc> OCLPtrStructure;

class OCLTypesTableImpl;

class OCLTypesTable {
public:
  static const OCLType &get(llvm::Record &R);

  static const OCLOpaqueTypeDef &getOpaqueTypeDef(llvm::Record &R);

  static const OCLVectorType *getVectorType(const OCLScalarType &Base, 
                                            unsigned Width);
  static const OCLQualifiedType &getQualifiedType(const OCLBasicType &Base,
                                                  const OCLQualifiedType::Qualifier Qual);
  static const OCLPointerType &getPointerType(const OCLBasicType &Base,
                                              const OCLPtrStructure &PtrS);

private:
  static std::unique_ptr<OCLTypesTableImpl> Impl;

  friend class OCLPointerGroupIterator;
  friend class OCLQualifiedGroupIterator;
  friend void LoadOCLTypes(const llvm::RecordKeeper &R, OCLTypesContainer &T);
  friend void LoadOCLOpaqueTypeDefs(const llvm::RecordKeeper &R, 
                                    OCLOpaqueTypeDefsContainer &T);
};

//===----------------------------------------------------------------------===//
// QualifiedGroup iterator
//===----------------------------------------------------------------------===//

class OCLQualifiedGroupIterator {
public:
  OCLQualifiedGroupIterator(const OCLQualifiedType &q, bool end = false)
    : Qual(q), End(end) {
      FindInnerGroup();
    }

  bool operator==(const OCLQualifiedGroupIterator &J) const {
    if (&Qual != &J.Qual) return false;
    if (!End && !J.End) return Iter == J.Iter;
    return End == J.End;
  }

  bool operator!=(const OCLQualifiedGroupIterator &J) const {
    return !(*this == J);
  }

  const OCLQualifiedType &operator*() const;
  const OCLQualifiedType *operator->() const;

  OCLQualifiedGroupIterator &operator++() {
    assert(!End && "Cannot increment 'end' iterator!");
    if (Singleton) End = true;
    else End = ++Iter == IterEnd;
    return *this;
  }

private:
  void FindInnerGroup();

private:
  const OCLQualifiedType &Qual;
  OCLGroupType::const_iterator Iter;
  OCLGroupType::const_iterator IterEnd;
  bool End;
  bool Singleton;

  friend class OCLTypesTableImpl;
};

//===----------------------------------------------------------------------===//
// PointerGroup iterator
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
