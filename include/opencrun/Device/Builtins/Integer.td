//===----------------------------------------------------------------------===//
// Integer builtin classes
//===----------------------------------------------------------------------===//

class IntegerBuiltin_Uii<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_u_integer, ocl_gentype_integer], 
  [isSameDimAs<Id<0>, Id<1>>, isSameBaseSizeAs<Id<0>, Id<1>>]
>;

class IntegerBuiltin_ii<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_integer, ocl_gentype_integer], 
  [isSameAs<Id<0>, Id<1>>]
>;

class IntegerBuiltin_Uiii<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_u_integer, ocl_gentype_integer, ocl_gentype_integer],
  [
    isSameDimAs<Id<0>, Id<1>>, 
    isSameBaseSizeAs<Id<0>, Id<1>>, 
    isSameBaseSizeAs<Id<0>, Id<2>>,
    isSameAs<Id<1>, Id<2>>
  ]
>;

class IntegerBuiltin_iii<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_integer, ocl_gentype_integer, ocl_gentype_integer],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>]
>;

class IntegerBuiltin_fast_iii<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_integer_int, ocl_gentype_integer_int, ocl_gentype_integer_int],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>]
>;

class IntegerBuiltin_iiii<string name> : OCLSimpleBuiltin<
  name, 
  [
    ocl_gentype_integer, ocl_gentype_integer, 
    ocl_gentype_integer, ocl_gentype_integer
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>]
>;

class IntegerBuiltin_fast_iiii<string name> : OCLSimpleBuiltin<
  name, 
  [
    ocl_gentype_integer_int, ocl_gentype_integer_int, 
    ocl_gentype_integer_int, ocl_gentype_integer_int
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>]
>;

class IntegerBuiltin_iii_ViViCi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_integer,
    ocl_gentype_integer,
    ocl_gentype_integer
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>]>,
  OCLBuiltinVariant<[
   ocl_vgentype_integer,
   ocl_vgentype_integer,
   ocl_sgentype_integer
  ],
  [isSameAs<Id<0>, Id<1>>, isSameBaseAs<Id<1>, Id<2>>]>
]>;

class IntegerBuiltin_SsScUc_UsUcUc_SiSsUs_UiUsUs_SlSiUi_UlUiUi<string name> : 
OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_s_short,
    ocl_s_char,
    ocl_u_char
  ],
  [
    isSameDimAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>, 
    isSameBaseSizeAs<Id<1>, Id<2>>
  ]>,
  OCLBuiltinVariant<[
   ocl_u_short,
   ocl_u_char,
   ocl_u_char
  ],
  [
    isSameDimAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>, 
    isSameAs<Id<1>, Id<2>>
  ]>,
  OCLBuiltinVariant<[
    ocl_s_int,
    ocl_s_short,
    ocl_u_short
  ],
  [
    isSameDimAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>, 
    isSameBaseSizeAs<Id<1>, Id<2>>
  ]>,
  OCLBuiltinVariant<[
   ocl_u_int,
   ocl_u_short,
   ocl_u_short
  ],
  [
    isSameDimAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>, 
    isSameAs<Id<1>, Id<2>>]>,
  OCLBuiltinVariant<[
    ocl_s_long,
    ocl_s_int,
    ocl_u_int
  ],
  [
    isSameDimAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>, 
    isSameBaseSizeAs<Id<1>, Id<2>>
  ]>,
  OCLBuiltinVariant<[
   ocl_u_long,
   ocl_u_int,
   ocl_u_int
  ],
  [
    isSameDimAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>, 
    isSameAs<Id<1>, Id<2>>
  ]>
]>;

class IntegerBuiltin_iiii_ViViCiCi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_integer,
    ocl_gentype_integer,
    ocl_gentype_integer,
    ocl_gentype_integer
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>]>,
  OCLBuiltinVariant<[
   ocl_vgentype_integer,
   ocl_vgentype_integer,
   ocl_sgentype_integer,
   ocl_sgentype_integer
  ],
  [
    isSameAs<Id<0>, Id<1>>, isSameAs<Id<2>, Id<3>>, 
    isSameBaseAs<Id<1>, Id<2>>
  ]>
]>;

//===----------------------------------------------------------------------===//
// Integer builtin declarations
//===----------------------------------------------------------------------===//

def blt_abs         : IntegerBuiltin_Uii<"abs">;

def blt_clz         : IntegerBuiltin_ii<"clz">;
def blt_popcount    : IntegerBuiltin_ii<"popcount">;

def blt_abs_diff    : IntegerBuiltin_Uiii<"abs_diff">;

def blt_add_sat     : IntegerBuiltin_iii<"add_sat">;
def blt_hadd        : IntegerBuiltin_iii<"hadd">;
def blt_rhadd       : IntegerBuiltin_iii<"rhadd">;
def blt_mul_hi      : IntegerBuiltin_iii<"mul_hi">;
def blt_rotate      : IntegerBuiltin_iii<"rotate">;
def blt_sub_sat     : IntegerBuiltin_iii<"sub_sat">;

def blt_mul24       : IntegerBuiltin_fast_iii<"mul24">;

def blt_mad_hi      : IntegerBuiltin_iiii<"mad_hi">;
def blt_mad_sat     : IntegerBuiltin_iiii<"mad_sat">;

def blt_mad24       : IntegerBuiltin_fast_iiii<"mad24">;

def blt_max_int     : IntegerBuiltin_iii_ViViCi<"max">;
def blt_min_int     : IntegerBuiltin_iii_ViViCi<"min">;

def blt_upsample    : IntegerBuiltin_SsScUc_UsUcUc_SiSsUs_UiUsUs_SlSiUi_UlUiUi<"upsample">;

def blt_clamp_int   : IntegerBuiltin_iiii_ViViCiCi<"clamp">;

