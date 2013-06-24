//===----------------------------------------------------------------------===//
// Math builtin classes
//===----------------------------------------------------------------------===//

let Group = "MATH" in {

class MathBuiltin_rr<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1>>]
>;

class MathBuiltin_ff<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_float, ocl_gentype_float], 
  [isSameAs<Id<0>, Id<1>>]
>;

class MathBuiltin_rrr<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_real, ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>]
>;

class MathBuiltin_fff<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_float, ocl_gentype_float, ocl_gentype_float], 
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>]
>;

class MathBuiltin_rrrr<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_real, ocl_gentype_real, ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>]
>;

class MathBuiltin_rrSn<string name> : OCLSimpleBuiltin<name,
  [ocl_gentype_real, ocl_gentype_real, ocl_s_int], 
  [isSameAs<Id<0>, Id<1>>, isSameDimAs<Id<1>, Id<2>>]
>;

class MathBuiltin_Snr<string name> : OCLSimpleBuiltin<name, 
  [ocl_s_int, ocl_gentype_real], 
  [isSameDimAs<Id<0>, Id<1>>]
>;

class MathBuiltin_fUn_dUl<string name> : OCLMultiBuiltin<name, [ 
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

class MathBuiltin_ffSn_ddSn<string name> : OCLMultiBuiltin<name, [
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

class MathBuiltin_ff_dd<string name> : OCLMultiBuiltin<name, [
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

class MathBuiltin_fff_ddd<string name> : OCLMultiBuiltin<name, [
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

class MathBuiltin_rrr_VfVfCf_VdVdCd<string name> : OCLMultiBuiltin<name, [
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

class MathBuiltin_rrP1Sn_rrP2Sn_rrP4Sn<string name> : OCLMultiBuiltin<name, [
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
OCLMultiBuiltin<name, [
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

class MathBuiltin_rrrP1Sn_rrrP2Sn_rrrP4Sn<string name> : OCLMultiBuiltin<name, [
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

class MathBuiltin_VrVrCn_rrSn<string name> : OCLMultiBuiltin<name, [
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

class MathBuiltin_rrP1r_rrP2r_rrP4r<string name> : OCLMultiBuiltin<name, [
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
OCLMultiBuiltin<name, [
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
def blt_erfc        : MathBuiltin_rr<"ercf">;
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

def blt_cospi       : MathBuiltin_ff_dd<"cospi">;
def blt_sinpi       : MathBuiltin_ff_dd<"sinpi">;
def blt_tanpi       : MathBuiltin_ff_dd<"tanpi">;
def blt_acospi      : MathBuiltin_ff_dd<"acospi">;
def blt_asinpi      : MathBuiltin_ff_dd<"asinpi">;
def blt_atanpi      : MathBuiltin_ff_dd<"atanpi">;
def blt_exp10       : MathBuiltin_ff_dd<"exp10">;
def blt_rsqrt       : MathBuiltin_ff_dd<"rsqrt">;

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

def blt_atan2pi     : MathBuiltin_fff_ddd<"atan2pi">;

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
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_acosf(param1);
    case 64:
      return __builtin_acos(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_acosh, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_acoshf(param1);
    case 64:
      return __builtin_acosh(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_acospi, RecursiveSplit<[{
  return __builtin_acosf(param1)/M_PIF;
}]>, "ff">;

def : OCLBuiltinImpl<blt_acospi, RecursiveSplit<[{
  return __builtin_acos(param1)/M_PI;
}]>, "dd">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_asin, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_asinf(param1);
    case 64:
      return __builtin_asin(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_asinh, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_asinhf(param1);
    case 64:
      return __builtin_asinh(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_asinpi, RecursiveSplit<[{
  return __builtin_asinf(param1)/M_PIF;
}]>, "ff">;

def : OCLBuiltinImpl<blt_asinpi, RecursiveSplit<[{
  return __builtin_asin(param1)/M_PI;
}]>, "dd">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atan, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_atanf(param1);
    case 64:
      return __builtin_atan(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atanh, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_atanhf(param1);
    case 64:
      return __builtin_atanh(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atanpi, RecursiveSplit<[{
  return __builtin_atanf(param1)/M_PIF;
}]>, "ff">;

def : OCLBuiltinImpl<blt_atanpi, RecursiveSplit<[{
  return __builtin_atan(param1)/M_PI;
}]>, "dd">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_cbrt, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_cbrtf(param1);
    case 64:
      return __builtin_cbrt(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_ceil, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_ceilf(param1);
    case 64:
      return __builtin_ceil(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_cos, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_cosf(param1);
    case 64:
      return __builtin_cos(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_cosh, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_coshf(param1);
    case 64:
      return __builtin_cosh(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_cospi, RecursiveSplit<[{
  return __builtin_cosf(param1 * M_PIF);
}]>, "ff">;

def : OCLBuiltinImpl<blt_cospi, RecursiveSplit<[{
  return __builtin_cos(param1 * M_PI);
}]>, "dd">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_erfc, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_erfcf(param1);
    case 64:
      return __builtin_erfc(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_erf, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_erff(param1);
    case 64:
      return __builtin_erf(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_exp, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_expf(param1);
    case 64:
      return __builtin_exp(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_exp2, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_exp2f(param1);
    case 64:
      return __builtin_exp2(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_exp10, RecursiveSplit<[{
  return __builtin_expf(param1 * __builtin_logf(10.0f));
}]>, "ff">;

def : OCLBuiltinImpl<blt_exp10, RecursiveSplit<[{
  return __builtin_exp(param1 * __builtin_log(10.0));
}]>, "dd">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_expm1, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_expm1f(param1);
    case 64:
      return __builtin_expm1(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fabs, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_fabsf(param1);
    case 64:
      return __builtin_fabs(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_floor, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_floorf(param1);
    case 64:
      return __builtin_floor(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_log, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_logf(param1);
    case 64:
      return __builtin_log(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_log2, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_log2f(param1);
    case 64:
      return __builtin_log2(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_log10, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_log10f(param1);
    case 64:
      return __builtin_log10(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_log1p, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_log1pf(param1);
    case 64:
      return __builtin_log1p(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_logb, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_logbf(param1);
    case 64:
      return __builtin_logb(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_lgamma, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_lgammaf(param1);
    case 64:
      return __builtin_lgamma(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_rint, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_rintf(param1);
    case 64:
      return __builtin_rint(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_round, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_roundf(param1);
    case 64:
      return __builtin_round(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_rsqrt, RecursiveSplit<[{
  return 1.0f/__builtin_sqrtf(param1);
}]>, "ff">;

def : OCLBuiltinImpl<blt_rsqrt, RecursiveSplit<[{
  return 1.0/__builtin_sqrt(param1);
}]>, "dd">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sin, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_sinf(param1);
    case 64:
      return __builtin_sin(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sinh, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_sinhf(param1);
    case 64:
      return __builtin_sinh(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sinpi, RecursiveSplit<[{
  return __builtin_sinf(param1 * M_PIF);
}]>, "ff">;

def : OCLBuiltinImpl<blt_sinpi, RecursiveSplit<[{
  return __builtin_sin(param1 * M_PI);
}]>, "dd">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sqrt, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_sqrtf(param1);
    case 64:
      return __builtin_sqrt(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_tan, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_tanf(param1);
    case 64:
      return __builtin_tan(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_tanh, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_tanhf(param1);
    case 64:
      return __builtin_tanh(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_tanpi, RecursiveSplit<[{
  return __builtin_tanf(param1 * M_PIF);
}]>, "ff">;

def : OCLBuiltinImpl<blt_tanpi, RecursiveSplit<[{
  return __builtin_tan(param1 * M_PI);
}]>, "dd">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_tgamma, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_tgammaf(param1);
    case 64:
      return __builtin_tgamma(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_trunc, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_truncf(param1);
    case 64:
      return __builtin_trunc(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atan2, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_atan2f(param1, param2);
    case 64:
      return __builtin_atan2(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atan2pi, RecursiveSplit<[{
  return __builtin_atan2f(param1, param2)/M_PIF;
}]>, "fff">;

def : OCLBuiltinImpl<blt_atan2pi, RecursiveSplit<[{
  return __builtin_atan2(param1, param2)/M_PI;
}]>, "ddd">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_copysign, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_copysignf(param1, param2);
    case 64:
      return __builtin_copysign(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fdim, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_fdimf(param1, param2);
    case 64:
      return __builtin_fdim(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fmod, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_fmodf(param1, param2);
    case 64:
      return __builtin_fmod(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_hypot, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_hypotf(param1, param2);
    case 64:
      return __builtin_hypot(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_maxmag, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_fmaxf(__builtin_fabsf(param1), __builtin_fabsf(param2));
    case 64:
      return __builtin_fmax(__builtin_fabs(param1), __builtin_fabs(param2));
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_minmag, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_fminf(__builtin_fabsf(param1), __builtin_fabsf(param2));
    case 64:
      return __builtin_fmin(__builtin_fabs(param1), __builtin_fabs(param2));
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_nextafter, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_nextafterf(param1, param2);
    case 64:
      return __builtin_nextafter(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_pow, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_powf(param1, param2);
    case 64:
      return __builtin_pow(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_powr, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_powf(param1, param2);
    case 64:
      return __builtin_pow(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_remainder, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_remainderf(param1, param2);
    case 64:
      return __builtin_remainder(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fmin, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_fminf(param1, param2);
    case 64:
      return __builtin_fmin(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fmax, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_fmaxf(param1, param2);
    case 64:
      return __builtin_fmax(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_pown, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_powif(param1, param2);
    case 64:
      return __builtin_powi(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_rootn, RecursiveSplit<[{
  return __builtin_powf(param1, 1.0f/param2);
}]>, "ffSn">;

def : OCLBuiltinImpl<blt_rootn, RecursiveSplit<[{
  return __builtin_pow(param1, 1.0/param2);
}]>, "ddSn">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffep-1f);
}], [TypedefId<"TYPE", Pointee<2>>]>, "ffP1f">;

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffep-1f);
}], [TypedefId<"TYPE", Pointee<2>>]>, "ffP2f">;

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffep-1f);
}], [TypedefId<"TYPE", Pointee<2>>]>, "ffP4f">;

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffffffffffp-1);
}], [TypedefId<"TYPE", Pointee<2>>]>, "ddP1d">;

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffffffffffp-1);
}], [TypedefId<"TYPE", Pointee<2>>]>, "ddP2d">;

def : OCLBuiltinImpl<blt_fract, DirectSplit<[{
  TYPE flr = __builtin_ocl_floor(param1);
  *param2 = flr;
  return __builtin_ocl_fmin(param1 - flr,  0x1.fffffffffffffp-1);
}], [TypedefId<"TYPE", Pointee<2>>]>, "ddP4d">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_frexp, DirectSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_frexpf(param1, (TYPE *) param2);
    case 64:
      return __builtin_frexp(param1, (TYPE *) param2);
  }
}], [TypedefId<"TYPE", Pointee<2>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_lgamma_r, DirectSplit<[{
  if (param1 <= 0) {
    float i, f, s;
    f = __builtin_modff(-param1, &i);
    if (f == 0.0f) {
      *param2 = 1;
      return HUGE_VALF;
    }
    *param2 = (__builtin_fmodf(i, 2.0f) != 0.0f) ? 1 : -1;
    s = __builtin_sinf(M_PIF * f);
    if (s < 0) s = -s;
      return LOG_PIF - __builtin_logf(s) - __builtin_lgammaf(1 - param1);
  }
  *param2 = 1;
  return __builtin_lgammaf(param1);
}]>, "ffP1Sn">;

def : OCLBuiltinImpl<blt_lgamma_r, DirectSplit<[{
  if (param1 <= 0) {
    float i, f, s;
    f = __builtin_modff(-param1, &i);
    if (f == 0.0f) {
      *param2 = 1;
      return HUGE_VALF;
    }
    *param2 = (__builtin_fmodf(i, 2.0f) != 0.0f) ? 1 : -1;
    s = __builtin_sinf(M_PIF * f);
    if (s < 0) s = -s;
      return LOG_PIF - __builtin_logf(s) - __builtin_lgammaf(1 - param1);
  }
  *param2 = 1;
  return __builtin_lgammaf(param1);
}]>, "ffP2Sn">;

def : OCLBuiltinImpl<blt_lgamma_r, DirectSplit<[{
  if (param1 <= 0) {
    float i, f, s;
    f = __builtin_modff(-param1, &i);
    if (f == 0.0f) {
      *param2 = 1;
      return HUGE_VALF;
    }
    *param2 = (__builtin_fmodf(i, 2.0f) != 0.0f) ? 1 : -1;
    s = __builtin_sinf(M_PIF * f);
    if (s < 0) s = -s;
      return LOG_PIF - __builtin_logf(s) - __builtin_lgammaf(1 - param1);
  }
  *param2 = 1;
  return __builtin_lgammaf(param1);
}]>, "ffP4Sn">;

def : OCLBuiltinImpl<blt_lgamma_r, DirectSplit<[{
  if (param1 <= 0) {
    double i, f, s;
    f = __builtin_modf(-param1, &i);
    if (f == 0.0) {
      *param2 = 1;
      return HUGE_VAL;
    }
    *param2 = (__builtin_fmod(i, 2.0) != 0.0) ? 1 : -1;
    s = __builtin_sin(M_PI * f);
    if (s < 0) s = -s;
      return LOG_PI - __builtin_log(s) - __builtin_lgamma(1 - param1);
  }
  *param2 = 1;
  return __builtin_lgamma(param1);
}]>, "ddP1Sn">;

def : OCLBuiltinImpl<blt_lgamma_r, DirectSplit<[{
  if (param1 <= 0) {
    double i, f, s;
    f = __builtin_modf(-param1, &i);
    if (f == 0.0) {
      *param2 = 1;
      return HUGE_VAL;
    }
    *param2 = (__builtin_fmod(i, 2.0) != 0.0) ? 1 : -1;
    s = __builtin_sin(M_PI * f);
    if (s < 0) s = -s;
      return LOG_PI - __builtin_log(s) - __builtin_lgamma(1 - param1);
  }
  *param2 = 1;
  return __builtin_lgamma(param1);
}]>, "ddP2Sn">;

def : OCLBuiltinImpl<blt_lgamma_r, DirectSplit<[{
  if (param1 <= 0) {
    double i, f, s;
    f = __builtin_modf(-param1, &i);
    if (f == 0.0) {
      *param2 = 1;
      return HUGE_VAL;
    }
    *param2 = (__builtin_fmod(i, 2.0) != 0.0) ? 1 : -1;
    s = __builtin_sin(M_PI * f);
    if (s < 0) s = -s;
      return LOG_PI - __builtin_log(s) - __builtin_lgamma(1 - param1);
  }
  *param2 = 1;
  return __builtin_lgamma(param1);
}]>, "ddP4Sn">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fma, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_fmaf(param1, param2, param3);
    case 64:
      return __builtin_fma(param1, param2, param3);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_mad, RecursiveSplit<[{
  return param1 * param2 + param3; 
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_remquo, DirectSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_remquof(param1, param2, (TYPE *) param3);
    case 64:
      return __builtin_remquo(param1, param2, (TYPE *) param3);
  }
}], [TypedefId<"TYPE", Pointee<3>>]>, "rrrP4Sn">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_modf, DirectSplit<[{
      return __builtin_modff(param1, (float *) param2);
}]>, "ffP1f">;

def : OCLBuiltinImpl<blt_modf, DirectSplit<[{
      return __builtin_modff(param1, (float *) param2);
}]>, "ffP2f">;

def : OCLBuiltinImpl<blt_modf, DirectSplit<[{
      return __builtin_modff(param1, (float *) param2);
}]>, "ffP4f">;

def : OCLBuiltinImpl<blt_modf, DirectSplit<[{
      return __builtin_modf(param1, (double *) param2);
}]>, "ddP1d">;

def : OCLBuiltinImpl<blt_modf, DirectSplit<[{
      return __builtin_modf(param1, (double *) param2);
}]>, "ddP2d">;

def : OCLBuiltinImpl<blt_modf, DirectSplit<[{
      return __builtin_modf(param1, (double *) param2);
}]>, "ddP4d">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sincos, DirectSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      *param2 = __builtin_cosf(param1);
      return __builtin_sinf(param1);
    case 64:
      *param2 = __builtin_cos(param1);
      return __builtin_sin(param1);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_ldexp, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_ldexpf(param1, param2);
    case 64:
      return __builtin_ldexp(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_nan, RecursiveSplit<[{
  union {
    uint i;
    float f;
  } n;

  n.i = (param1 | 0x7f800000U);
  return n.f;
}]>, "fUn">;

def : OCLBuiltinImpl<blt_nan, RecursiveSplit<[{
  union {
    ulong l;
    double d;
  } n;

  n.l = (param1 | 0x7f80000000000000U);
  return n.d;
}]>, "dUl">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_ilogb, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_ilogb(param1);
    case 64:
      return __builtin_ilogb(param1);
  }
}]>>;

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
