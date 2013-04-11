
class MathBuiltin <list<OCLType> ret_types = [],
                   list<list<OCLType>> param_types = [],
                   code base_impl> :
  GenBuiltin<ret_types,
             param_types,
             base_impl,
             [[ ocl_float
              , ocl_float_v_2
              , ocl_float_v_3
              , ocl_float_v_4
              , ocl_float_v_8
              , ocl_float_v_16
              ],
              [ ocl_float
              , ocl_float_v_2
              , ocl_float_v_3
              , ocl_float_v_4
              , ocl_float_v_8
              , ocl_float_v_16
              ],
              [ ocl_int
              , ocl_int_v_2
              , ocl_int_v_3
              , ocl_int_v_4
              , ocl_int_v_8
              , ocl_int_v_16
              ],
              [ ocl_uint
              , ocl_uint_v_2
              , ocl_uint_v_3
              , ocl_uint_v_4
              , ocl_uint_v_8
              , ocl_uint_v_16
              ]
             ]> {
  list<OCLAttribute> Attributes = [overloadable,pure];
}

def blt_math_acos        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_acos(x0);}]>;

def blt_math_acosh       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_acosh(x0);}]>;

def blt_math_acospi      : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return acos(x0) / M_PI;}]>;

def blt_math_asin        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_asin(x0);}]>;

def blt_math_asinh       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_asinh(x0);}]>;

def blt_math_asinpi      : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_asin(x0) / M_PI;}]>;

def blt_math_atan        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_atan(x0);}]>;

def blt_math_atan2       : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_atan2(x0, x1);}]>;

def blt_math_atanh       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_atanh(x0);}]>;

def blt_math_atanpi      : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_atan(x0) / M_PI;}]>;

def blt_math_atan2pi     : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_atan2(x0, x1) / M_PI;}]>;

def blt_math_cbrt        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_cbrt(x0);}]>;

def blt_math_ceil        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_ceil(x0);}]>;

def blt_math_copysign    : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_copysign(x0, x1);}]>;

def blt_math_cos         : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_cos(x0);}]>;

def blt_math_cosh        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_cosh(x0);}]>;

def blt_math_cospi       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_cos(M_PI * x0);}]>;

def blt_math_half_cos    : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_cos(x0);}]>;

// TODO: native_cos

def blt_math_half_divide : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return x0 / x1;}]>;

def blt_math_erf         : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_erf(x0);}]>;

def blt_math_erfc        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_erfc(x0);}]>;

def blt_math_exp         : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_exp(x0);}]>;

def blt_math_exp2        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_exp2(x0);}]>;

def blt_math_exp10       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_exp(x0 * 2.3025851f);}]>;

def blt_math_expm1       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_expm1(x0);}]>;

def blt_math_half_exp    : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_exp(x0);}]>;

def blt_math_half_exp2   : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_exp2(x0);}]>;

def blt_math_half_exp10  : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_exp(x0 * 2.3025851f);}]>;

// TODO: native_exp, native_exp2, native_exp10

def blt_math_fabs        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_fabs(x0);}]>;

def blt_math_fdim        : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_fdim(x0, x1);}]>;

def blt_math_floor       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_floor(x0);}]>;

def blt_math_fma         : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_fma(x0, x1, x2);}]>;

def blt_math_fmax        : MathBuiltin<[ ocl_gentype_1
                                       , ocl_gentype_2
                                       ],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ],
                                        [ ocl_gentype_2
                                        , ocl_float
                                        ]
                                       ],
                                       [{return __builtin_fmax(x0, x1);}]>;

// TODO: blt_math_fmax double and half cases.                                  

def blt_math_fmin        : MathBuiltin<[ ocl_gentype_1
                                       , ocl_gentype_2
                                       ],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ],
                                        [ ocl_gentype_2
                                        , ocl_float
                                        ]
                                       ],
                                       [{return __builtin_fmin(x0, x1);}]>;

// TODO: blt_math_fmin double and half cases.                                 

def blt_math_fmod        : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_fmod(x0, x1);}]>;

// TODO: blt_math_fract, blt_math_frexp

def blt_math_hypot       : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_hypot(x0, x1);}]>;

def blt_math_ilogb       : MathBuiltin<[ocl_gentype_3],
                                       [[ocl_gentype_2]],
                                       [{return __builtin_ilogb(x0);}]>;

// TODO: blt_math_ilogb double and half cases.                                  

