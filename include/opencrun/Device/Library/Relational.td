
//===----------------------------------------------------------------------===//
// Relational builtin classes
//===----------------------------------------------------------------------===//

let Group = "RELATIONAL" in {

class RelationalBuiltin_CnSi<string name> : OCLSimpleBuiltin<
  name,
  [ocl_int, ocl_gentype_s_integer],
  []
>;

class RelationalBuiltin_Snf_Sid<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_s_int,
    ocl_gentype_float
  ],
  [isSameDimAs<Id<0>, Id<1>>], "Snf">,
  OCLBuiltinVariant<[
   ocl_s_long_rel,
   ocl_gentype_double
  ],
  [isSameDimAs<Id<0>, Id<1>>], "Sid">
]>;

class RelationalBuiltin_Snff_Sidd<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_s_int,
    ocl_gentype_float,
    ocl_gentype_float
  ],
  [isSameAs<Id<1>, Id<2>>, isSameDimAs<Id<0>, Id<1>>], "Snff">,
  OCLBuiltinVariant<[
   ocl_s_long_rel,
   ocl_gentype_double,
   ocl_gentype_double
  ],
  [isSameAs<Id<1>, Id<2>>, isSameDimAs<Id<0>, Id<1>>], "Sidd">
]>;

class RelationalBuiltin_iiii_ffff_dddd<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_integer,
    ocl_gentype_integer,
    ocl_gentype_integer,
    ocl_gentype_integer
  ],
  [
    isSameAs<Id<0>, Id<1>>, 
    isSameAs<Id<1>, Id<2>>, 
    isSameAs<Id<2>, Id<3>>
  ], "iiii">,
  OCLBuiltinVariant<[
    ocl_gentype_float,
    ocl_gentype_float,
    ocl_gentype_float,
    ocl_gentype_float
  ],
  [
    isSameAs<Id<0>, Id<1>>, 
    isSameAs<Id<1>, Id<2>>, 
    isSameAs<Id<2>, Id<3>>
  ], "ffff">,
  OCLBuiltinVariant<[
    ocl_gentype_double,
    ocl_gentype_double,
    ocl_gentype_double,
    ocl_gentype_double
  ],
  [
    isSameAs<Id<0>, Id<1>>, 
    isSameAs<Id<1>, Id<2>>, 
    isSameAs<Id<2>, Id<3>>
  ], "dddd">
]>;

class RelationalBuiltin_gggSi_gggUi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype, 
    ocl_gentype,
    ocl_gentype,
    ocl_gentype_s_integer
  ],
  [
    isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, 
    isSameDimAs<Id<2>, Id<3>>, 
    isSameBaseSizeAs<Id<2>, Id<3>>
  ], "gggSi">,
  OCLBuiltinVariant<[
    ocl_gentype,
    ocl_gentype,
    ocl_gentype,
    ocl_gentype_u_integer
  ],
  [
    isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, 
    isSameDimAs<Id<2>, Id<3>>,
    isSameBaseSizeAs<Id<2>, Id<3>>], "gggUi">
]>;

}

//===----------------------------------------------------------------------===//
// Common builtin declarations
//===----------------------------------------------------------------------===//

def blt_isequal         : RelationalBuiltin_Snff_Sidd<"isequal">;
def blt_isnotequal      : RelationalBuiltin_Snff_Sidd<"isnotequal">;
def blt_isgreater       : RelationalBuiltin_Snff_Sidd<"isgreater">;
def blt_isgreaterequal  : RelationalBuiltin_Snff_Sidd<"isgreaterequal">;
def blt_isless          : RelationalBuiltin_Snff_Sidd<"isless">;
def blt_islessequal     : RelationalBuiltin_Snff_Sidd<"islessequal">;
def blt_islessgreater   : RelationalBuiltin_Snff_Sidd<"islessgreater">;
def blt_isordered       : RelationalBuiltin_Snff_Sidd<"isordered">;
def blt_isunordered     : RelationalBuiltin_Snff_Sidd<"isunordered">;

def blt_isfinite        : RelationalBuiltin_Snf_Sid<"isfinite">;
def blt_isinf           : RelationalBuiltin_Snf_Sid<"isinf">;
def blt_isnan           : RelationalBuiltin_Snf_Sid<"isnan">;
def blt_isnormal        : RelationalBuiltin_Snf_Sid<"isnormal">;
def blt_signbit         : RelationalBuiltin_Snf_Sid<"signbit">;

