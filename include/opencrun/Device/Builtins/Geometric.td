//===----------------------------------------------------------------------===//
// Geometric builtin classes
//===----------------------------------------------------------------------===//

class GeometricBuiltin_ff<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_gentype_float_geom, ocl_gentype_float_geom], 
  [isSameAs<Id<0>, Id<1>>]
>;

class GeometricBuiltin_Cff<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_float, ocl_gentype_float_geom], 
  []
>;

class GeometricBuiltin_Cfff<string name> : OCLSimpleBuiltin<
  name, 
  [ocl_float, ocl_gentype_float_geom, ocl_gentype_float_geom], 
  [isSameAs<Id<0>, Id<1>>]
>;

class GeometricBuiltin_ff_dd<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_gentype_float_geom,
    ocl_gentype_float_geom
  ],
  [isSameAs<Id<0>, Id<1>>],
  "ff">,
  OCLBuiltinVariant<[
   ocl_gentype_double_geom,
   ocl_gentype_double_geom
  ],
  [isSameAs<Id<0>, Id<1>>],
  "dd">
]>;

class GeometricBuiltin_Cff_Cdd<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_float,
    ocl_gentype_float_geom
  ],
  [],
  "Cff">,
  OCLBuiltinVariant<[
   ocl_double,
   ocl_gentype_double_geom
  ],
  [],
  "Cdd">
]>;

class GeometricBuiltin_Cfff_Cddd<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_float,
    ocl_gentype_float_geom,
    ocl_gentype_float_geom
  ],
  [isSameAs<Id<1>, Id<2>>],
  "Cfff">,
  OCLBuiltinVariant<[
   ocl_double,
   ocl_gentype_double_geom,
   ocl_gentype_double_geom
  ],
  [isSameAs<Id<1>, Id<2>>],
  "Cddd">
]>;

class GeometricBuiltin_VfVfVf_VdVdVd<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_vec_float_geom,
    ocl_vec_float_geom,
    ocl_vec_float_geom
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>],
  "VfVfVf">,
  OCLBuiltinVariant<[
   ocl_vec_double_geom,
   ocl_vec_double_geom,
   ocl_vec_double_geom
  ],
  [isSameAs<Id<0>, Id<1>>, isSameAs<Id<1>, Id<2>>],
  "VdVdVd">
]>;

//===----------------------------------------------------------------------===//
// Geometric builtin declarations
//===----------------------------------------------------------------------===//

def blt_cross           : GeometricBuiltin_VfVfVf_VdVdVd<"cross">;

def blt_dot             : GeometricBuiltin_Cfff_Cddd<"dot">;
def blt_distance        : GeometricBuiltin_Cfff_Cddd<"distance">;

def blt_length          : GeometricBuiltin_Cff_Cdd<"length">;

def blt_normalize       : GeometricBuiltin_ff_dd<"normalize">;

def blt_fast_length     : GeometricBuiltin_Cff<"fast_length">;

def blt_fast_normalize  : GeometricBuiltin_ff<"fast_normalize">;
