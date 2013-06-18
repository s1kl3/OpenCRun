//===----------------------------------------------------------------------===//
// Integer builtin classes
//===----------------------------------------------------------------------===//

let Group = "INTEGER" in {

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

class IntegerBuiltin_UiUi_UiSi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_u_integer, 
    ocl_gentype_u_integer
  ],
  [isSameAs<Id<0>, Id<1>>],
  "UiUi">,
  OCLBuiltinVariant<[
    ocl_gentype_u_integer,
    ocl_gentype_s_integer
  ],
  [isSameDimAs<Id<0>, Id<1>>, isSameBaseSizeAs<Id<0>, Id<1>>],
  "UiSi">
]>;

class IntegerBuiltin_UiUiUi_SiSiSi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_u_integer,
    ocl_gentype_u_integer,
    ocl_gentype_u_integer
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>],
  "UiUiUi">,
  OCLBuiltinVariant<[
    ocl_gentype_s_integer,
    ocl_gentype_s_integer,
    ocl_gentype_s_integer
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>],
  "SiSiSi">
]>;

class IntegerBuiltin_UiUiUiUi_SiSiSiSi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_u_integer,
    ocl_gentype_u_integer,
    ocl_gentype_u_integer,
    ocl_gentype_u_integer
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>],
  "UiUiUiUi">,
  OCLBuiltinVariant<[
    ocl_gentype_s_integer,
    ocl_gentype_s_integer,
    ocl_gentype_s_integer,
    ocl_gentype_s_integer
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>],
  "SiSiSiSi">
]>;

class IntegerBuiltin_iii_ViViCi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_integer,
    ocl_gentype_integer,
    ocl_gentype_integer
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>],
  "iii">,
  OCLBuiltinVariant<[
   ocl_vgentype_integer,
   ocl_vgentype_integer,
   ocl_sgentype_integer
  ],
  [isSameAs<Id<0>, Id<1>>, isSameBaseAs<Id<1>, Id<2>>],
  "ViViCi">
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
  ],
  "SsScUc">,
  OCLBuiltinVariant<[
   ocl_u_short,
   ocl_u_char,
   ocl_u_char
  ],
  [
    isSameDimAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>, 
    isSameAs<Id<1>, Id<2>>
  ],
  "UsUcUc">,
  OCLBuiltinVariant<[
    ocl_s_int,
    ocl_s_short,
    ocl_u_short
  ],
  [
    isSameDimAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>, 
    isSameBaseSizeAs<Id<1>, Id<2>>
  ],
  "SiSsUs">,
  OCLBuiltinVariant<[
   ocl_u_int,
   ocl_u_short,
   ocl_u_short
  ],
  [
    isSameDimAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>, 
    isSameAs<Id<1>, Id<2>>
  ],
  "UiUsUs">,
  OCLBuiltinVariant<[
    ocl_s_long,
    ocl_s_int,
    ocl_u_int
  ],
  [
    isSameDimAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>, 
    isSameBaseSizeAs<Id<1>, Id<2>>
  ],
  "SlSiUi">,
  OCLBuiltinVariant<[
   ocl_u_long,
   ocl_u_int,
   ocl_u_int
  ],
  [
    isSameDimAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>, 
    isSameAs<Id<1>, Id<2>>
  ],
  "UlUiUi">
]>;

class IntegerBuiltin_iiii_ViViCiCi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_integer,
    ocl_gentype_integer,
    ocl_gentype_integer,
    ocl_gentype_integer
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>],
  "iiii">,
  OCLBuiltinVariant<[
   ocl_vgentype_integer,
   ocl_vgentype_integer,
   ocl_sgentype_integer,
   ocl_sgentype_integer
  ],
  [
    isSameAs<Id<0>, Id<1>>, isSameAs<Id<2>, Id<3>>, 
    isSameBaseAs<Id<1>, Id<2>>
  ],
  "ViViCiCi">
]>;

}

//===----------------------------------------------------------------------===//
// Integer builtin declarations
//===----------------------------------------------------------------------===//

