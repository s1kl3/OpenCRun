class OCLParam<int op> {
  int Operand = op;
}

class Id<int op> : OCLParam<op>;

class Pointee<int op, int n = 1>
 : OCLParam<op> {
  int Nest = n;
}

//----------------------------------------------------------------------------//

class OCLTypeConstraint;

class OCLBinaryTypeConstraint<OCLParam op1, OCLParam op2>
 : OCLTypeConstraint {
  OCLParam FirstOperand = op1;
  OCLParam SecondOperand = op2;
}

class isSameAs<OCLParam op1, OCLParam op2>
 : OCLBinaryTypeConstraint<op1, op2>;

class isSameDimAs<OCLParam op1, OCLParam op2>
 : OCLBinaryTypeConstraint<op1, op2>;

class isSameBaseAs<OCLParam op1, OCLParam op2>
 : OCLBinaryTypeConstraint<op1, op2>;

class isSameBaseSizeAs<OCLParam op1, OCLParam op2>
 : OCLBinaryTypeConstraint<op1, op2>;

class isSameBaseKindAs<OCLParam op1, OCLParam op2>
 : OCLBinaryTypeConstraint<op1, op2>;

//----------------------------------------------------------------------------//

class OCLEmitTypedef<string typename, OCLParam op> {
  string TypeName = typename;
  OCLParam Op = op;
}

class OCLEmitTypedefActual<string typename, OCLParam op>
  : OCLEmitTypedef<typename, op>;

class OCLEmitTypedefUnsigned<string typename, OCLParam op>
  : OCLEmitTypedef<typename, op>;

//----------------------------------------------------------------------------//

class OCLStrategy<code scalarimpl, list<OCLEmitTypedef> typedefs> {
  code ScalarImpl = scalarimpl;
  list<OCLEmitTypedef> Typedefs = typedefs;
}

class RecursiveSplit<code scalarimpl, list<OCLEmitTypedef> typedefs = []> 
  : OCLStrategy<scalarimpl, typedefs>;

class DirectSplit<code scalarimpl, list<OCLEmitTypedef> typedefs = []> 
  : OCLStrategy<scalarimpl, typedefs>;

//----------------------------------------------------------------------------//

class OCLBuiltinVariant<list<OCLType> ops, 
                        list<OCLTypeConstraint> constrs, 
                        string variantid = ""> {
  list<OCLType> Operands = ops;
  list<OCLTypeConstraint> Constraints = constrs;
  string VariantId = variantid;
}

//----------------------------------------------------------------------------//

class OCLBuiltin<string name> {
  string Name = name;
}

class OCLSimpleBuiltin<string name, list<OCLType> ops, 
                       list<OCLTypeConstraint> constrs>
  : OCLBuiltin<name>, OCLBuiltinVariant<ops, constrs>;

class OCLMultiBuiltin<string name, list<OCLBuiltinVariant> var>
 : OCLBuiltin<name> {
  list<OCLBuiltinVariant> Variants = var;
}

//----------------------------------------------------------------------------//

class OCLBuiltinImpl<OCLBuiltin builtin, OCLStrategy s, string variantid = ""> {
  OCLBuiltin BuiltIn = builtin;
  OCLStrategy Strategy = s;
  string VariantId = variantid;
}

class CustomImpl<list<OCLBasicType> params, code impl> {
  list<OCLBasicType> Params = params;
  code Implementation = impl;
}

class CustomStrategy<bit partial, list<CustomImpl> impls> {
  bit IsPartial = partial;
  list<CustomImpl> Implementations = impls;
}