def blt_any             : RelationalBuiltin_CnSi<"any">;
def blt_all             : RelationalBuiltin_CnSi<"all">;

def blt_bitselect       : RelationalBuiltin_iiii_ffff_dddd<"bitselect">;

def blt_select          : RelationalBuiltin_gggSi_gggUi<"select">;

//===----------------------------------------------------------------------===//
// Common builtin implementations
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_isequal, TemplateStrategy<[{
  return param1 == param2; 
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_isnotequal, TemplateStrategy<[{
  return param1 != param2;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_isgreater, TemplateStrategy<[{
  return param1 > param2;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_isgreaterequal, TemplateStrategy<[{
  return param1 >= param2;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_isless, TemplateStrategy<[{
  return param1 < param2;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_islessequal, TemplateStrategy<[{
  return param1 <= param2;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_islessgreater, TemplateStrategy<[{
  return (param1 < param2) || (param1 > param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_isordered, TemplateStrategy<[{
  return (param1 == param1) 
      && (param2 == param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_isunordered, TemplateStrategy<[{
  return (param1 != param1)
      || (param2 != param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_isfinite, TemplateStrategy<[{
  return (param1 == param1) 
      && (__builtin_ocl_fabs(param1) != HUGE_VALF);
}]>, "Snf">;

def : OCLBuiltinImpl<blt_isfinite, TemplateStrategy<[{
  return (param1 == param1) 
      && (__builtin_ocl_fabs(param1) != HUGE_VAL);
}]>, "Sid">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_isinf, TemplateStrategy<[{
  return __builtin_ocl_fabs(param1) == HUGE_VALF;
}]>, "Snf">;

def : OCLBuiltinImpl<blt_isinf, TemplateStrategy<[{
  return __builtin_ocl_fabs(param1) == HUGE_VAL;
}]>, "Sid">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_isnan, TemplateStrategy<[{
  return param1 != param1;  
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_isnormal, TemplateStrategy<[{
  return (param1 == param1) 
      && (__builtin_ocl_fabs(param1) < HUGE_VALF)
      && (__builtin_ocl_fabs(param1) >= FLT_MIN);
}]>, "Snf">;

def : OCLBuiltinImpl<blt_isnormal, TemplateStrategy<[{
  return (param1 == param1) 
      && (__builtin_ocl_fabs(param1) < HUGE_VAL)
      && (__builtin_ocl_fabs(param1) >= DBL_MIN);
}]>, "Sid">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_signbit, TemplateStrategy<[{
  return param1 < 0.0f;
}]>, "Snf">;

def : OCLBuiltinImpl<blt_signbit, TemplateStrategy<[{
  return param1 < 0.0;
}]>, "Sid">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_any, RecursiveSplitReduction<[{
  return param1 < 0;
}], InfixBinAssocReduction<[{ || }]>>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_all, RecursiveSplitReduction<[{
  return param1 < 0;
}], InfixBinAssocReduction<[{ && }]>>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_bitselect, RecursiveSplit<[{
  return (param1 & ~param3) | (param2 & param3);
}]>, "iiii">;

def : OCLBuiltinImpl<blt_bitselect, RecursiveSplit<[{
  typedef union _float_bits {
    float f;
    int i;
  } float_bits;

  float_bits param1_bits, param2_bits, param3_bits;
  float_bits RetValue;
  
  param1_bits.f = param1;
  param2_bits.f = param2;
  param3_bits.f = param3;

  RetValue.f = 0.0f;

  RetValue.i = (param1_bits.i & ~param3_bits.i) | 
               (param2_bits.i & param3_bits.i);

  return RetValue.f;
}]>, "ffff">;

def : OCLBuiltinImpl<blt_bitselect, RecursiveSplit<[{
  typedef union _double_bits {
    double d;
    long l;
  } double_bits;

  double_bits param1_bits, param2_bits, param3_bits;
  double_bits RetValue;
  
  param1_bits.d = param1;
  param2_bits.d = param2;
  param3_bits.d = param3;

  RetValue.d = 0.0;

  RetValue.l = (param1_bits.l & ~param3_bits.l) |
               (param2_bits.l & param3_bits.l);

  return RetValue.d;
}]>, "dddd">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_select, TemplateStrategy<[{
  return param3 ? param2 : param1;
}]>>;


