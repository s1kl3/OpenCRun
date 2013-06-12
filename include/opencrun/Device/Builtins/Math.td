//===----------------------------------------------------------------------===//
// Math builtin classes
//===----------------------------------------------------------------------===//

let Group = "MATH" in {

class MathBuiltin_rr<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1>>]
>;

class MathBuiltin_rrr<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_real, ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1> >, isSameAs<Id<1>, Id<2>>]
>;

class MathBuiltin_rrrr<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_real, ocl_gentype_real, ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>]
>;

class MathBuiltin_rrSn<string name> : OCLSimpleBuiltin<name,
  [ocl_gentype_real, ocl_gentype_real, ocl_s_int], 
  [isSameAs<Id<0>,Id<1>>, isSameDimAs<Id<1>,Id<2>>]
>;

class MathBuiltin_Snr<string name> : OCLSimpleBuiltin<name, 
  [ocl_s_int, ocl_gentype_real], 
  [isSameDimAs<Id<0>,Id<1>>]
>;

class MathBuiltin_rUi<string name> : OCLSimpleBuiltin<name, 
  [ocl_gentype_real, ocl_gentype_u_integer], 
  [isSameDimAs<Id<0>,Id<1>>, isSameBaseSizeAs<Id<0>,Id<1>>]
>;

class MathBuiltin_rrP1Sn_rrP2Sn_rrP4Sn<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_global, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "rrP1Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_local, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "rrP2Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_private, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameDimAs<Id<1>, Pointee<2>>], "rrP4Sn">
]>;

class MathBuiltin_rrrP1Sn_rrrP2Sn_rrrP4Sn<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_global, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>,Id<2>>, 
      isSameDimAs<Id<2>, Pointee<3>>], "rrrP1Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_local, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>,Id<2>>, 
      isSameDimAs<Id<2>, Pointee<3>>], "rrrP2Sn">,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_private, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>,Id<2>>, 
      isSameDimAs<Id<2>, Pointee<3>>], "rrrP4Sn">
]>;

class MathBuiltin_VrVrCn_rrSn<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_real, 
     ocl_vgentype_real, 
     ocl_int
  ], [isSameAs<Id<0>,Id<1>>], "VrVrCn">,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_s_int
  ], [isSameAs<Id<0>,Id<1>>, isSameDimAs<Id<1>,Id<2>>], "rrSn">
]>;

class MathBuiltin_rrP1r_rrP2r_rrP4r<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real,
    ocl_gentype_real,
    OCLPointerType<ocl_gentype_real, ocl_as_global, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>, Pointee<2>>], "rrP1r">,
  OCLBuiltinVariant<[
    ocl_gentype_real,
    ocl_gentype_real,
    OCLPointerType<ocl_gentype_real, ocl_as_local, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>, Pointee<2>>], "rrP2r">,
  OCLBuiltinVariant<[
    ocl_gentype_real,
    ocl_gentype_real,
    OCLPointerType<ocl_gentype_real, ocl_as_private, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>, Pointee<2>>], "rrP4r">
]>;

}

//===----------------------------------------------------------------------===//
// Math builtin declarations
//===----------------------------------------------------------------------===//

def blt_acos        : MathBuiltin_rr<"acos">;
def blt_acosh       : MathBuiltin_rr<"acosh">;
def blt_acospi      : MathBuiltin_rr<"acospi">;
def blt_asin        : MathBuiltin_rr<"asin">;
def blt_asinh       : MathBuiltin_rr<"asinh">;
def blt_asinpi      : MathBuiltin_rr<"asinpi">;
def blt_atan        : MathBuiltin_rr<"atan">;
def blt_atanh       : MathBuiltin_rr<"atanh">;
def blt_atanpi      : MathBuiltin_rr<"atanpi">;
def blt_cbrt        : MathBuiltin_rr<"cbrt">;
def blt_ceil        : MathBuiltin_rr<"ceil">;
def blt_cos         : MathBuiltin_rr<"cos">;
def blt_cosh        : MathBuiltin_rr<"cosh">;
def blt_cospi       : MathBuiltin_rr<"cospi">;
def blt_erfc        : MathBuiltin_rr<"ercf">;
def blt_erf         : MathBuiltin_rr<"erf">;
def blt_exp         : MathBuiltin_rr<"exp">;
def blt_exp2        : MathBuiltin_rr<"exp2">;
def blt_exp10       : MathBuiltin_rr<"exp10">;
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
def blt_rsqrt       : MathBuiltin_rr<"rsqrt">;
def blt_sin         : MathBuiltin_rr<"sin">;
def blt_sinh        : MathBuiltin_rr<"sinh">;
def blt_sinpi       : MathBuiltin_rr<"sinpi">;
def blt_sqrt        : MathBuiltin_rr<"sqrt">;
def blt_tan         : MathBuiltin_rr<"tan">;
def blt_tanh        : MathBuiltin_rr<"tanh">;
def blt_tanpi       : MathBuiltin_rr<"tanpi">;
def blt_tgamma      : MathBuiltin_rr<"tgamma">;
def blt_trunc       : MathBuiltin_rr<"trunc">;


