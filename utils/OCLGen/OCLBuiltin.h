#ifndef OCLBUILTIN_H
#define OCLBUILTIN_H

#include "OCLType.h"

#include <list>
#include <map>
#include <set>

namespace opencrun {

//===----------------------------------------------------------------------===//
// BuiltinSign utils
//===----------------------------------------------------------------------===//
class OCLBuiltinVariant;

class BuiltinSign {
public:
  BuiltinSign() {}
  BuiltinSign(const PredicateSet &Preds) : Preds(Preds) { }

public:
  size_t size() const { return Tys.size(); }
  void reserve(size_t n) { Tys.reserve(n); } 

public:
  const OCLBasicType *operator[](size_t n) const { 
    return Tys[n]; 
  }

public:
  void push_back(const OCLBasicType *Ty) { Tys.push_back(Ty); }
  void pop_back() { Tys.pop_back(); }

public:
  const PredicateSet &getPredicates() const { return Preds; }
  void setPredicates(const PredicateSet &preds) { Preds = preds; }

private:
  std::vector<const OCLBasicType *> Tys;
  PredicateSet Preds;
};

typedef std::list<BuiltinSign> BuiltinSignList;

void ExpandSigns(const OCLBuiltinVariant &B, BuiltinSignList &R);

struct BuiltinSignCompare {
  bool operator()(const BuiltinSign &B1, const BuiltinSign &B2) const;
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
  virtual const OCLBasicType *get(const BuiltinSign &Ops) const = 0;

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

  virtual const OCLBasicType *get(const BuiltinSign &Ops) const;
};

class OCLParamPointee : public OCLParam {
public:
  static bool classof(const OCLParam *P) {
    return P->getKind() == PK_Pointee;
  }

public:
  OCLParamPointee(unsigned op, unsigned n) : OCLParam(PK_Pointee, op), Nest(n) {}

  virtual const OCLBasicType *get(const BuiltinSign &Ops) const;

private:
  unsigned Nest;
};

//===----------------------------------------------------------------------===//
// OCLTypeConstraint hierarchy
//===----------------------------------------------------------------------===//

class OCLTypeConstraint {
public:
  enum ConstraintKind {
    CK_Not,
    CK_Same,
    CK_SameDim,
    CK_SameBase,
    CK_SameBaseKind,
    CK_SameBaseSize,
    CK_SameBitSize,
    CK_CompatibleWithImageDim
  };

protected:
  OCLTypeConstraint(ConstraintKind K) : Kind(K), MaxOperand(0) {}

public:
  virtual ~OCLTypeConstraint();

  ConstraintKind getKind() const { return Kind; }
  virtual bool apply(const BuiltinSign &Ops) const = 0;
  unsigned getMaxOperand() const { return MaxOperand; }

private:
  ConstraintKind Kind;
protected:
  unsigned MaxOperand;
};

class OCLNotTypeConstraint : public OCLTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_Not;
  }

public:
  OCLNotTypeConstraint(const OCLTypeConstraint &BC)
   : OCLTypeConstraint(CK_Not), Basic(BC) {
    MaxOperand = BC.getMaxOperand();
  }

  virtual bool apply(const BuiltinSign &Ops) const {
    return !Basic.apply(Ops);
  }

private:
  const OCLTypeConstraint &Basic;
};

class OCLBinaryTypeConstraint : public OCLTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    switch (C->getKind()) {
    case CK_Same: case CK_SameDim: case CK_SameBase: 
    case CK_SameBaseKind: case CK_SameBaseSize: case CK_SameBitSize:
    case CK_CompatibleWithImageDim:
      return true;
    default: return false;
    }
  }

public:
  virtual bool apply(const BuiltinSign &Ops) const = 0;

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

  bool apply(const BuiltinSign &Ops) const;
};

class OCLSameDimTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameDim;
  }

public:
  OCLSameDimTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameDim, P1, P2) {}

  bool apply(const BuiltinSign &Ops) const;
};

class OCLSameBaseTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBase;
  }

public:
  OCLSameBaseTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBase, P1, P2) {}

  bool apply(const BuiltinSign &Ops) const;
};

class OCLSameBaseSizeTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBaseSize;
  }

public:
  OCLSameBaseSizeTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBaseSize, P1, P2) {}

  bool apply(const BuiltinSign &Ops) const;
};

class OCLSameBaseKindTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBaseSize;
  }

public:
  OCLSameBaseKindTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBaseSize, P1, P2) {}

  bool apply(const BuiltinSign &Ops) const;
};

class OCLSameBitSizeTypeConstraint : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_SameBitSize;
  }

