//===----------------------------------------------------------------------===//
// Relational builtin classes
//===----------------------------------------------------------------------===//

let Group = "RELATIONAL" in {

class RelationalBuiltin_gggg<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype, ocl_gentype, ocl_gentype, ocl_gentype], 
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>]
>;

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

def blt_bitselect       : RelationalBuiltin_gggg<"bitselect">;

def blt_select          : RelationalBuiltin_gggSi_gggUi<"select">;

//===----------------------------------------------------------------------===//
// Common builtin implementations
//===----------------------------------------------------------------------===//
