#include "OCLBuiltin.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Error.h"

#include <algorithm>
#include <map>

using namespace opencrun;

OCLParam::~OCLParam() {
}

const OCLBasicType *OCLParamId::get(const BuiltinSign &Ops) const {
  return Ops[getOperand()];
}

const OCLBasicType *OCLParamPointee::get(const BuiltinSign &Ops) const {
  const OCLBasicType *R = Ops[getOperand()];
  for (unsigned i = 0; i != Nest; ++i) {
    if (const OCLPointerType *P = llvm::dyn_cast<OCLPointerType>(R)) {
      R = llvm::dyn_cast<OCLBasicType>(&P->getBaseType());
      assert(R);
    } else llvm::PrintFatalError("Dereferencing not a pointer type!");
  }
  return R;
}

OCLTypeConstraint::~OCLTypeConstraint() {
}

bool OCLSameTypeConstraint::
apply(const std::vector<const OCLBasicType *> &Ops) const {
  return FirstParam.get(Ops) == SecondParam.get(Ops);
}

bool OCLSameDimTypeConstraint::
apply(const std::vector<const OCLBasicType *> &Ops) const {
  const OCLBasicType *P1 = FirstParam.get(Ops);
  const OCLBasicType *P2 = SecondParam.get(Ops);

  if (llvm::isa<OCLVectorType>(P1) && llvm::isa<OCLVectorType>(P2)) {
    const OCLVectorType *V1 = llvm::cast<OCLVectorType>(P1);
    const OCLVectorType *V2 = llvm::cast<OCLVectorType>(P2);

    return V1->getWidth() == V2->getWidth();
  }
  return llvm::isa<OCLScalarType>(P1) && llvm::isa<OCLScalarType>(P2);
}

bool OCLSameBaseTypeConstraint::
apply(const std::vector<const OCLBasicType *> &Ops) const {
  const OCLBasicType *P1 = FirstParam.get(Ops);
  const OCLBasicType *P2 = SecondParam.get(Ops);

  const OCLType *B1 = 0, *B2 = 0;

  if (const OCLScalarType *S = llvm::dyn_cast<OCLScalarType>(P1))
    B1 = S;
  if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(P1))
    B1 = &V->getBaseType();
  if (const OCLScalarType *S = llvm::dyn_cast<OCLScalarType>(P2))
    B2 = S;
  if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(P2))
    B2 = &V->getBaseType();

  if (B1 && B2) return B1 == B2;
  return false;
}

bool OCLSameBaseSizeTypeConstraint::
apply(const std::vector<const OCLBasicType *> &Ops) const {
  const OCLBasicType *P1 = FirstParam.get(Ops);
  const OCLBasicType *P2 = SecondParam.get(Ops);

  const OCLType *B1 = 0, *B2 = 0;

  if (const OCLScalarType *S = llvm::dyn_cast<OCLScalarType>(P1))
    B1 = S;
  if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(P1))
    B1 = &V->getBaseType();
  if (const OCLScalarType *S = llvm::dyn_cast<OCLScalarType>(P2))
    B2 = S;
  if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(P2))
    B2 = &V->getBaseType();

  if (!B1 || !B2) return false;

  if (!llvm::isa<OCLScalarType>(B1) || !llvm::isa<OCLScalarType>(B2))
    return false;

  const OCLScalarType *S1 = llvm::cast<OCLScalarType>(B1);
  const OCLScalarType *S2 = llvm::cast<OCLScalarType>(B2);

  return S1->getBitWidth() == S2->getBitWidth();
}

bool OCLSameBaseKindTypeConstraint::
apply(const std::vector<const OCLBasicType *> &Ops) const {
  const OCLBasicType *P1 = FirstParam.get(Ops);
  const OCLBasicType *P2 = SecondParam.get(Ops);

  const OCLType *B1 = 0, *B2 = 0;

  if (const OCLScalarType *S = llvm::dyn_cast<OCLScalarType>(P1))
    B1 = S;
  if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(P1))
    B1 = &V->getBaseType();
  if (const OCLScalarType *S = llvm::dyn_cast<OCLScalarType>(P2))
    B2 = S;
  if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(P2))
    B2 = &V->getBaseType();

  if (!B1 || !B2) return false;

  if (!llvm::isa<OCLScalarType>(B1) || !llvm::isa<OCLScalarType>(B2))
    return false;

  const OCLScalarType *S1 = llvm::cast<OCLScalarType>(B1);
  const OCLScalarType *S2 = llvm::cast<OCLScalarType>(B2);

  return S1->getKind() == S2->getKind();
}

