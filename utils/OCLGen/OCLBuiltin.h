#ifndef OCLBUILTIN_H
#define OCLBUILTIN_H

#include "OCLType.h"

#include <list>
#include <map>

namespace opencrun {

//===----------------------------------------------------------------------===//
// BuiltinSignature utils
//===----------------------------------------------------------------------===//
class OCLBuiltinVariant;

typedef std::vector<const OCLBasicType *> BuiltinSignature;
typedef std::list<BuiltinSignature> BuiltinSignatureList;

void ExpandSignature(const OCLBuiltinVariant &B, BuiltinSignatureList &R);

struct BuiltinSignatureCompare {
  bool operator()(const BuiltinSignature &B1, const BuiltinSignature &B2) const;
};

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
  virtual bool apply(const BuiltinSignature &Ops) const = 0;
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

public:
  virtual bool apply(const BuiltinSignature &Ops) const = 0;

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

  bool apply(const BuiltinSignature &Ops) const;
};

class OCLSameDimTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameDim;
  }

public:
  OCLSameDimTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameDim, P1, P2) {}

  bool apply(const BuiltinSignature &Ops) const;
};

class OCLSameBaseTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBase;
  }

public:
  OCLSameBaseTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBase, P1, P2) {}

  bool apply(const BuiltinSignature &Ops) const;
};

class OCLSameBaseSizeTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBaseSize;
  }

public:
  OCLSameBaseSizeTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBaseSize, P1, P2) {}

  bool apply(const BuiltinSignature &Ops) const;
};

class OCLSameBaseKindTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBaseSize;
  }

public:
  OCLSameBaseKindTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBaseSize, P1, P2) {}

  bool apply(const BuiltinSignature &Ops) const;
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
                    llvm::StringRef VarName)
   : Operands(Ops), Constraints(Constrs), VariantName(VarName) { }

public:
  const OCLType &getOperand(unsigned i) const { return *Operands[i]; }
  size_t size() const { return Operands.size(); }
  const ConstraintsContainer &getConstraints() const { return Constraints; }
  std::string getVariantName() const { return VariantName; }

private:
  OperandsContainer Operands;
  ConstraintsContainer Constraints;
  std::string VariantName;
};

class OCLBuiltin {
public:
  typedef std::map<llvm::StringRef, const OCLBuiltinVariant*> VariantsMap;
  typedef VariantsMap::const_iterator iterator;

public:
  OCLBuiltin(llvm::StringRef name, llvm::StringRef group, const VariantsMap &v)
   : Name(name), Group(group), Variants(v) {}

  ~OCLBuiltin();

public:
  std::string getName() const { return Name; }
  std::string getGroup() const { return Group; }
  iterator begin() const { return Variants.begin(); }
  iterator end() const { return Variants.end(); }
  iterator find(llvm::StringRef V) const { return Variants.find(V); }

private:
  std::string Name;
  std::string Group;
  VariantsMap Variants;
};

//===----------------------------------------------------------------------===//
// OCLBuiltinImpl
//===----------------------------------------------------------------------===//

class OCLDecl {
public:
  enum DeclKind {
    DK_Typedef,
    DK_TypedefId,
    DK_TypedefUnsigned
  };

protected:
  OCLDecl(DeclKind K) : Kind(K) {}

public:
  DeclKind getKind() const { return Kind; }

private:
  DeclKind Kind;
};

class OCLTypedefDecl : public OCLDecl {
public:
  static bool classof(const OCLDecl *D) {
    switch (D->getKind()) {
    case DK_Typedef: case DK_TypedefId: case DK_TypedefUnsigned:
      return true;
    default: return false;
    }
  }

protected:
  OCLTypedefDecl(DeclKind K, llvm::StringRef name, const OCLParam &p)
   : OCLDecl(K), Name(name), Param(p) {}

public:
  std::string getName() const { return Name; }
  const OCLParam &getParam() const { return Param; }

private:
  std::string Name;
  const OCLParam &Param;
};

class OCLTypedefIdDecl : public OCLTypedefDecl {
public:
  static bool classof(const OCLDecl *D) {
    return D->getKind() == DK_TypedefId;
  }

public:
  OCLTypedefIdDecl(llvm::StringRef name, const OCLParam &p)
   : OCLTypedefDecl(DK_TypedefId, name, p) {}
};

