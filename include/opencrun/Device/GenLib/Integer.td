
class IntegerBuiltin <list<OCLType> ret_types = [],
                      list<list<OCLType>> param_types = [],
                      code base_impl> :
  GenBuiltin<ret_types,
             param_types,
             base_impl,
             [[ ocl_char, ocl_char_v_2, ocl_char_v_3, ocl_char_v_4, ocl_char_v_8, ocl_char_v_16 
              , ocl_uchar, ocl_uchar_v_2, ocl_uchar_v_3, ocl_uchar_v_4, ocl_uchar_v_8, ocl_uchar_v_16
              , ocl_short, ocl_short_v_2, ocl_short_v_3, ocl_short_v_4, ocl_short_v_8, ocl_short_v_16
              , ocl_ushort, ocl_ushort_v_2, ocl_ushort_v_3, ocl_ushort_v_4, ocl_ushort_v_8, ocl_ushort_v_16
              , ocl_int, ocl_int_v_2, ocl_int_v_3, ocl_int_v_4, ocl_int_v_8, ocl_int_v_16
              , ocl_uint, ocl_uint_v_2, ocl_uint_v_3, ocl_uint_v_4, ocl_uint_v_8, ocl_uint_v_16
              , ocl_long, ocl_long_v_2, ocl_long_v_3, ocl_long_v_4, ocl_long_v_8, ocl_long_v_16
              , ocl_ulong, ocl_ulong_v_2, ocl_ulong_v_3, ocl_ulong_v_4, ocl_ulong_v_8, ocl_ulong_v_16
              ],
              [ ocl_uchar, ocl_uchar_v_2, ocl_uchar_v_3, ocl_uchar_v_4, ocl_uchar_v_8, ocl_uchar_v_16
              , ocl_uchar, ocl_uchar_v_2, ocl_uchar_v_3, ocl_uchar_v_4, ocl_uchar_v_8, ocl_uchar_v_16
              , ocl_ushort, ocl_ushort_v_2, ocl_ushort_v_3, ocl_ushort_v_4, ocl_ushort_v_8, ocl_ushort_v_16  
              , ocl_ushort, ocl_ushort_v_2, ocl_ushort_v_3, ocl_ushort_v_4, ocl_ushort_v_8, ocl_ushort_v_16  
              , ocl_uint, ocl_uint_v_2, ocl_uint_v_3, ocl_uint_v_4, ocl_uint_v_8, ocl_uint_v_16
              , ocl_uint, ocl_uint_v_2, ocl_uint_v_3, ocl_uint_v_4, ocl_uint_v_8, ocl_uint_v_16
              , ocl_ulong, ocl_ulong_v_2, ocl_ulong_v_3, ocl_ulong_v_4, ocl_ulong_v_8, ocl_ulong_v_16
              , ocl_ulong, ocl_ulong_v_2, ocl_ulong_v_3, ocl_ulong_v_4, ocl_ulong_v_8, ocl_ulong_v_16
              ],
              [ ocl_char, ocl_char, ocl_char, ocl_char, ocl_char, ocl_char
              , ocl_uchar, ocl_uchar, ocl_uchar, ocl_uchar, ocl_uchar, ocl_uchar
              , ocl_short, ocl_short, ocl_short, ocl_short, ocl_short, ocl_short
              , ocl_ushort, ocl_ushort, ocl_ushort, ocl_ushort, ocl_ushort, ocl_ushort
              , ocl_int, ocl_int, ocl_int, ocl_int, ocl_int, ocl_int
              , ocl_uint, ocl_uint, ocl_uint, ocl_uint, ocl_uint, ocl_uint
              , ocl_long, ocl_long, ocl_long, ocl_long, ocl_long, ocl_long
              , ocl_ulong, ocl_ulong, ocl_ulong, ocl_ulong, ocl_ulong, ocl_ulong
              ],
              [ ocl_int, ocl_int_v_2, ocl_int_v_3, ocl_int_v_4, ocl_int_v_8, ocl_int_v_16
              , ocl_uint, ocl_uint_v_2, ocl_uint_v_3, ocl_uint_v_4, ocl_uint_v_8, ocl_uint_v_16
              ]
             ]> {
  list<OCLAttribute> Attributes = [overloadable,pure];
}

def blt_integer_abs         : IntegerBuiltin<[ocl_gentype_2],
                                             [[ocl_gentype_1]],
                                             [{return x0 >= 0 ? x0 : -x0;}]>;