OCLBuiltin::~OCLBuiltin() {
  llvm::DeleteContainerSeconds(Variants);
}

static bool CompareLess(const OCLBuiltin *B1, const OCLBuiltin *B2) {
  return B1->getGroup() < B2->getGroup() ||
         (B1->getGroup() == B2->getGroup() && B1->getName() < B2->getName());
}

void opencrun::LoadOCLBuiltins(const llvm::RecordKeeper &R, 
                               OCLBuiltinsContainer &B) {
  std::vector<llvm::Record *> RawVects = 
    R.getAllDerivedDefinitions("OCLBuiltin");

  for (unsigned I = 0, E = RawVects.size(); I < E; ++I)
    B.push_back(&OCLBuiltinsTable::getBuiltin(*RawVects[I]));

  std::sort(B.begin(), B.end(), CompareLess);
}

void opencrun::LoadOCLBuiltinImpls(const llvm::RecordKeeper &R,
                                   OCLBuiltinImplsContainer &B) {
  std::vector<llvm::Record *> RawVects =
    R.getAllDerivedDefinitions("OCLBuiltinImpl");

  for (unsigned I = 0, E = RawVects.size(); I < E; ++I)
    B.push_back(&OCLBuiltinsTable::getBuiltinImpl(*RawVects[I]));
}

//===----------------------------------------------------------------------===//
// Builtins table implementation
//===----------------------------------------------------------------------===//

class opencrun::OCLBuiltinsTableImpl {
public:
  typedef std::map<llvm::Record *, const OCLBuiltin *> OCLBuiltinsMap;
  typedef std::map<llvm::Record *, const OCLTypeConstraint *> OCLConstraintsMap;
  typedef std::map<llvm::Record *, const OCLParam *> OCLParamsMap;

  typedef std::map<llvm::Record *, const OCLBuiltinImpl *> OCLBuiltinImplsMap;
  typedef std::map<llvm::Record *, const OCLStrategy *> OCLStrategiesMap;
  typedef std::map<llvm::Record *, const OCLReduction *> OCLReductionsMap;
  typedef std::map<llvm::Record *, const OCLDecl *> OCLDeclsMap;

public:
  ~OCLBuiltinsTableImpl() { 
    llvm::DeleteContainerSeconds(Builtins);
    llvm::DeleteContainerSeconds(Constraints);
    llvm::DeleteContainerSeconds(Params);

    llvm::DeleteContainerSeconds(BuiltinImpls);
    llvm::DeleteContainerSeconds(Strategies);
    llvm::DeleteContainerSeconds(Reductions);
    llvm::DeleteContainerSeconds(Decls);
  }

public:
  const OCLBuiltin &getBuiltin(llvm::Record &R) {
    if (!Builtins.count(&R))
      BuildBuiltin(R);

    return *Builtins[&R];
  }

  const OCLTypeConstraint &getTypeConstr(llvm::Record &R) {
    if (!Constraints.count(&R))
      BuildConstraint(R);

    return *Constraints[&R];
  }

  const OCLParam &getParam(llvm::Record &R) {
    if (!Params.count(&R))
      BuildParam(R);
    
    return *Params[&R];
  }  

  const OCLBuiltinImpl &getBuiltinImpl(llvm::Record &R) {
    if (!BuiltinImpls.count(&R))
      BuildBuiltinImpl(R);

    return *BuiltinImpls[&R];
  }

  const OCLStrategy &getStrategy(llvm::Record &R) {
    if (!Strategies.count(&R))
      BuildStrategy(R);

    return *Strategies[&R];
  }

  const OCLDecl &getDecl(llvm::Record &R) {
    if (!Decls.count(&R))
      BuildDecl(R);

    return *Decls[&R];
  }

