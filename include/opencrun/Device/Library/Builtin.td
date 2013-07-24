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

class OCLBuiltinVariant<list<OCLType> ops, list<OCLTypeConstraint> constrs,
                        string name> {
  string VariantName = name;
  list<OCLType> Operands = ops;
  list<OCLTypeConstraint> Constraints = constrs;
}

class OCLBuiltin<string name> {
  string Name = name;
  string Group = "";
}

class OCLSimpleBuiltin<string name, list<OCLType> ops, 
                       list<OCLTypeConstraint> constrs>
  : OCLBuiltin<name>, OCLBuiltinVariant<ops, constrs, "">;

class OCLMultiBuiltin<string name, list<OCLBuiltinVariant> var>
 : OCLBuiltin<name> {
  list<OCLBuiltinVariant> Variants = var;
}

//----------------------------------------------------------------------------//

class OCLDeclaration;

class OCLTypedef<string name, OCLParam param> : OCLDeclaration {
  string Name = name;
  OCLParam Param = param;
}

class TypedefId<string typename, OCLParam param>
 : OCLTypedef<typename, param>;

class TypedefUnsigned<string typename, OCLParam param>
 : OCLTypedef<typename, param>;


class OCLStrategy<list<OCLDeclaration> decls> {
  list<OCLDeclaration> Declarations = decls;
}

class OCLReduction;

def OCLDefaultReduction : OCLReduction;

class InfixBinAssocReduction<code op> : OCLReduction {
  code Operator = op;
}

class RecursiveSplit<code scalarimpl, list<OCLDeclaration> decls = []> 
 : OCLStrategy<decls> {
  code ScalarImpl = scalarimpl;
  OCLReduction Reduction = OCLDefaultReduction;
}

class RecursiveSplitReduction<code scalarimpl, OCLReduction red,
                              list<OCLDeclaration> decls = []> 
 : RecursiveSplit<scalarimpl, decls> {
  let Reduction = red;
}

class DirectSplit<code scalarimpl, list<OCLDeclaration> decls = []> 
 : OCLStrategy<decls> {
  code ScalarImpl = scalarimpl;
}

class TemplateStrategy<code impl, list<OCLDeclaration> decls = []>
  : OCLStrategy<decls> {
  code TemplateImpl = impl;
}

class OCLBuiltinImpl<OCLBuiltin builtin, OCLStrategy s, string varname = ""> {
  OCLBuiltin BuiltIn = builtin;
  OCLStrategy Strategy = s;
  string VariantName = varname;
  bit isTarget = 0;
}

class OCLTargetBuiltinImpl<OCLBuiltin builtin, OCLStrategy s, 
                           string varname = "">
 : OCLBuiltinImpl<builtin, s, varname> {
  let isTarget = 1;
}
