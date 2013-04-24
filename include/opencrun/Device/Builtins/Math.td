//===----------------------------------------------------------------------===//
// Math builtin classes
//===----------------------------------------------------------------------===//

class MathBuiltin_rr<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1> >]
>;

class MathBuiltin_rrr<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_real, ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1> >, isSameAs<Id<1>, Id<2>>]
>;

class MathBuiltin_rrrr<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_real, ocl_gentype_real, ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1> >, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>]
>;

class MathBuiltin_rrPi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_global, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameDimAs<Id<1>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_local, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameDimAs<Id<1>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_private, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameDimAs<Id<1>,Pointee<2>>]>
]>;

class MathBuiltin_rrrPi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_global, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>,Id<2>>, 
      isSameDimAs<Id<2>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_local, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>,Id<2>>, 
      isSameDimAs<Id<2>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_gentype_real, 
     OCLPointerType<ocl_s_int, ocl_as_private, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>,Id<2>>, 
      isSameDimAs<Id<2>,Pointee<3>>]>
]>;

class MathBuiltin_rri<string name> : OCLSimpleBuiltin<name,
  [ocl_gentype_real, ocl_gentype_real, ocl_s_int], 
  [isSameAs<Id<0>,Id<1>>, isSameDimAs<Id<1>,Id<2>>]
>;

class MathBuiltin_rri_rrSi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_gentype_real, 
     ocl_gentype_real, 
     ocl_s_int
  ], [isSameAs<Id<0>,Id<1>>, isSameDimAs<Id<1>,Id<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_real, 
     ocl_vgentype_real, 
     ocl_int
  ], [isSameAs<Id<0>,Id<1>>]>
]>;

class MathBuiltin_ir<string name> : OCLSimpleBuiltin<name, 
  [ocl_s_int, ocl_gentype_real], 
  [isSameDimAs<Id<0>,Id<1>>]
>;

class MathBuiltin_rrPr<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real,
    ocl_gentype_real,
    OCLPointerType<ocl_gentype_real, ocl_as_global, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>, Pointee<2>>]>,
  OCLBuiltinVariant<[
    ocl_gentype_real,
    ocl_gentype_real,
    OCLPointerType<ocl_gentype_real, ocl_as_local, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>, Pointee<2>>]>,
  OCLBuiltinVariant<[
    ocl_gentype_real,
    ocl_gentype_real,
    OCLPointerType<ocl_gentype_real, ocl_as_private, []>
  ], [isSameAs<Id<0>,Id<1>>, isSameAs<Id<1>, Pointee<2>>]>
]>;

class MathBuiltin_fi_dl<string name> : OCLSimpleBuiltin<name, 
  [ocl_gentype_real, ocl_gentype_u_integer], 
  [isSameDimAs<Id<0>,Id<1>>, isSameBaseSizeAs<Id<0>,Id<1>>]
>;

//===----------------------------------------------------------------------===//
// Math builtin declarations
//===----------------------------------------------------------------------===//

def blt_acos : MathBuiltin_rr<"acos">;
def blt_acosh : MathBuiltin_rr<"acosh">;
def blt_acospi : MathBuiltin_rr<"acospi">;
def blt_asin : MathBuiltin_rr<"asin">;
def blt_asinh : MathBuiltin_rr<"asinh">;
def blt_asinpi : MathBuiltin_rr<"asinpi">;
def blt_atan : MathBuiltin_rr<"atan">;
def blt_atan2: MathBuiltin_rrr<"atan2">;
def blt_atanh : MathBuiltin_rr<"atanh">;
def blt_atanpi : MathBuiltin_rr<"atanpi">;
def blt_atan2pi : MathBuiltin_rrr<"atanpi2">;
def blt_cbrt : MathBuiltin_rr<"cbrt">;
def blt_ceil : MathBuiltin_rr<"ceil">;
def blt_copysign : MathBuiltin_rrr<"copysign">;
def blt_cos : MathBuiltin_rr<"cos">;
def blt_cosh : MathBuiltin_rr<"cosh">;
def blt_cospi : MathBuiltin_rr<"cospi">;
def blt_ercf : MathBuiltin_rr<"ercf">;
def blt_erf : MathBuiltin_rr<"erf">;
def blt_exp : MathBuiltin_rr<"exp">;
def blt_exp2 : MathBuiltin_rr<"exp2">;
def blt_exp10 : MathBuiltin_rr<"exp10">;
def blt_expm1 : MathBuiltin_rr<"expm1">;
def blt_fabs : MathBuiltin_rr<"fabs">;
def blt_fdim : MathBuiltin_rrr<"fdim">;
def blt_floor : MathBuiltin_rr<"floor">;
def blt_fma : MathBuiltin_rrrr<"fma">;
def blt_fmod : MathBuiltin_rrr<"fmod">;
def blt_frexp : MathBuiltin_rrPi<"frexp">;
def blt_hypot : MathBuiltin_rrr<"hypot">;
def blt_ldexp : MathBuiltin_rri_rrSi<"ldexp">;
def blt_lgamma : MathBuiltin_rr<"lgamma">;
def blt_lgamma_r : MathBuiltin_rrPi<"lgamma_r">;
def blt_log : MathBuiltin_rr<"log">;
def blt_log2 : MathBuiltin_rr<"log2">;
def blt_log10 : MathBuiltin_rr<"log10">;
def blt_log1p : MathBuiltin_rr<"log1p">;
def blt_logb : MathBuiltin_rr<"logb">;
def blt_mad : MathBuiltin_rrrr<"mad">;
def blt_maxmag : MathBuiltin_rrr<"maxmag">;
def blt_minmag : MathBuiltin_rrr<"minmag">;
def blt_modf : MathBuiltin_rrPr<"modf">;
def blt_nan : MathBuiltin_fi_dl<"modf">;
def blt_nextafter : MathBuiltin_rrr<"nextafter">;
def blt_pow : MathBuiltin_rrr<"pow">;
def blt_pown : MathBuiltin_rri<"pown">;
def blt_powr : MathBuiltin_rrr<"powr">;
def blt_remainder : MathBuiltin_rrr<"remainder">;
def blt_remquo : MathBuiltin_rrrPi<"remquo">;
def blt_rint : MathBuiltin_rr<"rint">;
def blt_rootn : MathBuiltin_rri<"rootn">;
def blt_round : MathBuiltin_rr<"round">;
def blt_rsqrt : MathBuiltin_rr<"rsqrt">;
def blt_sin : MathBuiltin_rr<"sin">;
def blt_sincos : MathBuiltin_rrPr<"sincos">;
def blt_sinh : MathBuiltin_rr<"sinh">;
def blt_sinpi : MathBuiltin_rr<"sinpi">;
def blt_sqrt : MathBuiltin_rr<"sqrt">;
def blt_tan : MathBuiltin_rr<"tan">;
def blt_tanh : MathBuiltin_rr<"tanh">;
def blt_tanpi : MathBuiltin_rr<"tanpi">;
def blt_tgamma : MathBuiltin_rr<"tgamma">;
def blt_trunc : MathBuiltin_rr<"trunc">;
