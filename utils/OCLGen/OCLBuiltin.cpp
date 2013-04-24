#include "OCLBuiltin.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Error.h"

#include <map>

using namespace opencrun;

OCLParam::~OCLParam() {
}

const OCLBasicType *OCLParamId::get(const BuiltinSignature &Ops) const {
  return Ops[getOperand()];
}

const OCLBasicType *OCLParamPointee::get(const BuiltinSignature &Ops) const {
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


void opencrun::LoadOCLBuiltins(const llvm::RecordKeeper &R, 
                               OCLBuiltinsContainer &B) {
  std::vector<llvm::Record *> RawVects = 
    R.getAllDerivedDefinitions("OCLBuiltin");

  for(unsigned I = 0, E = RawVects.size(); I < E; ++I)
    B.push_back(&OCLBuiltinsTable::get(*RawVects[I]));
}

//===----------------------------------------------------------------------===//
// Builtins table implementation
//===----------------------------------------------------------------------===//

class opencrun::OCLBuiltinsTableImpl {
public:
  typedef std::map<llvm::Record *, const OCLBuiltin *> OCLBuiltinsMap;
  typedef std::map<llvm::Record *, const OCLTypeConstraint *> OCLConstraintsMap;
  typedef std::map<llvm::Record *, const OCLParam *> OCLParamsMap;

public:
  ~OCLBuiltinsTableImpl() { llvm::DeleteContainerSeconds(Builtins); }

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

private:
  void BuildBuiltinVariant(llvm::Record &R, 
                           std::vector<OCLBuiltinVariant> &Var) {

    std::vector<llvm::Record*> Ops = R.getValueAsListOfDefs("Operands");
    std::vector<const OCLType *> Operands;
    Operands.reserve(Ops.size());
    for (unsigned i = 0, e = Ops.size(); i != e; ++i)
      Operands.push_back(&OCLTypesTable::get(*Ops[i]));

    std::vector<llvm::Record*> Constrs = R.getValueAsListOfDefs("Constraints");
    OCLBuiltinVariant::ConstraintVector ConstrVec;
    ConstrVec.reserve(Constrs.size());
    for (unsigned i = 0, e = Constrs.size(); i != e; ++i)
      ConstrVec.push_back(&getTypeConstr(*Constrs[i]));

    Var.push_back(OCLBuiltinVariant(Operands, ConstrVec));
  }

  void BuildBuiltin(llvm::Record &R) {
    OCLBuiltin *Builtin = 0;

    assert(R.isSubClassOf("OCLBuiltin") && "Not a builtin!");

    std::string Name = R.getValueAsString("Name");

    std::vector<OCLBuiltinVariant> Variants;
    if (R.isSubClassOf("OCLMultiBuiltin")) {
      std::vector<llvm::Record*> Vars = R.getValueAsListOfDefs("Variants");
      Variants.reserve(Vars.size());
      for (unsigned i = 0, e = Vars.size(); i != e; ++i) {
        BuildBuiltinVariant(*Vars[i], Variants);
      }
    } else if (R.isSubClassOf("OCLSimpleBuiltin")) {
      Variants.reserve(1);
      BuildBuiltinVariant(R, Variants);
    } else
      llvm::PrintFatalError("Invalid builtin: " + R.getName());

    Builtin = new OCLBuiltin(Name, Variants);

    Builtins[&R] = Builtin;
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
    } else
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

private:
  OCLBuiltinsMap Builtins;
  OCLConstraintsMap Constraints;
  OCLParamsMap Params;
};

llvm::OwningPtr<OCLBuiltinsTableImpl> OCLBuiltinsTable::Impl;

const OCLBuiltin &OCLBuiltinsTable::get(llvm::Record &R) {
  if (!Impl) Impl.reset(new OCLBuiltinsTableImpl());
  return Impl->getBuiltin(R);
}

//===----------------------------------------------------------------------===//
// Builtin signature expander
//===----------------------------------------------------------------------===//

typedef std::multimap<unsigned, const OCLTypeConstraint* > ConstraintMap;

static bool IsConsistent(const ConstraintMap &CMap, BuiltinSignature &Sign) {
  unsigned LastOperand = Sign.size() - 1;

  for (ConstraintMap::const_iterator I = CMap.find(LastOperand), E = CMap.end(); 
       I != E && I->first == LastOperand; ++I)
    if (!I->second->apply(Sign))
      return false;

  return true;
}

static void EnumerateSignature(const OCLBuiltinVariant &B, unsigned Index, 
                               const ConstraintMap &CMap,
                               BuiltinSignature &Tmp, 
                               std::list<BuiltinSignature> &R) {
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
        EnumerateSignature(B, Index + 1, CMap, Tmp, R);
      Tmp.pop_back();
    }
  } else if (const OCLPointerType *PT = llvm::dyn_cast<OCLPointerType>(&Cur)) {
    OCLPointerGroupIterator I(*PT);
    OCLPointerGroupIterator E(*PT, true);

    for (; I != E; ++I) {
      Tmp.push_back(&*I);
      if (IsConsistent(CMap, Tmp))
        EnumerateSignature(B, Index + 1, CMap, Tmp, R);
      Tmp.pop_back();
    }
  } else {
    Tmp.push_back(llvm::cast<OCLBasicType>(&Cur));
    if (IsConsistent(CMap, Tmp))
      EnumerateSignature(B, Index + 1, CMap, Tmp, R);
    Tmp.pop_back();
  }
}

static ConstraintMap ComputeConstraintMap(const OCLBuiltinVariant &B) {
  ConstraintMap M;
  const OCLBuiltinVariant::ConstraintVector &V = B.getConstraints();
  for (unsigned i = 0, e = V.size(); i != e; ++i) {
    M.insert(std::make_pair(V[i]->getMaxOperand(), V[i]));
  }
  return M;
}

void opencrun::ExpandSignature(const OCLBuiltinVariant &B, 
                               std::list<BuiltinSignature> &R) {
  BuiltinSignature Sign;
  Sign.reserve(B.size());

  ConstraintMap CMap = ComputeConstraintMap(B);

  EnumerateSignature(B, 0, CMap, Sign, R);
}
