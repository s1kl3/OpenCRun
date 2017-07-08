//===----------------------------------------------------------------------===//
// Math builtin classes
//===----------------------------------------------------------------------===//

let Group = "MATH" in {

class MathBuiltin_rr<string name> : OCLGenericBuiltinSimple<
  name, 
  [ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1>>]
>;

class MathBuiltin_ff<string name> : OCLGenericBuiltinSimple<
  name, 
  [ocl_gentype_float, ocl_gentype_float], 
  [isSameAs<Id<0>, Id<1>>]
>;

class MathBuiltin_rrr<string name> : OCLGenericBuiltinSimple<
  name, 
  [ocl_gentype_real, ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>]
>;

class MathBuiltin_fff<string name> : OCLGenericBuiltinSimple<
  name, 
  [ocl_gentype_float, ocl_gentype_float, ocl_gentype_float], 
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>]
>;

class MathBuiltin_rrrr<string name> : OCLGenericBuiltinSimple<
  name, 
  [ocl_gentype_real, ocl_gentype_real, ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>]
>;

class MathBuiltin_rrSn<string name> : OCLGenericBuiltinSimple<name,
  [ocl_gentype_real, ocl_gentype_real, ocl_s_int], 
  [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>]
>;

class MathBuiltin_Snr<string name> : OCLGenericBuiltinSimple<name, 
  [ocl_s_int, ocl_gentype_real], 
  [isSameDimAs<Id<0>, Id<1>>]
>;

class MathBuiltin_fUn_dUl<string name> : OCLGenericBuiltin<name, [ 
  OCLBuiltinVariant<[
    ocl_gentype_float, 
    ocl_u_int
  ],
  [isSameDimAs<Id<0>, Id<1>>, isSameBaseSizeAs<Id<0>, Id<1>>], "fUn">,
  OCLBuiltinVariant<[
    ocl_gentype_double, 
    ocl_u_long
  ],
  [isSameDimAs<Id<0>, Id<1>>, isSameBaseSizeAs<Id<0>, Id<1>>], "dUl">
]>;

class MathBuiltin_ffSn_ddSn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_float, 
    ocl_gentype_float, 
    ocl_s_int
  ], 
  [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>], "ffSn">,
  OCLBuiltinVariant<[
    ocl_gentype_double, 
    ocl_gentype_double, 
    ocl_s_int
  ], 
  [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>], "ddSn">
]>;

class MathBuiltin_ff_dd<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_float,
    ocl_gentype_float
  ], 
  [isSameAs<Id<0>, Id<1>>], "ff">,
  OCLBuiltinVariant<[
    ocl_gentype_double,
    ocl_gentype_double
  ], 
  [isSameAs<Id<0>, Id<1>>], "dd">
]>;

class MathBuiltin_fff_ddd<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_float,
    ocl_gentype_float,
    ocl_gentype_float
  ], 
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>], "fff">,
  OCLBuiltinVariant<[
    ocl_gentype_double,
    ocl_gentype_double,
    ocl_gentype_double
  ], 
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>], "ddd">
]>;

class MathBuiltin_rrr_VfVfCf_VdVdCd<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real,
    ocl_gentype_real,
    ocl_gentype_real
  ], 
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>], "rrr">,
  OCLBuiltinVariant<[
    ocl_vgentype_float,
    ocl_vgentype_float,
    ocl_float
  ], 
  [isSameAs<Id<0>, Id<1>>], "ffCf">,
  OCLBuiltinVariant<[
    ocl_vgentype_double,
    ocl_vgentype_double,
    ocl_double
  ], 
  [isSameAs<Id<0>, Id<1>>], "ddCd">
]>;

class MathBuiltin_rrP1Sn_rrP2Sn_rrP4Sn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_global, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "rrP1Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_local, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "rrP2Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_private, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "rrP4Sn">
]>;