def blt_abs         : IntegerBuiltin_UiUi_UiSi<"abs">;

def blt_clz         : IntegerBuiltin_ii<"clz">;
def blt_popcount    : IntegerBuiltin_ii<"popcount">;

def blt_abs_diff    : IntegerBuiltin_Uiii<"abs_diff">;

def blt_hadd        : IntegerBuiltin_iii<"hadd">;
def blt_rhadd       : IntegerBuiltin_iii<"rhadd">;
def blt_mul_hi      : IntegerBuiltin_iii<"mul_hi">;
def blt_rotate      : IntegerBuiltin_iii<"rotate">;

def blt_mul24       : IntegerBuiltin_fast_iii<"mul24">;

def blt_mad_hi      : IntegerBuiltin_iiii<"mad_hi">;

def blt_mad24       : IntegerBuiltin_fast_iiii<"mad24">;

def blt_add_sat     : IntegerBuiltin_UiUiUi_SiSiSi<"add_sat">;
def blt_sub_sat     : IntegerBuiltin_UiUiUi_SiSiSi<"sub_sat">;

def blt_mad_sat     : IntegerBuiltin_UiUiUiUi_SiSiSiSi<"mad_sat">;

def blt_max_int     : IntegerBuiltin_iii_ViViCi<"max">;
def blt_min_int     : IntegerBuiltin_iii_ViViCi<"min">;

def blt_clamp_int   : IntegerBuiltin_iiii_ViViCiCi<"clamp">;

def blt_upsample    : IntegerBuiltin_SsScUc_UsUcUc_SiSsUs_UiUsUs_SlSiUi_UlUiUi<"upsample">;

//===----------------------------------------------------------------------===//
// Integer builtin implementations
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_abs, RecursiveSplit<[{
  return param1;
}]>, "UiUi">;

