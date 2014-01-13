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

class isSameBitSizeAs<OCLParam op1, OCLParam op2>
 : OCLBinaryTypeConstraint<op1, op2>;

class Not<OCLTypeConstraint c> : OCLTypeConstraint {
  OCLTypeConstraint Basic = c;
}

//----------------------------------------------------------------------------//

class OCLRoundingMode<string name, bits<2> defaultFor = 0b00> {
  string Name = name;
  bit DefaultForInteger = defaultFor{0};
  bit DefaultForReal = defaultFor{1};
}

def ocl_rm_rtz : OCLRoundingMode<"rtz", 0b01>;
def ocl_rm_rte : OCLRoundingMode<"rte", 0b10>;
def ocl_rm_rtp : OCLRoundingMode<"rtp">;
def ocl_rm_rtn : OCLRoundingMode<"rtn">;

class OCLBuiltinVariant<list<OCLType> ops, list<OCLTypeConstraint> constrs,
                        string name> {
  string VariantName = name;
  list<OCLType> Operands = ops;
  list<OCLTypeConstraint> Constraints = constrs;
}

class OCLBuiltin<string name, list<OCLBuiltinVariant> var> {
  string Name = name;
  string Group = "";
  list<OCLBuiltinVariant> Variants = var;
  list<OCLPredicate> Predicates = [];
}

class OCLGenericBuiltin<string name, list<OCLBuiltinVariant> var>
 : OCLBuiltin<name, var>;

class OCLConvertBuiltin<OCLRoundingMode rm, bit sat, 
                        list<OCLBuiltinVariant> var>
 : OCLBuiltin<"convert", var> {
  OCLRoundingMode RoundingMode = rm;
  bit Saturation = sat;
}

class OCLReinterpretBuiltin<list<OCLBuiltinVariant> var>
 : OCLBuiltin<"as", var>;

class OCLGenericBuiltinSimple<string name, list<OCLType> ops, 
                              list<OCLTypeConstraint> constrs>
  : OCLGenericBuiltin<name, []>, OCLBuiltinVariant<ops, constrs, "">;

class OCLConvertBuiltinSimple<OCLRoundingMode round, bit sat, list<OCLType> ops,
                              list<OCLTypeConstraint> constrs>
  : OCLConvertBuiltin<round, sat, []>, OCLBuiltinVariant<ops, constrs, "">;

class OCLReinterpretBuiltinSimple<list<OCLType> ops, 
                                  list<OCLTypeConstraint> constrs>
  : OCLReinterpretBuiltin<[]>, OCLBuiltinVariant<ops, constrs, "">;

//----------------------------------------------------------------------------//

class OCLDeclaration;

class LibM<OCLParam param, string suffix = ""> : OCLDeclaration {
  OCLParam Param = param;
  string Suffix = suffix;
}

class OCLTypedef<string name, OCLParam param> : OCLDeclaration {
  string Name = name;
  OCLParam Param = param;
}

class TypedefId<string typename, OCLParam param>
 : OCLTypedef<typename, param>;

class TypedefUnsigned<string typename, OCLParam param>
 : OCLTypedef<typename, param>;

class OCLTypeValue<string id, OCLParam param> : OCLDeclaration {
  string ID = id;
  OCLParam Param = param;
}

class MinValue<string id, OCLParam param>
 : OCLTypeValue<id, param>;

class MaxValue<string id, OCLParam param>
 : OCLTypeValue<id, param>;

class BuiltinName<string name, OCLBuiltin builtin> : OCLDeclaration {
  string Name = name;
  OCLBuiltin Builtin = builtin;
}

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

class OCLRequirement;

class OCLInclude<string filename> : OCLRequirement {
  string FileName = filename;
}

class OCLCodeBlock<code block> : OCLRequirement {
  code CodeBlock = block;
}

class OCLBuiltinImpl<OCLBuiltin builtin, OCLStrategy s, string varname = ""> {
  OCLBuiltin BuiltIn = builtin;
  OCLStrategy Strategy = s;
  string VariantName = varname;
  bit isTarget = 0;
  list<OCLRequirement> Requirements = [];
}

class OCLTargetBuiltinImpl<OCLBuiltin builtin, OCLStrategy s, 
                           string varname = "">
 : OCLBuiltinImpl<builtin, s, varname> {
  let isTarget = 1;
}
