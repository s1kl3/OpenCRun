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
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>]>,
  OCLBuiltinVariant<[
    ocl_vgentype_real,
    ocl_vgentype_real,
    ocl_sgentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameBaseAs<Id<1>, Id<2>>]>
]>;

class CommonBuiltin_rrr_VrCrVr<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>]>,
  OCLBuiltinVariant<[
    ocl_vgentype_real,
    ocl_sgentype_real,
    ocl_vgentype_real
  ],
  [isSameAs<Id<0>, Id<2>>, isSameBaseAs<Id<1>, Id<2>>]>
]>;

class CommonBuiltin_rrrr_VrVrVrCr<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>]>,
  OCLBuiltinVariant<[
    ocl_vgentype_real,
    ocl_vgentype_real,
    ocl_vgentype_real,
    ocl_sgentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameBaseAs<Id<2>, Id<3>>]>
]>;

class CommonBuiltin_rrrr_VrCrCrVr<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>]>,
  OCLBuiltinVariant<[
    ocl_vgentype_real,
    ocl_sgentype_real,
    ocl_sgentype_real,
    ocl_vgentype_real
  ],
  [isSameAs<Id<0>, Id<3>>, isSameAs<Id<1>, Id<2>>, isSameBaseAs<Id<0>, Id<1>>]>
]>;

class CommonBuiltin_rrrr_VrVrCrCr<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_real, 
    ocl_gentype_real, 
    ocl_gentype_real,
    ocl_gentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>, isSameAs<Id<2>, Id<3>>]>,
  OCLBuiltinVariant<[
    ocl_vgentype_real,
    ocl_vgentype_real,
    ocl_sgentype_real,
    ocl_sgentype_real
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<2>, Id<3>>, isSameBaseAs<Id<1>, Id<2>>]>
]>;

//===----------------------------------------------------------------------===//
// Common builtin declarations
//===----------------------------------------------------------------------===//

def blt_degrees     : CommonBuiltin_rr<"degrees">;
def blt_radians     : CommonBuiltin_rr<"radians">;
def blt_sign        : CommonBuiltin_rr<"sign">;

def blt_max         : CommonBuiltin_rrr_VrVrCr<"max">;
def blt_min         : CommonBuiltin_rrr_VrVrCr<"min">;

def blt_mix         : CommonBuiltin_rrrr_VrVrVrCr<"mix">;

def blt_clamp       : CommonBuiltin_rrrr_VrVrCrCr<"clamp">;

def blt_smoothstep  : CommonBuiltin_rrrr_VrCrCrVr<"smoothstep">;
