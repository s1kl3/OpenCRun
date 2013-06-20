//===----------------------------------------------------------------------===//
// Geometric builtin classes
//===----------------------------------------------------------------------===//

let Group = "GEOMETRIC" in {

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
  [isSameAs<Id<1>, Id<2>>]
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

class GeometricBuiltin_V3fV3fV3f_V4fV4fV4f_V3dV3dV3d_V4dV4dV4d<string name> : 
OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_float_3,
    ocl_float_3,
    ocl_float_3
  ],
  [],
  "V3fV3fV3f">,
  OCLBuiltinVariant<[
    ocl_float_4,
    ocl_float_4,
    ocl_float_4
  ],
  [],
  "V4fV4fV4f">,
  OCLBuiltinVariant<[
   ocl_double_3,
   ocl_double_3,
   ocl_double_3
  ],
  [],
  "V3dV3dV3d">,
  OCLBuiltinVariant<[
   ocl_double_4,
   ocl_double_4,
   ocl_double_4
  ],
  [],
  "V4dV4dV4d">
]>;

}

//===----------------------------------------------------------------------===//
// Geometric builtin declarations
//===----------------------------------------------------------------------===//

def blt_cross : GeometricBuiltin_V3fV3fV3f_V4fV4fV4f_V3dV3dV3d_V4dV4dV4d<"cross">;

def blt_dot             : GeometricBuiltin_Cfff_Cddd<"dot">;
def blt_distance        : GeometricBuiltin_Cfff_Cddd<"distance">;

def blt_length          : GeometricBuiltin_Cff_Cdd<"length">;

def blt_normalize       : GeometricBuiltin_ff_dd<"normalize">;

def blt_fast_distance   : GeometricBuiltin_Cfff<"fast_distance">;

def blt_fast_length     : GeometricBuiltin_Cff<"fast_length">;

def blt_fast_normalize  : GeometricBuiltin_ff<"fast_normalize">;

//===----------------------------------------------------------------------===//
// Geometric builtin implementations
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_cross, TemplateStrategy<[{
  return (float3) (param1.y * param2.z - param1.z * param2.y,
                   param1.z * param2.x - param1.x * param2.z,     
                   param1.x * param2.y - param1.y * param2.x);
}]>, "V3fV3fV3f">;

def : OCLBuiltinImpl<blt_cross, TemplateStrategy<[{
  return (float4) (param1.y * param2.z - param1.z * param2.y,
                   param1.z * param2.x - param1.x * param2.z,     
                   param1.x * param2.y - param1.y * param2.x,
                   0.0f);
}]>, "V4fV4fV4f">;

def : OCLBuiltinImpl<blt_cross, TemplateStrategy<[{
  return (double3) (param1.y * param2.z - param1.z * param2.y,
                    param1.z * param2.x - param1.x * param2.z,     
                    param1.x * param2.y - param1.y * param2.x);
}]>, "V3dV3dV3d">;

def : OCLBuiltinImpl<blt_cross, TemplateStrategy<[{
  return (double4) (param1.y * param2.z - param1.z * param2.y,
                    param1.z * param2.x - param1.x * param2.z,     
                    param1.x * param2.y - param1.y * param2.x,
                    0.0);
}]>, "V4dV4dV4d">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_dot, MergeSplit<[{
  return param1 * param2;
}], "+">>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_distance, TemplateStrategy<[{
  return __builtin_ocl_length(param1 - param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_length, TemplateStrategy<[{
  return __builtin_sqrtf(__builtin_ocl_dot(param1, param1));
}]>, "Cff">;

def : OCLBuiltinImpl<blt_length, TemplateStrategy<[{
  return __builtin_sqrt(__builtin_ocl_dot(param1, param1));
}]>, "Cdd">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_normalize, TemplateStrategy<[{
  return param1 / __builtin_ocl_length(param1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fast_distance, TemplateStrategy<[{
  return __builtin_ocl_fast_length(param1 - param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fast_length, TemplateStrategy<[{
  return __builtin_ocl_half_sqrt(__builtin_ocl_dot(param1, param1));
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_fast_normalize, TemplateStrategy<[{
  return param1 * __builtin_ocl_half_rsqrt(__builtin_ocl_dot(param1, param1));
}]>>;