class MathBuiltin_ffP1Sn_ffP2Sn_ffP4Sn_ddP1Sn_ddP2Sn_ddP4Sn<string name> : 
OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_gentype_float, 
     ocl_gentype_float, 
     OCLPointerType<ocl_s_int, ocl_as_global, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "ffP1Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_float, 
     ocl_gentype_float, 
     OCLPointerType<ocl_s_int, ocl_as_local, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "ffP2Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_float, 
     ocl_gentype_float, 
     OCLPointerType<ocl_s_int, ocl_as_private, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "ffP4Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_double, 
     ocl_gentype_double, 
     OCLPointerType<ocl_s_int, ocl_as_global, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "ddP1Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_double, 
     ocl_gentype_double, 
     OCLPointerType<ocl_s_int, ocl_as_local, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "ddP2Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_double, 
     ocl_gentype_double, 
     OCLPointerType<ocl_s_int, ocl_as_private, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "ddP4Sn">
]>;

class MathBuiltin_rrrP1Sn_rrrP2Sn_rrrP4Sn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_global, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, 
      isSameDimAs<Id<2>, Pointee<3>>], "rrrP1Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_local, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, 
      isSameDimAs<Id<2>, Pointee<3>>], "rrrP2Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_private, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, 
      isSameDimAs<Id<2>, Pointee<3>>], "rrrP4Sn">
]>;

class MathBuiltin_VrVrCn_rrSn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_real, 
     ocl_vgentype_real, 
     ocl_int
  ], [isSameAs<Id<0>, Id<1>>], "VrVrCn">,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_s_int
  ], [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>], "rrSn">
]>;

class MathBuiltin_rrP1r_rrP2r_rrP4r<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real,
    ocl_gentype_real,
    OCLPointerType<ocl_gentype_real, ocl_as_global, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Pointee<2>>], "rrP1r">,
  OCLBuiltinVariant<[
    ocl_gentype_real,
    ocl_gentype_real,
    OCLPointerType<ocl_gentype_real, ocl_as_local, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Pointee<2>>], "rrP2r">,
  OCLBuiltinVariant<[
    ocl_gentype_real,
    ocl_gentype_real,
    OCLPointerType<ocl_gentype_real, ocl_as_private, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Pointee<2>>], "rrP4r">
]>;

class MathBuiltin_ffP1f_ffP2f_ffP4f_ddP1d_ddP2d_ddP4d<string name> : 
OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_float,
    ocl_gentype_float,
    OCLPointerType<ocl_gentype_float, ocl_as_global, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Pointee<2>>], "ffP1f">,
  OCLBuiltinVariant<[
    ocl_gentype_float,
    ocl_gentype_float,
    OCLPointerType<ocl_gentype_float, ocl_as_local, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Pointee<2>>], "ffP2f">,
  OCLBuiltinVariant<[
    ocl_gentype_float,
    ocl_gentype_float,
    OCLPointerType<ocl_gentype_float, ocl_as_private, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Pointee<2>>], "ffP4f">,
  OCLBuiltinVariant<[
    ocl_gentype_double,
    ocl_gentype_double,
    OCLPointerType<ocl_gentype_double, ocl_as_global, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Pointee<2>>], "ddP1d">,
  OCLBuiltinVariant<[
    ocl_gentype_double,
    ocl_gentype_double,
    OCLPointerType<ocl_gentype_double, ocl_as_local, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Pointee<2>>], "ddP2d">,
  OCLBuiltinVariant<[
    ocl_gentype_double,
    ocl_gentype_double,
    OCLPointerType<ocl_gentype_double, ocl_as_private, []>
  ], [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Pointee<2>>], "ddP4d">
]>;

}

//===----------------------------------------------------------------------===//
// Math builtin declarations
//===----------------------------------------------------------------------===//