public:
  OCLSameBitSizeTypeConstraint(const OCLParam &P1, const OCLParam &P2)
   : OCLBinaryTypeConstraint(CK_SameBitSize, P1, P2) {}

  bool apply(const BuiltinSign &Ops) const;
};

class OCLCompatibleWithImageDim : public OCLBinaryTypeConstraint {
public:
  static bool classof(const OCLTypeConstraint *C) {
    return C->getKind() == CK_CompatibleWithImageDim;
  }

public:
  OCLCompatibleWithImageDim(const OCLParam &P1, const OCLParam &P2)
    : OCLBinaryTypeConstraint(CK_CompatibleWithImageDim, P1, P2) {}

  bool apply(const BuiltinSign &Ops) const;
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
                    llvm::StringRef VarName,
                    const PredicateSet &Preds)
   : Operands(Ops), Constraints(Constrs), VariantName(VarName), Predicates(Preds) { }

public:
  const OCLType &getOperand(unsigned i) const { return *Operands[i]; }
  size_t size() const { return Operands.size(); }
  const ConstraintsContainer &getConstraints() const { return Constraints; }
  std::string getVariantName() const { return VariantName; }
  const PredicateSet &getPredicates() const { return Predicates; }
  void setPredicates(const PredicateSet &preds) { Predicates = preds; }

private:
  OperandsContainer Operands;
  ConstraintsContainer Constraints;
  std::string VariantName;
  PredicateSet Predicates;
};

class OCLBuiltin {
public:
  enum BuiltinKind {
    BK_Generic,
    BK_Convert,
    BK_Reinterpret
  };

public:
  typedef std::map<llvm::StringRef, const OCLBuiltinVariant*> VariantsMap;
  typedef VariantsMap::const_iterator var_iterator;

protected:
  OCLBuiltin(BuiltinKind K, llvm::StringRef group, llvm::StringRef name,
             const VariantsMap &variants)
   : Kind(K), Group(group), Name(name), Variants(variants) {}

public:
  virtual ~OCLBuiltin();

public:
  BuiltinKind getKind() const { return Kind; }
  std::string getName() const { return Name; }
  std::string getGroup() const { return Group; }
  var_iterator begin() const { return Variants.begin(); }
  var_iterator end() const { return Variants.end(); }
  var_iterator find(llvm::StringRef V) const { return Variants.find(V); }

private:
  BuiltinKind Kind;
  std::string Group;
  std::string Name;
  VariantsMap Variants;
};

class OCLGenericBuiltin : public OCLBuiltin {
public:
  static bool classof(const OCLBuiltin *B) {
    return B->getKind() == BK_Generic;
  }

public:
  OCLGenericBuiltin(llvm::StringRef Group, llvm::StringRef Name,
                    const VariantsMap &Variants)
   : OCLBuiltin(BK_Generic, Group, Name, Variants) {}
};

class OCLCastBuiltin : public OCLBuiltin {
public:
  static bool classof(const OCLBuiltin *B) {
    return B->getKind() != BK_Generic;
  }

protected:
  OCLCastBuiltin(BuiltinKind K, llvm::StringRef Group, llvm::StringRef Name,
                 const VariantsMap &Variants) 
   : OCLBuiltin(K, Group, Name, Variants) {}
};

class OCLRoundingMode {
public:
  enum RoundingModeDefault {
    RMD_None = 0,
    RMD_Integer = 1 << 0,
    RMD_Real = 1 << 1
  };

public:
  OCLRoundingMode(llvm::StringRef name, unsigned rmd)
   : Name(name), DefaultFor(rmd) {}

public:
  std::string getName() const { return Name; }
  bool isDefaultFor(const OCLBasicType &Ty) const;

private:
  std::string Name;
  unsigned DefaultFor;
};

class OCLConvertBuiltin : public OCLCastBuiltin {
public:
  static bool classof(const OCLBuiltin *B) {
    return B->getKind() == BK_Convert;
  }

public:
  OCLConvertBuiltin(llvm::StringRef Group, llvm::StringRef Name, bool sat,
                    const OCLRoundingMode &rm, const VariantsMap &Variants)
   : OCLCastBuiltin(BK_Convert, Group, Name, Variants), RoundingMode(rm), 
     Saturation(sat) {}

  const OCLRoundingMode &getRoundingMode() const { return RoundingMode; }
  bool hasSaturation() const { return Saturation; }

private:
  const OCLRoundingMode &RoundingMode;
  bool Saturation;
};