def blt_integer_abs_diff    : IntegerBuiltin<[ocl_gentype_2],
                                             [
                                              [ ocl_gentype_1
                                              , ocl_gentype_1
                                              ]
                                             ],
                                             [{return x0 >= x1 ? x0 - x1 : x1 - x0;}]>;

// TODO: blt_integer_add_sat    

def blt_integer_clamp       : IntegerBuiltin<[ ocl_gentype_1
                                             , ocl_gentype_1
                                             ],
                                             [
                                              [ ocl_gentype_1
                                              , ocl_gentype_1
                                              , ocl_gentype_1
                                              ],
                                              [ ocl_gentype_1
                                              , ocl_gentype_3
                                              , ocl_gentype_3
                                              ]
                                             ],
                                            [{return __builtin_fmin(__builtin_fmax(x0, x1), x2);}]>;

def blt_integer_clz         : IntegerBuiltin<[ocl_gentype_1],
                                             [[ocl_gentype_1]],
                                             [{
  switch(sizeof(x0) * 8) {
    case 8: {
      unsigned int n = 0;
      if(x0 == 0) return 8;
      if((x0 & 0xF0) == 0) {
        n += 4;
        x0 <<= 4;
      }
      if((x0 & 0xC0) == 0) {
        n += 2;
        x0 <<= 2;
      }
      if((x0 & 0x80) == 0) n += 1;

      return n;
    }
    case 16: {
      unsigned int n = 0;
      if(x0 == 0) return 16;
      if((x0 & 0xFF00) == 0) {
        n += 8;
        x0 <<= 8;
      }
      if((x0 & 0xF000) == 0) {
        n += 4;
        x0 <<= 4;
      }
      if((x0 & 0xC000) == 0) {
        n += 2;
        x0 <<= 2;
      }
      if((x0 & 0x8000) == 0) n += 1;

      return n;
    }        
    case 32:
      return __builtin_clz(x0);
    case 64:
      return __builtin_clzl(x0);
  }
                                             }]>;

def blt_integer_hadd        : IntegerBuiltin<[ocl_gentype_1],
                                             [
                                              [ ocl_gentype_1
                                              , ocl_gentype_1
                                              ]
                                             ],
                                            [{return ((long)x0 + (long)x1) >> 1;}]>;

def blt_integer_rhadd       : IntegerBuiltin<[ocl_gentype_1],
                                             [
                                              [ ocl_gentype_1
                                              , ocl_gentype_1
                                              ]
                                             ],
                                             [{return ((long)x0 + (long)x1 + 1);}]>;

// TODO: blt_integer_mad24, blt_integer_mad_hi, blt_integer_mad_sat

def blt_integer_max         : IntegerBuiltin<[ ocl_gentype_1
                                             , ocl_gentype_1
                                             ],
                                             [
                                              [ ocl_gentype_1
                                              , ocl_gentype_1 
                                              ],
                                              [ ocl_gentype_1
                                              , ocl_gentype_3
                                              ]
                                             ],
                                            [{return x0 < x1 ? x1 : x0;}]>; 

def blt_integer_min         : IntegerBuiltin<[ ocl_gentype_1
                                             , ocl_gentype_1
                                             ],
                                             [
                                              [ ocl_gentype_1
                                              , ocl_gentype_1
                                              ],
                                              [ ocl_gentype_1
                                              , ocl_gentype_3
                                              ]
                                             ],
                                            [{return x1 < x0 ? x1 : x0;}]>;

def blt_integer_mul24       : IntegerBuiltin<[ocl_gentype_4],
                                             [
                                              [ ocl_gentype_4
                                              , ocl_gentype_4
                                              ]
                                             ],
                                             [{return x0 * x1;}]>;

def blt_integer_rotate      : IntegerBuiltin<[ocl_gentype_1],
                                             [
                                              [ ocl_gentype_1
                                              , ocl_gentype_1
                                              ]
                                             ],
                                             [{return (x0 << x1) | (x0 >> ((sizeof(x0) * 8) - x1));}]>;

def blt_integer_popcount    : IntegerBuiltin<[ocl_gentype_1],
                                             [[ocl_gentype_1]],
                                             [{return __builtin_popcount(x0);}]>;

// TODO: blt_integer_mul_hi, blt_integer_sub_sat, blt_integer_upsample