def blt_acos        : MathBuiltin_rr<"acos">;
def blt_acosh       : MathBuiltin_rr<"acosh">;
def blt_asin        : MathBuiltin_rr<"asin">;
def blt_asinh       : MathBuiltin_rr<"asinh">;
def blt_atan        : MathBuiltin_rr<"atan">;
def blt_atanh       : MathBuiltin_rr<"atanh">;
def blt_cbrt        : MathBuiltin_rr<"cbrt">;
def blt_ceil        : MathBuiltin_rr<"ceil">;
def blt_cos         : MathBuiltin_rr<"cos">;
def blt_cosh        : MathBuiltin_rr<"cosh">;
def blt_erfc        : MathBuiltin_rr<"erfc">;
def blt_erf         : MathBuiltin_rr<"erf">;
def blt_exp         : MathBuiltin_rr<"exp">;
def blt_exp2        : MathBuiltin_rr<"exp2">;
def blt_expm1       : MathBuiltin_rr<"expm1">;
def blt_fabs        : MathBuiltin_rr<"fabs">;
def blt_floor       : MathBuiltin_rr<"floor">;
def blt_log         : MathBuiltin_rr<"log">;
def blt_log2        : MathBuiltin_rr<"log2">;
def blt_log10       : MathBuiltin_rr<"log10">;
def blt_log1p       : MathBuiltin_rr<"log1p">;
def blt_logb        : MathBuiltin_rr<"logb">;
def blt_lgamma      : MathBuiltin_rr<"lgamma">;
def blt_rint        : MathBuiltin_rr<"rint">;
def blt_round       : MathBuiltin_rr<"round">;
def blt_sin         : MathBuiltin_rr<"sin">;
def blt_sinh        : MathBuiltin_rr<"sinh">;
def blt_sqrt        : MathBuiltin_rr<"sqrt">;
def blt_tan         : MathBuiltin_rr<"tan">;
def blt_tanh        : MathBuiltin_rr<"tanh">;
def blt_tgamma      : MathBuiltin_rr<"tgamma">;
def blt_trunc       : MathBuiltin_rr<"trunc">;

def blt_cospi       : MathBuiltin_rr<"cospi">;
def blt_sinpi       : MathBuiltin_rr<"sinpi">;
def blt_tanpi       : MathBuiltin_rr<"tanpi">;
def blt_acospi      : MathBuiltin_rr<"acospi">;
def blt_asinpi      : MathBuiltin_rr<"asinpi">;
def blt_atanpi      : MathBuiltin_rr<"atanpi">;
def blt_exp10       : MathBuiltin_rr<"exp10">;
def blt_rsqrt       : MathBuiltin_rr<"rsqrt">;

def blt_atan2       : MathBuiltin_rrr<"atan2">;
def blt_copysign    : MathBuiltin_rrr<"copysign">;
def blt_fdim        : MathBuiltin_rrr<"fdim">;
def blt_fmod        : MathBuiltin_rrr<"fmod">;
def blt_hypot       : MathBuiltin_rrr<"hypot">;
def blt_maxmag      : MathBuiltin_rrr<"maxmag">;
def blt_minmag      : MathBuiltin_rrr<"minmag">;
def blt_nextafter   : MathBuiltin_rrr<"nextafter">;
def blt_pow         : MathBuiltin_rrr<"pow">;
def blt_powr        : MathBuiltin_rrr<"powr">;
def blt_remainder   : MathBuiltin_rrr<"remainder">;

def blt_atan2pi     : MathBuiltin_rrr<"atan2pi">;

def blt_fmin        : MathBuiltin_rrr_VfVfCf_VdVdCd<"fmin">;
def blt_fmax        : MathBuiltin_rrr_VfVfCf_VdVdCd<"fmax">;

def blt_fma         : MathBuiltin_rrrr<"fma">;
def blt_mad         : MathBuiltin_rrrr<"mad">;

def blt_pown        : MathBuiltin_rrSn<"pown">;
def blt_rootn       : MathBuiltin_ffSn_ddSn<"rootn">;

def blt_ldexp       : MathBuiltin_VrVrCn_rrSn<"ldexp">;

def blt_nan         : MathBuiltin_fUn_dUl<"nan">;

def blt_ilogb       : MathBuiltin_Snr<"ilogb">;

def blt_fract       : MathBuiltin_ffP1f_ffP2f_ffP4f_ddP1d_ddP2d_ddP4d<"fract">;

def blt_frexp       : MathBuiltin_rrP1Sn_rrP2Sn_rrP4Sn<"frexp">;

def blt_lgamma_r    : MathBuiltin_ffP1Sn_ffP2Sn_ffP4Sn_ddP1Sn_ddP2Sn_ddP4Sn<"lgamma_r">;

def blt_remquo      : MathBuiltin_rrrP1Sn_rrrP2Sn_rrrP4Sn<"remquo">;

def blt_modf        : MathBuiltin_ffP1f_ffP2f_ffP4f_ddP1d_ddP2d_ddP4d<"modf">;

def blt_sincos      : MathBuiltin_rrP1r_rrP2r_rrP4r<"sincos">;

