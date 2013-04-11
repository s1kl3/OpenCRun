
class CommonBuiltin <list<OCLType> ret_types = [],
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
              ]
             ]> {
  list<OCLAttribute> Attributes = [overloadable,pure];
}

def blt_common_clamp    : CommonBuiltin<[ ocl_gentype_1
                                        , ocl_gentype_1
                                        ],
                                        [
                                         [ ocl_gentype_1
                                         , ocl_gentype_1
                                         , ocl_gentype_1 
                                         ],
                                         [ ocl_gentype_1
                                         , ocl_float
                                         , ocl_float
                                         ]
                                        ],
                                        [{return __builtin_fmin(__builtin_fmax(x0, x1), x2);}]>;

def blt_common_degrees  : CommonBuiltin<[ocl_gentype_1],
                                        [[ocl_gentype_1]],
                                        [{return (180 / M_PI) * x0;}]>;

def blt_common_max      : CommonBuiltin<[ ocl_gentype_1
                                        , ocl_gentype_1
                                        ],
                                        [
                                         [ ocl_gentype_1
                                         , ocl_gentype_1 
                                         ],
                                         [ ocl_gentype_1
                                         , ocl_float
                                         ]
                                        ],
                                        [{return x0 < x1 ? x1 : x0;}]>;

def blt_common_min      : CommonBuiltin<[ ocl_gentype_1
                                        , ocl_gentype_1
                                        ],
                                        [
                                         [ ocl_gentype_1
                                         , ocl_gentype_1 
                                         ],
                                         [ ocl_gentype_1
                                         , ocl_float
                                         ]
                                        ],
                                        [{return x1 < x0 ? x1 : x0;}]>;

def blt_common_mix      : CommonBuiltin<[ ocl_gentype_1
                                        , ocl_gentype_1
                                        ],
                                        [
                                         [ ocl_gentype_1
                                         , ocl_gentype_1
                                         , ocl_gentype_1
                                         ],
                                         [ ocl_gentype_1
                                         , ocl_gentype_1
                                         , ocl_float
                                         ]
                                        ],
                                        [{return x0 + (x1 - x0) * x2;}]>;

def blt_common_radians  : CommonBuiltin<[ocl_gentype_1],
                                        [[ocl_gentype_1]],
                                        [{return (M_PI / 180) * x0;}]>;

def blt_common_sign     : CommonBuiltin<[ocl_gentype_1],
                                        [[ocl_gentype_1]],
                                        [{
  if(x0 > 0.0f)
    return 1.0f;
  else if(x0 < 0.0f)
    return -1.0f;
  else if(x0 == -0.0f)
    return -0.0f;
  else if(x0 == 0.0f)
    return 0.0f;
  else if(__builtin_isnan(x0))
    return 0.0f;
  else
    return 0.0f;
                                        }]>;

def blt_common_smoothstep   : CommonBuiltin<[ ocl_gentype_1
                                            , ocl_gentype_1
                                            ],
                                            [
                                             [ ocl_gentype_1
                                             , ocl_gentype_1
                                             , ocl_gentype_1
                                             ],
                                             [ ocl_float
                                             , ocl_float
                                             , ocl_gentype_1
                                             ]
                                            ],
                                            [{
  if(x2 <= x0)
    return 0.0f;
  else if(x2 >= x0)
    return 1.0f;
  else
    return __builtin_ocl_clamp((x2 - x0)/(x1 - x0), 0.0f, 1.0f) *
           __builtin_ocl_clamp((x2 - x0)/(x1 - x0), 0.0f, 1.0f) *
           (3.0f - 2.0f * __builtin_ocl_clamp((x2 - x0)/(x1 - x0), 0.0f, 1.0f));
                                            }]>;

def blt_common_step     : CommonBuiltin<[ ocl_gentype_1
                                        , ocl_gentype_1
                                        ],
                                        [
                                         [ ocl_gentype_1
                                         , ocl_gentype_1
                                         ],
                                         [ ocl_float
                                         , ocl_gentype_1
                                         ]
                                        ],
                                        [{return x1 < x0 ? 0.0f : 1.0f;}]>;