def : OCLBuiltinImpl<blt_abs, RecursiveSplit<[{
  return param1 >= 0 ? param1 : -(param1);
}]>, "UiSi">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_clz, RecursiveSplit<[{
  TYPE shamt = sizeof(TYPE)*8/2;
  TYPE mask = (~0x0 << shamt);
  TYPE n = 0;

  if (param1 == 0)
    return sizeof(TYPE)*8;

  while (shamt >= 1) {
    if ((param1 & mask) == 0) {
      n += shamt;
      param1 <<= shamt;
    }

    shamt /= 2;
    mask <<= shamt;
  }

  return n;
}], [OCLEmitTypedefActual<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_popcount, RecursiveSplit<[{
  ulong x = (ulong)param1;

  const ulong m1  = 0x5555555555555555;
  const ulong m2  = 0x3333333333333333;
  const ulong m4  = 0x0f0f0f0f0f0f0f0f;
  const ulong h01 = 0x0101010101010101;

  x -= (x >> 1) & m1;
  x = (x & m2) + ((x >> 2) & m2);
  x = (x + (x >> 4)) & m4;
  return (x * h01) >> 56;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_abs_diff, RecursiveSplit<[{
  return param1 >= param2 ? (param1 - param2) : (param2 - param1); 
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_hadd, RecursiveSplit<[{
  return (param1 + param2) >> 1;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_rhadd, RecursiveSplit<[{
  return (param1 + param2 + 1) >> 1;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_mul_hi, RecursiveSplit<[{
  TYPE shamt = sizeof(TYPE)*8/2;
  TYPE lo_mask = (1 << shamt) - 1;

  TYPE xl = param1 & lo_mask;
  TYPE xh = param1 >> shamt;
  TYPE yl = param2 & lo_mask;
  TYPE yh = param2 >> shamt;

  UTYPE xy_ll = xl * yl;
  UTYPE xy_lh = xl * yh;
  UTYPE xy_hl = xh * yl;
  UTYPE xy_hh = xh * yh;

  UTYPE lo_tmp = xy_ll + (xy_lh << shamt);
  UTYPE c = lo_tmp < xy_ll;
  UTYPE lo = lo_tmp + (xy_hl << shamt);
  c += lo < lo_tmp;

  return xy_hh + ((TYPE)xy_lh >> shamt) + ((TYPE)xy_hl >> shamt) + c;  
}], [OCLEmitTypedefActual<"TYPE", Id<0>>, OCLEmitTypedefUnsigned<"UTYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_rotate, RecursiveSplit<[{
  return (param1 << param2) | (param1 >> ((sizeof(param1) * 8) - param2));
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_mul24, RecursiveSplit<[{
  return param1 * param2;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_mad_hi, RecursiveSplit<[{
  return __builtin_ocl_mul_hi(param1, param2) + param3;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_mad24, RecursiveSplit<[{
  return __builtin_ocl_mul24(param1, param2) + param3;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_add_sat, RecursiveSplit<[{
  TYPE max_val = ~0;

  return (param1 > (max_val - param2)) ? max_val : param1 + param2;
}], [OCLEmitTypedefActual<"TYPE", Id<0>>]>, "UiUiUi">;

def : OCLBuiltinImpl<blt_add_sat, RecursiveSplit<[{
  UTYPE max_val = (UTYPE)~0 >> 1; 
  UTYPE min_val = 1 << (sizeof(TYPE)*8 - 1);
  
  if ((param2 > 0) && (param1 > (TYPE)max_val - param2))
    return max_val;
  else if ((param2 < 0) && (param1 < (TYPE)min_val - param2))
    return min_val;
  else
    return param1 + param2;
}],
[OCLEmitTypedefActual<"TYPE", Id<0>>, OCLEmitTypedefUnsigned<"UTYPE", Id<0>>]>,
"SiSiSi">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sub_sat, RecursiveSplit<[{
  return (param1 < param2) ? 0 : param1 - param2;
}]>, "UiUiUi">;

def : OCLBuiltinImpl<blt_sub_sat, RecursiveSplit<[{
  UTYPE max_val = (UTYPE)~0 >> 1;
  UTYPE min_val = 1 << (sizeof(TYPE)*8 - 1);

  if ((param2 > 0) && (param1 < (TYPE)min_val + param2))
    return min_val;
  else if ((param2 < 0) && (param1 > (TYPE)max_val + param2))
    return max_val;
  else
    return param1 - param2;
}],
[OCLEmitTypedefActual<"TYPE", Id<0>>, OCLEmitTypedefUnsigned<"UTYPE", Id<0>>]>, 
"SiSiSi">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_mad_sat, RecursiveSplit<[{
  if ((param1 == 0) || (param2 == 0))
    return param3;

  TYPE max_val = ~0;

  return ((param2 != 0) && (param1 > (max_val - param3)/param2)) ?
    max_val : param1 * param2 + param3;
}], [OCLEmitTypedefActual<"TYPE", Id<0>>]>, "UiUiUiUi">;

def : OCLBuiltinImpl<blt_mad_sat, RecursiveSplit<[{
  if ((param1 == 0) || (param2 == 0))
    return param3;

  UTYPE max_val = (UTYPE)~0 >> 1;
  UTYPE min_val = 1 << (sizeof(TYPE)*8 - 1);

  if ((param3 > 0) && (param1 > ((TYPE)max_val - param3)/param2))
    return max_val;
  else if ((param3 < 0) && (param1 < ((TYPE)min_val - param3)/param2))
    return min_val;
  else
    return param1 * param2 + param3;
}], 
[OCLEmitTypedefActual<"TYPE", Id<0>>, OCLEmitTypedefUnsigned<"UTYPE", Id<0>>]>,
"SiSiSiSi">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_max_int, RecursiveSplit<[{
  return param1 < param2 ? param2 : param1;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_min_int, RecursiveSplit<[{
  return param2 < param1 ? param2 : param1;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_clamp_int, RecursiveSplit<[{
  return __builtin_ocl_min(__builtin_ocl_max(param1, param2), param3);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_upsample, RecursiveSplit<[{
  return ((RETTYPE)param1 << (sizeof(param1) * 8)) | (RETTYPE)param2;
}], [OCLEmitTypedefActual<"RETTYPE", Id<0>>]>>;