// half_* versions - to be overridden in case of fast FPU
def blt_half_cos    : MathBuiltin_ff<"half_cos">;
def blt_half_divide : MathBuiltin_fff<"half_divide">;
def blt_half_exp    : MathBuiltin_ff<"half_exp">;
def blt_half_exp2   : MathBuiltin_ff<"half_exp2">;
def blt_half_exp10  : MathBuiltin_ff<"half_exp10">;
def blt_half_log    : MathBuiltin_ff<"half_log">;
def blt_half_log2   : MathBuiltin_ff<"half_log2">;
def blt_half_log10  : MathBuiltin_ff<"half_log10">;
def blt_half_powr   : MathBuiltin_fff<"half_powr">;
def blt_half_recip  : MathBuiltin_ff<"half_recip">;
def blt_half_rsqrt  : MathBuiltin_ff<"half_rsqrt">;
def blt_half_sin    : MathBuiltin_ff<"half_sin">;
def blt_half_sqrt   : MathBuiltin_ff<"half_sqrt">;
def blt_half_tan    : MathBuiltin_ff<"half_tan">;

// native_* versions - to be overridden by target-dependent implementations
def blt_native_cos      : MathBuiltin_ff<"native_cos">;
def blt_native_divide   : MathBuiltin_fff<"native_divide">;
def blt_native_exp      : MathBuiltin_ff<"native_exp">;
def blt_native_exp2     : MathBuiltin_ff<"native_exp2">;
def blt_native_exp10    : MathBuiltin_ff<"native_exp10">;
def blt_native_log      : MathBuiltin_ff<"native_log">;
def blt_native_log2     : MathBuiltin_ff<"native_log2">;
def blt_native_log10    : MathBuiltin_ff<"native_log10">;
def blt_native_powr     : MathBuiltin_fff<"native_powr">;
def blt_native_recip    : MathBuiltin_ff<"native_recip">;
def blt_native_rsqrt    : MathBuiltin_ff<"native_rsqrt">;
def blt_native_sin      : MathBuiltin_ff<"native_sin">;
def blt_native_sqrt     : MathBuiltin_ff<"native_sqrt">;
def blt_native_tan      : MathBuiltin_ff<"native_tan">;

