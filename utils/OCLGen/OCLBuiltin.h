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
  virtual bool apply(const std::vector<const OCLBasicType *> &Ops) const = 0;
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
  virtual bool apply(const std::vector<const OCLBasicType *> &Ops) const = 0;
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

  bool apply(const std::vector<const OCLBasicType *> &Ops) const;
};

class OCLSameDimTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameDim;
  }

public:
  OCLSameDimTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameDim, P1, P2) {}

  bool apply(const std::vector<const OCLBasicType *> &Ops) const;
};

class OCLSameBaseTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBase;
  }

public:
  OCLSameBaseTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBase, P1, P2) {}

  bool apply(const std::vector<const OCLBasicType *> &Ops) const;
};

class OCLSameBaseSizeTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBaseSize;
  }

public:
  OCLSameBaseSizeTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBaseSize, P1, P2) {}

  bool apply(const std::vector<const OCLBasicType *> &Ops) const;
};

class OCLSameBaseKindTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBaseSize;
  }

public:
  OCLSameBaseKindTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBaseSize, P1, P2) {}

  bool apply(const std::vector<const OCLBasicType *> &Ops) const;
};

//===----------------------------------------------------------------------===//
// OCLEmitTypedef hierarchy 
//===----------------------------------------------------------------------===//

class OCLEmitTypedef {
public:
  enum TypedefKind {
    TK_Actual,
    TK_Unsigned
  };

protected:
  OCLEmitTypedef(TypedefKind kind, llvm::StringRef ty, const OCLParam &op) : 
    Kind(kind),
    TypeName(ty),
    Op(op) { }

public:
  TypedefKind getKind() const { return Kind; }
  std::string getName() const { return TypeName; }
  const OCLParam &getParam() const { return Op; }

private:
  TypedefKind Kind;
  std::string TypeName;
  const OCLParam &Op;
};

class OCLEmitTypedefActual : public OCLEmitTypedef {
public:
  static bool classof(const OCLEmitTypedef *E) {
    return E->getKind() == TK_Actual;
  }

public:
  OCLEmitTypedefActual(llvm::StringRef ty, const OCLParam &op) :
    OCLEmitTypedef(TK_Actual, ty, op) { }
};

class OCLEmitTypedefUnsigned : public OCLEmitTypedef {
public:
  static bool classof(const OCLEmitTypedef *E) {
    return E->getKind() == TK_Unsigned;
  }

public:
  OCLEmitTypedefUnsigned(llvm::StringRef ty, const OCLParam &op) :
    OCLEmitTypedef(TK_Unsigned, ty, op) { }
};

//===----------------------------------------------------------------------===//
// OCLStrategy hierarchy 
//===----------------------------------------------------------------------===//

class OCLStrategy {
public:
  enum StrategyKind {
    SK_RecursiveSplit,
    SK_DirectSplit
  };

public:
  typedef std::vector<const OCLEmitTypedef *> TypedefsContainer;

public:
  typedef TypedefsContainer::const_iterator iterator;

public:
  iterator begin() const { return Typedefs.begin(); }
  iterator end() const { return Typedefs.end(); }

protected:
  OCLStrategy(StrategyKind kind,
              llvm::StringRef scalarimpl,
              TypedefsContainer &typedefs) : 
    Kind(kind),
    ScalarImpl(scalarimpl),
    Typedefs(typedefs) { }

public:
  StrategyKind getKind() const { return Kind; }
  std::string getScalarImpl() const { return ScalarImpl; }

private:
  StrategyKind Kind;
  std::string ScalarImpl;
  TypedefsContainer Typedefs;
};

class RecursiveSplit : public OCLStrategy {
public:
  static bool classof(const OCLStrategy *S) {
    return S->getKind() == SK_RecursiveSplit;
  }

public:
  RecursiveSplit(llvm::StringRef scalarimpl, 
                 TypedefsContainer &typedefs) : 
    OCLStrategy(SK_RecursiveSplit, scalarimpl, typedefs) { }
};

