#ifndef OCLBUILTIN_H
#define OCLBUILTIN_H

#include "OCLType.h"

#include <list>

namespace opencrun {

typedef std::vector<const OCLBasicType *> BuiltinSignature;
void SortBuiltinSignatureList(std::list<BuiltinSignature> &l);

//===----------------------------------------------------------------------===//
// Param accessor
//===----------------------------------------------------------------------===//

class OCLParam {
public:
  enum ParamKind {
    PK_Id,
    PK_Pointee
  };

protected:
  OCLParam(ParamKind K, unsigned op) : Kind(K), Operand(op) {}

public:
  virtual ~OCLParam();
  ParamKind getKind() const { return Kind; }
  unsigned getOperand() const { return Operand; }
  virtual const OCLBasicType *get(const BuiltinSignature &Ops) const = 0;

private:
  ParamKind Kind;
  unsigned Operand;
};

class OCLParamId : public OCLParam {
public:
  static bool classof(const OCLParam *P) {
    return P->getKind() == PK_Id;
  }

public:
  OCLParamId(unsigned op) : OCLParam(PK_Id, op) {}

  virtual const OCLBasicType *get(const BuiltinSignature &Ops) const;
};

class OCLParamPointee : public OCLParam {
public:
  static bool classof(const OCLParam *P) {
    return P->getKind() == PK_Pointee;
  }

public:
  OCLParamPointee(unsigned op, unsigned n) : OCLParam(PK_Pointee, op), Nest(n) {}

  virtual const OCLBasicType *get(const BuiltinSignature &Ops) const;

private:
  unsigned Nest;
};

//===----------------------------------------------------------------------===//
// OCLTypeConstraint hierarchy
//===----------------------------------------------------------------------===//

class OCLTypeConstraint {
public:
  enum ConstraintKind {
    CK_Binary,
    CK_Same,
    CK_SameDim,
    CK_SameBase,
    CK_SameBaseKind,
    CK_SameBaseSize
  };

protected:
  OCLTypeConstraint(ConstraintKind K) : Kind(K), MaxOperand(0) {}

public:
  virtual ~OCLTypeConstraint();

  ConstraintKind getKind() const { return Kind; }
  virtual bool apply(const std::vector<const OCLBasicType*> &Ops) const = 0;
  unsigned getMaxOperand() const { return MaxOperand; }

private:
  ConstraintKind Kind;
protected:
  unsigned MaxOperand;
};

class OCLBinaryTypeConstraint : public OCLTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    switch (C->getKind()) {
    case CK_Binary: case CK_Same: case CK_SameDim:
    case CK_SameBase: case CK_SameBaseKind: case CK_SameBaseSize:
      return true;
    default: return false;
    }
  }
  virtual bool apply(const std::vector<const OCLBasicType*> &Ops) const = 0;
protected:
  OCLBinaryTypeConstraint(ConstraintKind K, 
                          const OCLParam &P1, const OCLParam &P2)
   : OCLTypeConstraint(K), FirstParam(P1), SecondParam(P2) {
    MaxOperand = std::max(FirstParam.getOperand(), SecondParam.getOperand());
  }

protected:
  const OCLParam &FirstParam;
  const OCLParam &SecondParam;
};

class OCLSameTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_Same;
  }

public:
  OCLSameTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_Same, P1, P2) {}

  bool apply(const std::vector<const OCLBasicType*> &Ops) const;
};

class OCLSameDimTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameDim;
  }

public:
  OCLSameDimTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameDim, P1, P2) {}

  bool apply(const std::vector<const OCLBasicType*> &Ops) const;
};

class OCLSameBaseTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBase;
  }

public:
  OCLSameBaseTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBase, P1, P2) {}

  bool apply(const std::vector<const OCLBasicType*> &Ops) const;
};

class OCLSameBaseSizeTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBaseSize;
  }

public:
  OCLSameBaseSizeTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBaseSize, P1, P2) {}

  bool apply(const std::vector<const OCLBasicType*> &Ops) const;
};

class OCLSameBaseKindTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBaseSize;
  }

public:
  OCLSameBaseKindTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBaseSize, P1, P2) {}

  bool apply(const std::vector<const OCLBasicType*> &Ops) const;
};

//===----------------------------------------------------------------------===//
// OCLBuiltin
//===----------------------------------------------------------------------===//

class OCLBuiltinVariant {
public:
  typedef std::vector<const OCLTypeConstraint*> ConstraintVector;
public:
  OCLBuiltinVariant(std::vector<const OCLType*> &Ops, ConstraintVector &Constrs)
   : Operands(Ops), Constraints(Constrs) {}

public:
  const OCLType &getOperand(unsigned i) const { return *Operands[i]; }
  size_t size() const { return Operands.size(); }
  const ConstraintVector &getConstraints() const { return Constraints; }

private:
  std::vector<const OCLType*> Operands;
  ConstraintVector Constraints;
};

class OCLBuiltin {
public:
  typedef std::vector<OCLBuiltinVariant> VariantsContainer;
  typedef VariantsContainer::const_iterator iterator;
public:
  OCLBuiltin(llvm::StringRef name, llvm::StringRef group, 
             const std::vector<OCLBuiltinVariant> &v)
   : Name(name), Group(group), Variants(v) {}

public:
  std::string getName() const { return Name; }
  std::string getGroup() const { return Group; }
  iterator begin() const { return Variants.begin(); }
  iterator end() const { return Variants.end(); }

private:
  std::string Name;
  std::string Group;
  std::vector<OCLBuiltinVariant> Variants;
};

//===----------------------------------------------------------------------===//
// Builtins table singleton
//===----------------------------------------------------------------------===//

class OCLBuiltinsTableImpl;

class OCLBuiltinsTable {
public:
  static const OCLBuiltin &get(llvm::Record &R);

private:
  static llvm::OwningPtr<OCLBuiltinsTableImpl> Impl;
};

//===----------------------------------------------------------------------===//
// OCLBuiltins Loader
//===----------------------------------------------------------------------===//

typedef std::vector<const OCLBuiltin*> OCLBuiltinsContainer;
void LoadOCLBuiltins(const llvm::RecordKeeper &R, OCLBuiltinsContainer &B);

void ExpandSignature(const OCLBuiltinVariant &B, std::list<BuiltinSignature> &R);
}

#endif