  const OCLReduction &getReduction(llvm::Record &R) {
    if (!Reductions.count(&R))
      BuildReduction(R);

    return *Reductions[&R];
  }

private:
  void BuildBuiltin(llvm::Record &R) {
    OCLBuiltin *Builtin = 0;

    assert(R.isSubClassOf("OCLBuiltin") && "Not a builtin!");

    std::string Name = R.getValueAsString("Name");
    std::string Group = R.getValueAsString("Group");

    OCLBuiltin::VariantsMap Variants;
    std::vector<llvm::Record *> Vars = R.getValueAsListOfDefs("Variants");
    if (R.isSubClassOf("OCLBuiltinVariant")) {
      assert(Vars.empty());
      Vars.push_back(&R);
    }
    for (unsigned i = 0, e = Vars.size(); i != e; ++i)
      BuildBuiltinVariant(*Vars[i], Variants);

    if (R.isSubClassOf("OCLGenericBuiltin"))
      Builtin = new OCLGenericBuiltin(Group, Name, Variants);
    else if (R.isSubClassOf("OCLReinterpretBuiltin"))
      Builtin = new OCLReinterpretBuiltin(Group, Name, Variants);
    else if (R.isSubClassOf("OCLConvertBuiltin")) {
      llvm::StringRef Rounding = R.getValueAsString("Rounding");
      bool Saturation = R.getValueAsBit("Saturation");
      Builtin = new OCLConvertBuiltin(Group, Name, Saturation, 
                                      Rounding, Variants);
    } else
      llvm::PrintFatalError("Invalid builtin: " + R.getName());

    Builtins[&R] = Builtin;
  }

  void BuildBuiltinVariant(llvm::Record &R, OCLBuiltin::VariantsMap &Var) {
    std::vector<llvm::Record*> Ops = R.getValueAsListOfDefs("Operands");
    OCLBuiltinVariant::OperandsContainer Operands;
    Operands.reserve(Ops.size());
    for (unsigned i = 0, e = Ops.size(); i != e; ++i)
      Operands.push_back(&OCLTypesTable::get(*Ops[i]));

    std::vector<llvm::Record*> Constrs = R.getValueAsListOfDefs("Constraints");
    OCLBuiltinVariant::ConstraintsContainer ConstrVec;
    ConstrVec.reserve(Constrs.size());
    for (unsigned i = 0, e = Constrs.size(); i != e; ++i)
      ConstrVec.push_back(&getTypeConstr(*Constrs[i]));

    llvm::StringRef VarName = R.getValueAsString("VariantName");

    Var[VarName] = new OCLBuiltinVariant(Operands, ConstrVec, VarName);
  }

  void BuildConstraint(llvm::Record &R) {
    OCLTypeConstraint *Constraint = 0;

    assert(R.isSubClassOf("OCLTypeConstraint") && "Not a constraint!");

    if (R.isSubClassOf("OCLBinaryTypeConstraint")) {
      const OCLParam &P1 = getParam(*R.getValueAsDef("FirstOperand"));
      const OCLParam &P2 = getParam(*R.getValueAsDef("SecondOperand"));

      if (R.isSubClassOf("isSameAs"))
        Constraint = new OCLSameTypeConstraint(P1, P2);
      else if (R.isSubClassOf("isSameDimAs"))
        Constraint = new OCLSameDimTypeConstraint(P1, P2);
      else if (R.isSubClassOf("isSameBaseAs"))
        Constraint = new OCLSameBaseTypeConstraint(P1, P2);
      else if (R.isSubClassOf("isSameBaseSizeAs"))
        Constraint = new OCLSameBaseSizeTypeConstraint(P1, P2);
      else if (R.isSubClassOf("isSameBaseKindAs"))
        Constraint = new OCLSameBaseKindTypeConstraint(P1, P2);
      else
        llvm::PrintFatalError("Invalid TypeConstraint: " + R.getName());
    }
    else if (R.isSubClassOf("Not")) {
      const OCLTypeConstraint &Basic = getTypeConstr(*R.getValueAsDef("Basic"));

      Constraint = new OCLNotTypeConstraint(Basic);
    }
    else
      llvm::PrintFatalError("Invalid TypeConstraint: " + R.getName());

    Constraints[&R] = Constraint;
  }