class DirectSplit : public OCLStrategy {
public:
  static bool classof(const OCLStrategy *S) {
    return S->getKind() == SK_DirectSplit;
  }

public:
  DirectSplit(llvm::StringRef scalarimpl, 
              TypedefsContainer &typedefs) : 
    OCLStrategy(SK_DirectSplit, scalarimpl, typedefs) { }
};

//===----------------------------------------------------------------------===//
// OCLBuiltin
//===----------------------------------------------------------------------===//

class OCLBuiltinVariant {
public:
  typedef std::vector<const OCLType *> OperandsContainer;
  typedef std::vector<const OCLTypeConstraint *> ConstraintsContainer;

public:
  OCLBuiltinVariant(OperandsContainer &Ops, 
                    ConstraintsContainer &Constrs,
                    llvm::StringRef VarId)
   : Operands(Ops), Constraints(Constrs), VariantId(VarId) { }

public:
  const OCLType &getOperand(unsigned i) const { return *Operands[i]; }
  size_t size() const { return Operands.size(); }
  const ConstraintsContainer &getConstraints() const { return Constraints; }
  std::string getVariantId() const { return VariantId; }

private:
  OperandsContainer Operands;
  ConstraintsContainer Constraints;
  std::string VariantId;
};

class OCLBuiltin {
public:
  typedef std::vector<OCLBuiltinVariant> VariantsContainer;
  typedef VariantsContainer::const_iterator iterator;

public:
  OCLBuiltin(llvm::StringRef name, 
             llvm::StringRef group,
             const VariantsContainer &v)
   : Name(name), Group(group), Variants(v) { }

public:
  std::string getName() const { return Name; }
  std::string getGroup() const { return Group; }
  iterator begin() const { return Variants.begin(); }
  iterator end() const { return Variants.end(); }

private:
  std::string Name;
  std::string Group;
  VariantsContainer Variants;
};

//===----------------------------------------------------------------------===//
// OCLBuiltinImpl
//===----------------------------------------------------------------------===//

class OCLBuiltinImpl {
public:
  OCLBuiltinImpl(const OCLBuiltin &builtin, 
                 const OCLStrategy &strategy,
                 const std::string variantid = "") 
    : BuiltIn(builtin),
      Strategy(strategy),
      VariantId(variantid) { }

public:
  const OCLBuiltin &getBuiltin() const { return BuiltIn; }
  const OCLStrategy &getStrategy() const { return Strategy; }
  const std::string getVariantId() const { return VariantId; }

private:
  const OCLBuiltin &BuiltIn;
  const OCLStrategy &Strategy;
  const std::string VariantId;
};

//===----------------------------------------------------------------------===//
// Builtins table singleton
//===----------------------------------------------------------------------===//

class OCLBuiltinsTableImpl;

class OCLBuiltinsTable {
public:
  static const OCLBuiltin &getBuiltin(llvm::Record &R);
  static const OCLBuiltinImpl &getBuiltinImpl(llvm::Record &R);

private:
  static llvm::OwningPtr<OCLBuiltinsTableImpl> Impl;
};

//===----------------------------------------------------------------------===//
// OCLBuiltins Loader
//===----------------------------------------------------------------------===//

typedef std::vector<const OCLBuiltin *> OCLBuiltinsContainer;

void LoadOCLBuiltins(const llvm::RecordKeeper &R, OCLBuiltinsContainer &B);

void ExpandSignature(const OCLBuiltinVariant &B, std::list<BuiltinSignature> &R);


//===----------------------------------------------------------------------===//
// OCLBuiltinImpls Loader
//===----------------------------------------------------------------------===//

typedef std::vector<const OCLBuiltinImpl *> OCLBuiltinImplsContainer;

void LoadOCLBuiltinImpls(const llvm::RecordKeeper &R, OCLBuiltinImplsContainer &B);

}

#endif