//===----------------------------------------------------------------------===//
// Math builtin implementations
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_acos, RecursiveSplit<[{
  return __libm(acos)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_acosh, RecursiveSplit<[{
  return __libm(acosh)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_acospi, RecursiveSplit<[{
  return __libm(acos)(param1) / __libm_CONST_(M_PI);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_asin, RecursiveSplit<[{
  return __libm(asin)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_asinh, RecursiveSplit<[{
  return __libm(asinh)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_asinpi, RecursiveSplit<[{
  return __libm(asin)(param1) / __libm_CONST_(M_PI);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atan, RecursiveSplit<[{
  return __libm(atan)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atanh, RecursiveSplit<[{
  return __libm(atanh)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atanpi, RecursiveSplit<[{
  return __libm(atan)(param1) / __libm_CONST_(M_PI);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_cbrt, RecursiveSplit<[{
  return __libm(cbrt)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_ceil, RecursiveSplit<[{
  return __libm(ceil)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_cos, RecursiveSplit<[{
  return __libm(cos)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_cosh, RecursiveSplit<[{
  return __libm(cosh)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_cospi, RecursiveSplit<[{
  return __libm(cos)(param1 * __libm_CONST_(M_PI));
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_erfc, RecursiveSplit<[{
  return __libm(erfc)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_erf, RecursiveSplit<[{
  return __libm(erf)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_exp, RecursiveSplit<[{
  return __libm(exp)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_exp2, RecursiveSplit<[{
  return __libm(exp2)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_exp10, RecursiveSplit<[{
  return __libm(exp)(param1 * __libm(log)(__libm_const(10.0)));
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_expm1, RecursiveSplit<[{
  return __libm(expm1)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fabs, RecursiveSplit<[{
  return __libm(fabs)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_floor, RecursiveSplit<[{
  return __libm(floor)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_log, RecursiveSplit<[{
  return __libm(log)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_log2, RecursiveSplit<[{
  return __libm(log2)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_log10, RecursiveSplit<[{
  return __libm(log10)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_log1p, RecursiveSplit<[{
  return __libm(log1p)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_logb, RecursiveSplit<[{
  return __libm(logb)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_lgamma, RecursiveSplit<[{
  return __libm(lgamma)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_rint, RecursiveSplit<[{
  return __libm(rint)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_round, RecursiveSplit<[{
  return __libm(round)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_rsqrt, RecursiveSplit<[{
  return __libm_const(1.0) / __libm(sqrt)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sin, RecursiveSplit<[{
  return __libm(sin)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sinh, RecursiveSplit<[{
  return __libm(sinh)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sinpi, RecursiveSplit<[{
  return __libm(sin)(param1 * __libm_CONST_(M_PI));
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sqrt, RecursiveSplit<[{
  return __libm(sqrt)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_tan, RecursiveSplit<[{
  return __libm(tan)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_tanh, RecursiveSplit<[{
  return __libm(tanh)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_tanpi, RecursiveSplit<[{
  return __libm(tan)(param1 * __libm_CONST_(M_PI));
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_tgamma, RecursiveSplit<[{
  return __libm(tgamma)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_trunc, RecursiveSplit<[{
  return __libm(trunc)(param1);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atan2, RecursiveSplit<[{
  return __libm(atan2)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atan2pi, RecursiveSplit<[{
  return __libm(atan2)(param1, param2) / __libm_CONST_(M_PI);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_copysign, RecursiveSplit<[{
  return __libm(copysign)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fdim, RecursiveSplit<[{
  return __libm(fdim)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fmod, RecursiveSplit<[{
  return __libm(fmod)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_hypot, RecursiveSplit<[{
  return __libm(hypot)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_maxmag, RecursiveSplit<[{
  TYPE abs1 = __libm(fabs)(param1);
  TYPE abs2 = __libm(fabs)(param2);

  if (abs1 > abs2) return param1;
  else if (abs2 > abs1) return param2;
  else return __libm(fmax)(param1, param2);
}], [LibM<Id<0>>, TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_minmag, RecursiveSplit<[{
  TYPE abs1 = __libm(fabs)(param1);
  TYPE abs2 = __libm(fabs)(param2);

  if (abs1 < abs2) return param1;
  else if (abs2 < abs1) return param2;
  else return __libm(fmin)(param1, param2);
}], [LibM<Id<0>>, TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_nextafter, RecursiveSplit<[{
  return __libm(nextafter)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_pow, RecursiveSplit<[{
  return __libm(pow)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_powr, RecursiveSplit<[{
  return __libm(pow)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_remainder, RecursiveSplit<[{
  return __libm(remainder)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fmin, RecursiveSplit<[{
  return __libm(fmin)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fmax, RecursiveSplit<[{
  return __libm(fmax)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_pown, RecursiveSplit<[{
  return __libm(powi)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_rootn, RecursiveSplit<[{
  return __libm(pow)(param1, __libm_const(1.0) / param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffep-1f);
}], [TypedefId<"TYPE", Id<0>>]>, "ffP1f">;

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffep-1f);
}], [TypedefId<"TYPE", Id<0>>]>, "ffP2f">;

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffep-1f);
}], [TypedefId<"TYPE", Id<0>>]>, "ffP4f">;

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffffffffffp-1);
}], [TypedefId<"TYPE", Id<0>>]>, "ddP1d">;

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffffffffffp-1);
}], [TypedefId<"TYPE", Id<0>>]>, "ddP2d">;

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffffffffffp-1);
}], [TypedefId<"TYPE", Id<0>>]>, "ddP4d">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_frexp, DirectSplit<[{
  PTYPE exp;
  TYPE res = __libm(frexp)(param1, &exp);
  *param2 = exp;
  return res;
}], [LibM<Id<1>>, TypedefId<"TYPE", Id<1>>, TypedefId<"PTYPE", Pointee<2>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_lgamma_r, DirectSplit<[{
  if (param1 <= 0) {
    TYPE i, f, s;
    f = __builtin_ocl_modf(-param1, &i);
    if (f == __libm_const(0.0)) {
      *param2 = 1;
      return __libm_CONST(HUGE_VAL);
    }
    *param2 = (__builtin_ocl_fmod(i, __libm_const(2.0)) != __libm_const(0.0)) ? 1 : -1;
    s = __builtin_ocl_sinpi(f);
    if (s < 0) s = -s;
    return __builtin_ocl_log(__libm_CONST_(M_PI)) - __builtin_ocl_log(s) - __builtin_ocl_lgamma(1 - param1);
  }
  *param2 = 1;
  return __builtin_ocl_lgamma(param1);
}], [LibM<Id<1>>, TypedefId<"TYPE", Id<1>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fma, RecursiveSplit<[{
  return __libm(fma)(param1, param2, param3);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_mad, RecursiveSplit<[{
  return param1 * param2 + param3; 
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_remquo, DirectSplit<[{
  TYPE quo;
  TYPE rem = __libm(remquo)(param1, param2, &quo);
  *param3 = quo;
  return rem;
}], [LibM<Id<1>>, TypedefId<"TYPE", Pointee<3>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_modf, DirectSplit<[{
  TYPE intpart;
  TYPE mod = __libm(modf)(param1, &intpart);
  *param2 = intpart;
  return mod;
}], [LibM<Id<1>>, TypedefId<"TYPE", Pointee<2>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sincos, TemplateStrategy<[{
  *param2 = __builtin_ocl_cos(param1);
  return __builtin_ocl_sin(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_ldexp, RecursiveSplit<[{
  return __libm(ldexp)(param1, param2);
}], [LibM<Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_nan, RecursiveSplit<[{
  // Infinity/NaN encoding: s111 1111 1aXX XXXX XXXX XXXX XXXX XXXX
  // Infinity <==> M == 0
  // qNaN <==> a == 1
  // sNaN <==> a == 0 && M != 0
  uint val = (param1 | 0x7fc00000U);
  return as_float(val);
}]>, "fUn">;

def : OCLBuiltinImpl<blt_nan, RecursiveSplit<[{
  // Infinity/NaN encoding: s111 1111 1111 aXXX XXXX XXXX XXXX XXXX
  //                        XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX
  // Infinity <==> M == 0
  // qNaN <==> a == 1
  // sNaN <==> a == 0 && M != 0
  ulong val = (param1 | 0x7ff8000000000000U);
  return as_double(val);
}]>, "dUl">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_ilogb, RecursiveSplit<[{
  return __libm(ilogb)(param1);
}], [LibM<Id<1>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_cos, RecursiveSplit<[{
  return __builtin_ocl_cos(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_divide, RecursiveSplit<[{
  return param1 / param2;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_exp, RecursiveSplit<[{
  return __builtin_ocl_exp(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_exp2, RecursiveSplit<[{
  return __builtin_ocl_exp2(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_exp10, RecursiveSplit<[{
  return __builtin_ocl_exp10(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_log, RecursiveSplit<[{
  return __builtin_ocl_log(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_log2, RecursiveSplit<[{
  return __builtin_ocl_log2(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_log10, RecursiveSplit<[{
  return __builtin_ocl_log10(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_powr, RecursiveSplit<[{
  return __builtin_ocl_powr(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_recip, RecursiveSplit<[{
  return 1.0f / param1;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_rsqrt, RecursiveSplit<[{
  return __builtin_ocl_rsqrt(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_sin, RecursiveSplit<[{
  return __builtin_ocl_sin(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_sqrt, RecursiveSplit<[{
  return __builtin_ocl_sqrt(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_half_tan, RecursiveSplit<[{
  return __builtin_ocl_tan(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_cos, RecursiveSplit<[{
  return __builtin_ocl_cos(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_divide, RecursiveSplit<[{
  return param1 / param2;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_exp, RecursiveSplit<[{
  return __builtin_ocl_exp(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_exp2, RecursiveSplit<[{
  return __builtin_ocl_exp2(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_exp10, RecursiveSplit<[{
  return __builtin_ocl_exp10(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_log, RecursiveSplit<[{
  return __builtin_ocl_log(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_log2, RecursiveSplit<[{
  return __builtin_ocl_log2(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_log10, RecursiveSplit<[{
  return __builtin_ocl_log10(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_powr, RecursiveSplit<[{
  return __builtin_ocl_powr(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_recip, RecursiveSplit<[{
  return 1.0f / param1;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_rsqrt, RecursiveSplit<[{
  return __builtin_ocl_rsqrt(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_sin, RecursiveSplit<[{
  return __builtin_ocl_sin(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_sqrt, RecursiveSplit<[{
  return __builtin_ocl_sqrt(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_native_tan, RecursiveSplit<[{
  return __builtin_ocl_tan(param1);
}]>>;