def blt_atan2       : MathBuiltin_rrr<"atan2">;
def blt_atan2pi     : MathBuiltin_rrr<"atanpi2">;
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

def blt_pown        : MathBuiltin_rrSn<"pown">;
def blt_rootn       : MathBuiltin_rrSn<"rootn">;

def blt_frexp       : MathBuiltin_rrP1Sn_rrP2Sn_rrP4Sn<"frexp">;
def blt_lgamma_r    : MathBuiltin_rrP1Sn_rrP2Sn_rrP4Sn<"lgamma_r">;

def blt_fma         : MathBuiltin_rrrr<"fma">;
def blt_mad         : MathBuiltin_rrrr<"mad">;

def blt_remquo      : MathBuiltin_rrrP1Sn_rrrP2Sn_rrrP4Sn<"remquo">;

def blt_modf        : MathBuiltin_rrP1r_rrP2r_rrP4r<"modf">;
def blt_sincos      : MathBuiltin_rrP1r_rrP2r_rrP4r<"sincos">;

def blt_ldexp       : MathBuiltin_VrVrCn_rrSn<"ldexp">;

def blt_nan         : MathBuiltin_rUi<"nan">;

def blt_ilogb       : MathBuiltin_Snr<"ilogb">;

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
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_acosf(param1)/M_PI;
    case 64:
      return __builtin_acos(param1)/M_PI;
  }
}]>>;

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
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_asinf(param1)/M_PI;
    case 64:
      return __builtin_asin(param1)/M_PI;
  }
}]>>;

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
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_atanf(param1)/M_PI;
    case 64:
      return __builtin_atan(param1)/M_PI;
  }
}]>>;

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

def : OCLBuiltinImpl<blt_cospi, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_cosf(param1 * M_PI);
    case 64:
      return __builtin_cos(param1 * M_PI);
  }
}]>>;

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
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_expf(param1 * __builtin_logf(10.0));
    case 64:
      return __builtin_exp(param1 * __builtin_log(10.0));
  }
}]>>;

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
  switch (sizeof(param1) * 8) {
    case 32:
      return 1.0/__builtin_sqrtf(param1);
    case 64:
      return 1.0/__builtin_sqrt(param1);
  }
}]>>;

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
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_sinf(param1 * M_PI);
    case 64:
      return __builtin_sin(param1 * M_PI);
  }
}]>>;

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
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_tanf(param1 * M_PI);
    case 64:
      return __builtin_tan(param1 * M_PI);
  }
}]>>;

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
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_atan2f(param1, param2)/M_PI;
    case 64:
      return __builtin_atan2(param1, param2)/M_PI;
  }
}]>>;

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
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_powf(param1, 1.0/param2);
    case 64:
      return __builtin_pow(param1, 1.0/param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_frexp, DirectSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_frexpf(param1, param2);
    case 64:
      return __builtin_frexp(param1, param2);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_lgamma_r, DirectSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_lgammaf(param1);
    case 64:
      return __builtin_lgamma(param1);
  }
}]>>;

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
      return __builtin_remquof(param1, param2, param3);
    case 64:
      return __builtin_remquo(param1, param2, param3);
  }
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_modf, DirectSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_modff(param1, param2);
    case 64:
      return __builtin_modf(param1, param2);
  }
}]>>;

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

def : OCLBuiltinImpl<blt_ilogb, RecursiveSplit<[{
  switch (sizeof(param1) * 8) {
    case 32:
      return __builtin_ilogb(param1, param2);
    case 64:
      return __builtin_ilogb(param1, param2);
  }
}]>>;

