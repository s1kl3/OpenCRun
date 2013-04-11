
class Relational_1_Builtin <list<OCLType> ret_types = [],
                            list<list<OCLType>> param_types = [],
                            code base_impl> :
  GenBuiltin<ret_types,
             param_types,
             base_impl,
             [[ ocl_int
              , ocl_int_v_2
              , ocl_int_v_3
              , ocl_int_v_4
              , ocl_int_v_8
              , ocl_int_v_16
              ],
              [ ocl_float
              , ocl_float_v_2
              , ocl_float_v_3
              , ocl_float_v_4
              , ocl_float_v_8
              , ocl_float_v_16
              ]
             ]> {
  list<OCLAttribute> Attributes = [overloadable,pure];
}

class Relational_2_Builtin <list<OCLType> ret_types = [],
                            list<list<OCLType>> param_types = [],
                            code base_impl> :
  GenBuiltin<ret_types,
             param_types,
             base_impl,
             [
              [ ocl_char, ocl_char_v_2, ocl_char_v_3, ocl_char_v_4, ocl_char_v_8, ocl_char_v_16
              , ocl_short, ocl_short_v_2, ocl_short_v_3, ocl_short_v_4, ocl_short_v_8, ocl_short_v_16
              , ocl_int, ocl_int_v_2, ocl_int_v_3, ocl_int_v_4, ocl_int_v_8, ocl_int_v_16
              , ocl_long, ocl_long_v_2, ocl_long_v_3, ocl_long_v_4, ocl_long_v_8, ocl_long_v_16
              ]
             ]> {
  list<OCLAttribute> Attributes = [overloadable,pure];
}

def blt_relational_isfinite        : Relational_1_Builtin<[ocl_gentype_1],
                                                          [[ocl_gentype_2]],
                                                          [{return __builtin_isfinite(x0);}]>;

// TODO: blt_relational_isfinite double and half cases.                                              

def blt_relational_inf             : Relational_1_Builtin<[ocl_gentype_1],
                                                          [[ocl_gentype_2]],
                                                          [{return __builtin_isinf(x0);}]>;

// TODO: blt_relational_inf double and half cases.                                              

def blt_relational_isnan           : Relational_1_Builtin<[ocl_gentype_1],
                                                          [[ocl_gentype_2]],
                                                          [{return __builtin_isnan(x0);}]>;

// TODO: blt_relational_isnan double and half cases.                                              

def blt_relational_isnormal        : Relational_1_Builtin<[ocl_gentype_1],
                                                          [[ocl_gentype_2]],
                                                          [{return __builtin_isnormal(x0);}]>;

// TODO: blt_relational_isnormal double and half cases.

def blt_relational_isunordered     : Relational_1_Builtin<[ocl_gentype_1],
                                                          [
                                                           [ ocl_gentype_2
                                                           , ocl_gentype_2
                                                           ]
                                                          ],
                                                          [{return __builtin_isunordered(x0, x1);}]>;

// TODO: blt_relational_isunordered double and half cases.

def blt_relational_signbit         : Relational_1_Builtin<[ocl_gentype_1],
                                                          [[ocl_gentype_2]],
                                                          [{return __builtin_signbit(x0);}]>;

// TODO: blt_relational_signbit double and half cases.

def blt_relational_isequal         : Relational_1_Builtin<[ocl_gentype_1],
                                                          [
                                                           [ ocl_gentype_2
                                                           , ocl_gentype_2
                                                           ]
                                                          ],
                                                          [{
  if(__builtin_ocl_isnan(x0) || __builtin_ocl_isnan(x1))
    return 0;

  return x0 == x1;
                                                          }]>;

// TODO: blt_relational_isequal double and half cases.

def blt_relational_isnotequal      : Relational_1_Builtin<[ocl_gentype_1],
                                                          [
                                                           [ ocl_gentype_2
                                                           , ocl_gentype_2
                                                           ]
                                                          ],
                                                          [{
  if(__builtin_ocl_isnan(x0) || __builtin_ocl_isnan(x1))
    return 1;

  return x0 != x1;
                                                          }]>;

// TODO: blt_relational_isnotequal double and half cases.

def blt_relational_isgreater       : Relational_1_Builtin<[ocl_gentype_1],
                                                          [
                                                           [ ocl_gentype_2
                                                           , ocl_gentype_2
                                                           ]
                                                          ],
                                                          [{
  if(__builtin_ocl_isnan(x0) || __builtin_ocl_isnan(x1))
    return 0;

  return x0 > x1;
                                                          }]>;

// TODO: blt_relational_isgreater double and half cases.

def blt_relational_isgreaterequal  : Relational_1_Builtin<[ocl_gentype_1],
                                                          [
                                                           [ ocl_gentype_2
                                                           , ocl_gentype_2
                                                           ]
                                                          ],
                                                          [{
  if(__builtin_ocl_isnan(x0) || __builtin_ocl_isnan(x1))
    return 0;

  return x0 >= x1;
                                                          }]>;

// TODO: blt_relational_isgreaterequal double and half cases.

def blt_relational_isless          : Relational_1_Builtin<[ocl_gentype_1],
                                                          [
                                                           [ ocl_gentype_2
                                                           , ocl_gentype_2
                                                           ]
                                                           ],
                                                          [{
  if(__builtin_ocl_isnan(x0) || __builtin_ocl_isnan(x1))
    return 0;

  return x0 < x1;
                                                          }]>;

// TODO: blt_relational_isless double and half cases.

def blt_relational_islessequal     : Relational_1_Builtin<[ocl_gentype_1],
                                                          [
                                                           [ ocl_gentype_2
                                                           , ocl_gentype_2
                                                           ]
                                                          ],
                                                          [{
  if(__builtin_ocl_isnan(x0) || __builtin_ocl_isnan(x1))
    return 0;

  return x0 <= x1;
                                                          }]>;

// TODO: blt_relational_islessequal double and half cases.

def blt_relational_islessgreater   : Relational_1_Builtin<[ocl_gentype_1],
                                                          [
                                                           [ ocl_gentype_2
                                                           , ocl_gentype_2
                                                           ]
                                                          ],
                                                          [{
  if(__builtin_ocl_isnan(x0) || __builtin_ocl_isnan(x1))
    return 0;

  return (x0 < x1) || (x0 > x1);
                                                          }]>;

// TODO: blt_relational_islessgreater double and half cases.

def blt_relational_isordered       : Relational_1_Builtin<[ocl_gentype_1],
                                                          [
                                                           [ ocl_gentype_2
                                                           , ocl_gentype_2
                                                           ]
                                                          ],
                                                          [{
  return __builtin_ocl_isequal(x0, x0) && __builtin_ocl_isequal(x1, x1);
                                                          }]>;

// TODO: blt_relational_isordered double and half cases.


def blt_relational_any             : Relational_2_Builtin<[ocl_int],
                                                          [[ocl_gentype_1]],
                                                          [{return ((x0 >> (sizeof(x0) * 8 - 1)) & 0x1);}]>;

def blt_relational_all             : Relational_2_Builtin<[ocl_int],
                                                          [[ocl_gentype_1]],
                                                          [{return ((x0 >> (sizeof(x0) * 8 - 1)) & 0x1);}]>;