class OCLTypedefUnsignedDecl : public OCLTypedefDecl {
public:
  static bool classof(const OCLDecl *D) {
    return D->getKind() == DK_TypedefUnsigned;
  }

public:
  OCLTypedefUnsignedDecl(llvm::StringRef name, const OCLParam &p)
   : OCLTypedefDecl(DK_TypedefUnsigned, name, p) {}
};

class OCLReduction {
public:
  enum ReductionKind {
    RK_InfixBinAssoc
  };

protected:
  OCLReduction(ReductionKind K) : Kind(K) {}

public:
  ReductionKind getKind() const { return Kind; }

private:
  ReductionKind Kind;
};

class OCLInfixBinAssocReduction : public OCLReduction {
public:
  static bool classof(const OCLReduction *R) {
    return R->getKind() == RK_InfixBinAssoc;
  }

public:
  OCLInfixBinAssocReduction(llvm::StringRef op)
   : OCLReduction(RK_InfixBinAssoc), Operator(op) {}

  std::string getOperator() const { return Operator; }

private:
  std::string Operator;
};

class OCLStrategy {
public:
  enum StrategyKind {
    SK_RecursiveSplit,
    SK_DirectSplit,
    SK_TemplateStrategy
  };

public:
  typedef std::vector<const OCLDecl *> DeclsContainer;

public:
  typedef DeclsContainer::const_iterator decl_iterator;

public:
  decl_iterator begin() const { return Decls.begin(); }
  decl_iterator end() const { return Decls.end(); }

protected:
  OCLStrategy(StrategyKind kind, DeclsContainer &decls)
   : Kind(kind), Decls(decls) {}

public:
  StrategyKind getKind() const { return Kind; }

private:
  StrategyKind Kind;
  DeclsContainer Decls;
};

class OCLRecursiveSplit : public OCLStrategy {
public:
  static bool classof(const OCLStrategy *S) {
    return S->getKind() == SK_RecursiveSplit;
  }

public:
  OCLRecursiveSplit(llvm::StringRef scalarimpl, DeclsContainer &decls,
                    const OCLReduction *red = 0)
   : OCLStrategy(SK_RecursiveSplit, decls), ScalarImpl(scalarimpl), 
     Reduction(red) {}

  std::string getScalarImpl() const { return ScalarImpl; }
  const OCLReduction *getReduction() const { return Reduction; }

private:
  std::string ScalarImpl;
  const OCLReduction *Reduction;
};

class OCLDirectSplit : public OCLStrategy {
public:
  static bool classof(const OCLStrategy *S) {
    return S->getKind() == SK_DirectSplit;
  }

public:
  OCLDirectSplit(llvm::StringRef scalarimpl, DeclsContainer &decls)
   : OCLStrategy(SK_DirectSplit, decls), ScalarImpl(scalarimpl) {}

  std::string getScalarImpl() const { return ScalarImpl; }

private:
  std::string ScalarImpl;
};

class OCLTemplateStrategy : public OCLStrategy {
public:
  static bool classof(const OCLStrategy *S) {
    return S->getKind() == SK_TemplateStrategy;
  }

public:
  OCLTemplateStrategy(llvm::StringRef impl, DeclsContainer &decls)
   : OCLStrategy(SK_TemplateStrategy, decls), TemplateImpl(impl) {}

  std::string getTemplateImpl() const { return TemplateImpl; }

private:
  std::string TemplateImpl;
};

class OCLBuiltinImpl {
public:
  OCLBuiltinImpl(const OCLBuiltin &builtin, const OCLStrategy &strategy,
                 const std::string varname, bool target) 
    : BuiltIn(builtin), Strategy(strategy), VariantName(varname), 
      IsTarget(target) {}

public:
  const OCLBuiltin &getBuiltin() const { return BuiltIn; }
  const OCLStrategy &getStrategy() const { return Strategy; }
  const std::string getVariantName() const { return VariantName; }
  bool isTarget() const { return IsTarget; }

private:
  const OCLBuiltin &BuiltIn;
  const OCLStrategy &Strategy;
  std::string VariantName;
  bool IsTarget;
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
// OCLBuiltins and OCLBuiltinImpls Loaders
//===----------------------------------------------------------------------===//

typedef std::vector<const OCLBuiltin *> OCLBuiltinsContainer;

void LoadOCLBuiltins(const llvm::RecordKeeper &R, OCLBuiltinsContainer &B);

typedef std::vector<const OCLBuiltinImpl *> OCLBuiltinImplsContainer;

void LoadOCLBuiltinImpls(const llvm::RecordKeeper &R, 
                         OCLBuiltinImplsContainer &B);

}

#endif
