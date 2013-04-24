class OCLParam<int op> {
  int Operand = op;
}

class Id<int op> : OCLParam<op>;

class Pointee<int op, int n = 1>
 : OCLParam<op> {
  int Nest = n;
}

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

class OCLBuiltinVariant<list<OCLType> ops, list<OCLTypeConstraint> constrs> {
  list<OCLType> Operands = ops;
  list<OCLTypeConstraint> Constraints = constrs;
}

class OCLBuiltin<string name> {
  string Name = name;
}

class OCLMultiBuiltin<string name, list<OCLBuiltinVariant> var>
 : OCLBuiltin<name> {
  list<OCLBuiltinVariant> Variants = var;
}

class OCLSimpleBuiltin<string name, list<OCLType> ops, 
                       list<OCLTypeConstraint> constrs>
  : OCLBuiltin<name>, OCLBuiltinVariant<ops, constrs>;

class OCLStrategy;

class RecursiveSplit<code scalarimpl> : OCLStrategy {
  code ScalarImplementation = scalarimpl;
}

class CustomImpl<list<OCLBasicType> params, code impl> {
  list<OCLBasicType> Params = params;
  code Implementation = impl;
}

class CustomStrategy<bit partial, list<CustomImpl> impls> {
  bit IsPartial = partial;
  list<CustomImpl> Implementations = impls;
}

class OCLBuiltinImpl<OCLBuiltinVariant b, OCLStrategy s> {
  OCLBuiltinVariant BuiltIn = b;
  OCLStrategy Strategy = s;
}
