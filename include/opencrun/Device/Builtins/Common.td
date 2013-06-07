//===----------------------------------------------------------------------===//
// Common builtin classes
//===----------------------------------------------------------------------===//

class CommonBuiltin_rr<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_real, ocl_gentype_real], 
  [isSameAs<Id<0>, Id<1>>]
>;

class CommonBuiltin_rrr_VrVrCr<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>],
  "rrr">,
  OCLBuiltinVariant<[
    ocl_vgentype_real,
    ocl_vgentype_real,
    ocl_sgentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameBaseAs<Id<1>, Id<2>>],
  "VrVrCr">
]>;

class CommonBuiltin_rrr_VrCrVr<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>],
  "rrr">,
  OCLBuiltinVariant<[
    ocl_vgentype_real,
    ocl_sgentype_real,
    ocl_vgentype_real
  ],
  [isSameAs<Id<0>, Id<2>>, isSameBaseAs<Id<1>, Id<2>>],
  "VrCrVr">
]>;

class CommonBuiltin_rrrr_VrVrVrCr<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>],
  "rrrr">,
  OCLBuiltinVariant<[
    ocl_vgentype_real,
    ocl_vgentype_real,
    ocl_vgentype_real,
    ocl_sgentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameBaseAs<Id<2>, Id<3>>],
  "VrVrVrCr">
]>;

class CommonBuiltin_rrrr_VrCrCrVr<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>],
  "rrrr">,
  OCLBuiltinVariant<[
    ocl_vgentype_real,
    ocl_sgentype_real,
    ocl_sgentype_real,
    ocl_vgentype_real
  ],
  [isSameAs<Id<0>, Id<3>>, isSameAs<Id<1>, Id<2>>, isSameBaseAs<Id<0>, Id<1>>],
  "VrCrCrVr">
]>;

class CommonBuiltin_rrrr_VrVrCrCr<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real,
    ocl_gentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>],
  "rrrr">,
  OCLBuiltinVariant<[
    ocl_vgentype_real,
    ocl_vgentype_real,
    ocl_sgentype_real,
    ocl_sgentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<2>, Id<3>>, isSameBaseAs<Id<1>, Id<2>>],
  "VrVrCrCr">
]>;

//===----------------------------------------------------------------------===//
// Common builtin declarations
//===----------------------------------------------------------------------===//

def blt_degrees     : CommonBuiltin_rr<"degrees">;
def blt_radians     : CommonBuiltin_rr<"radians">;
def blt_sign        : CommonBuiltin_rr<"sign">;

def blt_max         : CommonBuiltin_rrr_VrVrCr<"max">;
def blt_min         : CommonBuiltin_rrr_VrVrCr<"min">;

def blt_step        : CommonBuiltin_rrr_VrCrVr<"step">;

def blt_mix         : CommonBuiltin_rrrr_VrVrVrCr<"mix">;

def blt_clamp       : CommonBuiltin_rrrr_VrVrCrCr<"clamp">;

def blt_smoothstep  : CommonBuiltin_rrrr_VrCrCrVr<"smoothstep">;

//===----------------------------------------------------------------------===//
// Common builtin implementations
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_degrees, RecursiveSplit<[{
  return (180 / M_PI) * param1;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_radians, RecursiveSplit<[{
  return (M_PI / 180) * param1;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_sign, RecursiveSplit<[{
  if (param1 > 0.0f)
    return 1.0f;
  else if (param1 < 0.0f)
    return -1.0f;
  else if (param1 == -0.0f)
    return -0.0f;
  else if (__builtin_isnanf(param1))
    return 0.0f;
  else
    return 0.0f;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_max, RecursiveSplit<[{
  return param1 < param2 ? param2 : param1;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_min, RecursiveSplit<[{
  return param2 < param1 ? param2 : param1;  
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_step, RecursiveSplit<[{
  return param2 < param1 ? 0.0f : 1.0f; 
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_mix, RecursiveSplit<[{
  return param1 + (param2 - param1) * param3;
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_clamp, RecursiveSplit<[{
  return __builtin_ocl_min(__builtin_ocl_max(param1, param2), param3));
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_smoothstep, RecursiveSplit<[{
  if (param3 <= param1)
    return 0.0f;
  else if (param3 >= param1)
    return 1.0f;
  else
    return __builtin_ocl_clamp((param3 - param1)/(param2 - param1), 0.0f, 1.0f) *
    __builtin_ocl_clamp((param3 - param1)/(param2 - param1), 0.0f, 1.0f) *
    (3.0f - 2.0f * __builtin_ocl_clamp((param3 - param1)/(param2 - param1), 0.0f, 1.0f));
}]>>;