  void BuildParam(llvm::Record &R) {
    OCLParam *Param = 0;

    assert(R.isSubClassOf("OCLParam") && "Not a param!");

    unsigned Op = R.getValueAsInt("Operand");

    if (R.isSubClassOf("Id")) {
      Param = new OCLParamId(Op);
    } else if (R.isSubClassOf("Pointee")) {
      unsigned Nest = R.getValueAsInt("Nest");

      if (Nest == 0) 
        llvm::PrintFatalError("Invalid 'nest' pointee level!");

      Param = new OCLParamPointee(Op, Nest);
    } else
      llvm::PrintFatalError("Invalid Param: " + R.getName());

    Params[&R] = Param;
  }

  void BuildBuiltinImpl(llvm::Record &R) {
    OCLBuiltinImpl *BuiltinImpl = 0;

    assert(R.isSubClassOf("OCLBuiltinImpl") && "Not a builtin implementation!");

    const OCLBuiltin &Builtin = getBuiltin(*R.getValueAsDef("BuiltIn"));
    const OCLStrategy &Strategy = getStrategy(*R.getValueAsDef("Strategy"));
    llvm::StringRef VarName = R.getValueAsString("VariantName");
    bool IsTarget = R.getValueAsBit("isTarget");

    BuiltinImpl = new OCLBuiltinImpl(Builtin, Strategy, VarName, IsTarget);

    BuiltinImpls[&R] = BuiltinImpl;
  }

  void BuildDecl(llvm::Record &R) {
    OCLDecl *Decl = 0;

    if (R.isSubClassOf("OCLTypedef")) {
      llvm::StringRef Name = R.getValueAsString("Name");
      const OCLParam &Param = getParam(*R.getValueAsDef("Param"));
      if (R.isSubClassOf("TypedefId"))
        Decl = new OCLTypedefIdDecl(Name, Param);
      else if (R.isSubClassOf("TypedefUnsigned"))
        Decl = new OCLTypedefUnsignedDecl(Name, Param);
      else
        llvm::PrintFatalError("Invalid OCLTypedef: " + R.getName());
    }
    else if (R.isSubClassOf("OCLTypeValue")) {
      llvm::StringRef ID = R.getValueAsString("ID");
      const OCLParam &Param = getParam(*R.getValueAsDef("Param"));
      if (R.isSubClassOf("MinValue"))
        Decl = new OCLMinValueDecl(ID, Param);
      else if (R.isSubClassOf("MaxValue"))
        Decl = new OCLMaxValueDecl(ID, Param);
      else
        llvm::PrintFatalError("Invalid OCLTypeValue: " + R.getName());
    }
    else
      llvm::PrintFatalError("Invalid OCLDecl: " + R.getName());

    Decls[&R] = Decl;
  }

  void BuildReduction(llvm::Record &R) {
    OCLReduction *Red = 0;
    
    if (R.isSubClassOf("InfixBinAssocReduction")) {
      Red = new OCLInfixBinAssocReduction(R.getValueAsString("Operator"));
    } else if (R.getName() == "OCLDefaultReduction") {
      Red = 0;
    } else
      llvm::PrintFatalError("Invalid OCLReduction: " + R.getName());

    Reductions[&R] = Red;
  }

  void BuildStrategy(llvm::Record &R) {
    OCLStrategy *Strategy = 0;

    std::vector<llvm::Record *> Declarations =
      R.getValueAsListOfDefs("Declarations");
    OCLStrategy::DeclsContainer Decls;
    Decls.reserve(Declarations.size());
    for (unsigned I = 0, E = Declarations.size(); I != E; ++I)
      Decls.push_back(&getDecl(*Declarations[I]));

    if (R.isSubClassOf("RecursiveSplit")) {
      llvm::StringRef ScalarImpl = R.getValueAsString("ScalarImpl");
      const OCLReduction *Red = &getReduction(*R.getValueAsDef("Reduction"));
      Strategy = new OCLRecursiveSplit(ScalarImpl, Decls, Red); 
    } else if (R.isSubClassOf("DirectSplit")) {
      llvm::StringRef ScalarImpl = R.getValueAsString("ScalarImpl");
      Strategy = new OCLDirectSplit(ScalarImpl, Decls);
    } else if (R.isSubClassOf("TemplateStrategy")) {
      llvm::StringRef TemplateImpl = R.getValueAsString("TemplateImpl");
      Strategy = new OCLTemplateStrategy(TemplateImpl, Decls);
    } else
      llvm::PrintFatalError("Invalid OCLStrategy: " + R.getName());

    Strategies[&R] = Strategy;
  }

private:
  OCLBuiltinsMap Builtins;
  OCLConstraintsMap Constraints;
  OCLParamsMap Params;