def blt_math_ldexp       : MathBuiltin<[ ocl_gentype_2
                                       , ocl_gentype_2
                                       ],
                                       [
                                        [ ocl_gentype_2
                                        , ocl_gentype_3
                                        ],
                                        [ ocl_gentype_2
                                        , ocl_int
                                        ]
                                       ],
                                       [{return __builtin_ldexp(x0, x1);}]>;

// TODO: blt_math_ldexp double and half cases.

def blt_math_lgamma      : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_lgamma(x0);}]>;

// TODO: blt_math_lgamma_r

def blt_math_log         : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_log(x0);}]>;

def blt_math_log2        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_log2(x0);}]>;

def blt_math_log10       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_log10(x0);}]>;

def blt_math_log1p       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_log1p(x0);}]>;

def blt_math_logb        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_logb(x0);}]>;

def blt_math_half_log    : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_log(x0);}]>;

def blt_math_half_log2   : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_log2(x0);}]>;

def blt_math_half_log10  : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_log10(x0);}]>;

// TODO: native_log, naive_log2, native_log10

def blt_math_mad         : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return x0 * x1 + x2;}]>;

def blt_math_maxmag      : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_fmax(__builtin_fabs(x0), __builtin_fabs(x1));}]>;

def blt_math_minmag      : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_fmin(__builtin_fabs(x0), __builtin_fabs(x1));}]>;

// TODO: blt_math_modf

def blt_math_nan         : MathBuiltin<[ocl_gentype_2],
                                       [[ocl_gentype_4]],
                                       [{return x0 |= 0x7F800000U;}]>;

// TODO: blt_math_nan double and half cases.

def blt_math_nextafter   : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_nextafter(x0, x1);}]>;

def blt_math_pow         : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_pow(x0, x1);}]>;

def blt_math_pown        : MathBuiltin<[ocl_gentype_2],
                                       [
                                        [ ocl_gentype_2
                                        , ocl_gentype_3
                                        ]
                                       ],
                                       [{return __builtin_powi(x0, x1);}]>;

// TODO: blt_math_pown double and half cases.

def blt_math_powr        : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_pow(x0, x1);}]>;

// TODO: blt_math_native_powr

def blt_math_half_powr   : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_pow(x0, x1);}]>;

def blt_math_half_recip  : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return 1.0f / x0;}]>;

// TODO: blt_math_native_recip

def blt_math_remainder   : MathBuiltin<[ocl_gentype_1],
                                       [
                                        [ ocl_gentype_1
                                        , ocl_gentype_1
                                        ]
                                       ],
                                       [{return __builtin_remainder(x0, x1);}]>;

// TODO: blt_math_remquo

def blt_math_rint        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_rint(x0);}]>;

def blt_math_round       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_round(x0);}]>;

def blt_math_rootn       : MathBuiltin<[ocl_gentype_2],
                                       [
                                        [ ocl_gentype_2
                                        , ocl_gentype_3
                                        ]
                                       ],
                                       [{return __builtin_pow(x0, 1.0f / x1);}]>;

// TODO: blt_math_rootn double and half cases;                                  

def blt_math_sqrt        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_sqrt(x0);}]>;

def blt_math_half_sqrt   : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_sqrt(x0);}]>;

// TODO: blt_math_native_sqrt

def blt_math_rsqrt       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return 1.0f / __builtin_sqrt(x0);}]>;

def blt_math_half_rsqrt  : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return 1.0f / __builtin_sqrt(x0);}]>;

// TODO: blt_math_native_rsqrt

def blt_math_sin         : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_sin(x0);}]>;

// TODO: blt_math_sincos

def blt_math_sinh        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_sinh(x0);}]>;

def blt_math_sinpi       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_sin(M_PI * x0);}]>;

def blt_math_half_sin    : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_sin(x0);}]>;

// TODO: blt_math_native_sin

def blt_math_tan         : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_tan(x0);}]>;

def blt_math_tanh        : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_tanh(x0);}]>;

def blt_math_tanpi       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return  __builtin_tan(M_PI * x0);}]>;

def blt_math_half_tan    : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_tan(x0);}]>;

// TODO: blt_math_native_tan                                  

def blt_math_tgamma      : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_tgamma(x0);}]>;

def blt_math_trunc       : MathBuiltin<[ocl_gentype_1],
                                       [[ocl_gentype_1]],
                                       [{return __builtin_trunc(x0);}]>;