class OCLReinterpretBuiltin : public OCLCastBuiltin {
public:
  static bool classof(const OCLBuiltin *B) {
    return B->getKind() == BK_Reinterpret;
  }

public:
  OCLReinterpretBuiltin(llvm::StringRef Group, llvm::StringRef Name,
                        const VariantsMap &Variants)
   : OCLCastBuiltin(BK_Reinterpret, Group, Name, Variants) {}
};

//===----------------------------------------------------------------------===//
// OCLBuiltinImpl
//===----------------------------------------------------------------------===//

class OCLDecl {
public:
  enum DeclKind {
    DK_TypedefId,
    DK_TypedefUnsigned,
    DK_MinValue,
    DK_MaxValue,
    DK_BuiltinName,
    DK_LibM
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
    case DK_TypedefId: case DK_TypedefUnsigned:
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

class OCLTypeValueDecl : public OCLDecl {
public:
  static bool classof(const OCLDecl *D) {
    switch (D->getKind()) {
    case DK_MinValue: case DK_MaxValue:
      return true;
    default: return false;
    }
  }

protected:
  OCLTypeValueDecl(DeclKind K, llvm::StringRef id, const OCLParam &p)
   : OCLDecl(K), ID(id), Param(p) {}

public:
  std::string getID() const { return ID; }
  const OCLParam &getParam() const { return Param; }

private:
  std::string ID;
  const OCLParam &Param;
};

class OCLMinValueDecl : public OCLTypeValueDecl {
public:
  static bool classof(const OCLDecl *D) {
    return D->getKind() == DK_MinValue;
  }

public:
  OCLMinValueDecl(llvm::StringRef id, const OCLParam &p) 
   : OCLTypeValueDecl(DK_MinValue, id, p) {}
};

class OCLMaxValueDecl : public OCLTypeValueDecl {
public:
  static bool classof(const OCLDecl *D) {
    return D->getKind() == DK_MaxValue;
  }

public:
  OCLMaxValueDecl(llvm::StringRef id, const OCLParam &p) 
   : OCLTypeValueDecl(DK_MaxValue, id, p) {}
};

class OCLBuiltinNameDecl : public OCLDecl {
public:
  static bool classof(const OCLDecl *D) {
    return D->getKind() == DK_BuiltinName;
  }

public:
  OCLBuiltinNameDecl(llvm::StringRef name, const OCLBuiltin &b)
   : OCLDecl(DK_BuiltinName), Name(name), Builtin(b) {}

public:
  std::string getName() const { return Name; }
  const OCLBuiltin &getBuiltin() const { return Builtin; }

private:
  std::string Name;
  const OCLBuiltin &Builtin;
};

class OCLLibMDecl : public OCLDecl {
public:
  static bool classof(const OCLDecl *D) {
    return D->getKind() == DK_LibM;
  }

public:
  OCLLibMDecl(const OCLParam &p) : OCLDecl(DK_LibM), Param(p) {}

  const OCLParam &getParam() const { return Param; }

private:
  const OCLParam &Param;
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

class OCLRequirement {
public:
  enum RequirementKind {
    RK_Include,
    RK_CodeBlock
  };

protected:
  OCLRequirement(RequirementKind K) : Kind(K) {}

public:
  RequirementKind getKind() const { return Kind; }

private:
  RequirementKind Kind;
};

class OCLIncludeRequirement : public OCLRequirement {
public:
  static bool classof(const OCLRequirement *R) {
    return R->getKind() == RK_Include;
  }

public:
  OCLIncludeRequirement(llvm::StringRef filename)
   : OCLRequirement(RK_Include), FileName(filename) {}

public:
  std::string getFileName() const { return FileName; }

private:
  std::string FileName;
};

class OCLCodeBlockRequirement : public OCLRequirement {
public:
  static bool classof(const OCLRequirement *R) {
    return R->getKind() == RK_CodeBlock;
  }

public:
  OCLCodeBlockRequirement(llvm::StringRef codeblock)
   : OCLRequirement(RK_CodeBlock), CodeBlock(codeblock) {}

public:
  std::string getCodeBlock() const { return CodeBlock; }

private:
  std::string CodeBlock;
};

class OCLBuiltinImpl {
public:
  typedef std::vector<const OCLRequirement*> RequirementsContainer;
  typedef RequirementsContainer::const_iterator req_iterator;

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

  void addRequirement(const OCLRequirement *R) {
    Requirements.push_back(R);
  }
  req_iterator req_begin() const { return Requirements.begin(); }
  req_iterator req_end() const { return Requirements.end(); }

private:
  const OCLBuiltin &BuiltIn;
  const OCLStrategy &Strategy;
  std::string VariantName;
  bool IsTarget;
  RequirementsContainer Requirements;
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