  OCLBuiltinImplsMap BuiltinImpls;
  OCLStrategiesMap Strategies;
  OCLReductionsMap Reductions;
  OCLDeclsMap Decls;
};

llvm::OwningPtr<OCLBuiltinsTableImpl> OCLBuiltinsTable::Impl;

const OCLBuiltin &OCLBuiltinsTable::getBuiltin(llvm::Record &R) {
  if (!Impl) Impl.reset(new OCLBuiltinsTableImpl());
  return Impl->getBuiltin(R);
}

const OCLBuiltinImpl &OCLBuiltinsTable::getBuiltinImpl(llvm::Record &R) {
  if (!Impl) Impl.reset(new OCLBuiltinsTableImpl());
  return Impl->getBuiltinImpl(R);
}

//===----------------------------------------------------------------------===//
// Builtin signature expander
//===----------------------------------------------------------------------===//

typedef std::multimap<unsigned, const OCLTypeConstraint* > ConstraintMap;

static bool IsConsistent(const ConstraintMap &CMap, BuiltinSign &Sign) {
  unsigned LastOperand = Sign.size() - 1;

  for (ConstraintMap::const_iterator I = CMap.find(LastOperand), E = CMap.end(); 
       I != E && I->first == LastOperand; ++I)
    if (!I->second->apply(Sign))
      return false;

  return true;
}

static void EnumerateSigns(const OCLBuiltinVariant &B, unsigned Index, 
                           const ConstraintMap &CMap, BuiltinSign &Tmp, 
                           BuiltinSignList &R) {
  if (Index >= B.size()) {
    R.push_back(Tmp);
    return;
  }

  const OCLType &Cur = B.getOperand(Index);
  if (const OCLGroupType *GT = llvm::dyn_cast<OCLGroupType>(&Cur)) {
    for (OCLGroupType::const_iterator I = GT->begin(), E = GT->end();
         I != E; ++I) {
      Tmp.push_back(*I);
      if (IsConsistent(CMap, Tmp))
        EnumerateSigns(B, Index + 1, CMap, Tmp, R);
      Tmp.pop_back();
    }
  } else if (const OCLPointerType *PT = llvm::dyn_cast<OCLPointerType>(&Cur)) {
    OCLPointerGroupIterator I(*PT);
    OCLPointerGroupIterator E(*PT, true);

    for (; I != E; ++I) {
      Tmp.push_back(&*I);
      if (IsConsistent(CMap, Tmp))
        EnumerateSigns(B, Index + 1, CMap, Tmp, R);
      Tmp.pop_back();
    }
  } else {
    Tmp.push_back(llvm::cast<OCLBasicType>(&Cur));
    if (IsConsistent(CMap, Tmp))
      EnumerateSigns(B, Index + 1, CMap, Tmp, R);
    Tmp.pop_back();
  }
}

static ConstraintMap ComputeConstraintMap(const OCLBuiltinVariant &B) {
  ConstraintMap M;
  const OCLBuiltinVariant::ConstraintsContainer &V = B.getConstraints();
  for (unsigned i = 0, e = V.size(); i != e; ++i) {
    M.insert(std::make_pair(V[i]->getMaxOperand(), V[i]));
  }
  return M;
}

void opencrun::ExpandSigns(const OCLBuiltinVariant &B, BuiltinSignList &L) {
  BuiltinSign S;
  S.reserve(B.size());

  ConstraintMap CMap = ComputeConstraintMap(B);

  EnumerateSigns(B, 0, CMap, S, L);
}

//===----------------------------------------------------------------------===//
// Builtin signature ordering 
//===----------------------------------------------------------------------===//

bool BuiltinSignCompare::operator()(const BuiltinSign &S1, 
                                    const BuiltinSign &S2) const {
  unsigned i;
  unsigned e = std::min(S1.size(), S2.size());

  for (i = 0; i != e && S1[i] == S2[i]; ++i);

  if (i != e) return S1[i]->compareLess(S2[i]);
  return S1.size() < S2.size();
}
