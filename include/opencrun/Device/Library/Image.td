//===----------------------------------------------------------------------===//
// Image builtin classes
//===----------------------------------------------------------------------===//

let Group = "IMAGE" in {

class ImageBuiltin_VnIMg_VnIn_VnDbICn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int_4,
    ocl_image_no_buff,
    ocl_sampler_t,
    ocl_coord
  ],
  [isCompatibleWithImageDim<Id<1>, Id<3>>],
  "VnIMg">,
  OCLBuiltinVariant<[
    ocl_int_4,
    ocl_image_no_buff,
    ocl_coord_int
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "VnIn">,
  OCLBuiltinVariant<[
    ocl_int_4,
    ocl_image1d_buffer_t,
    ocl_int
  ],
  [],
  "VnDbICn">
]>;

class ImageBuiltin_VUnIMg_VUnIn_VUnDbICn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_uint_4,
    ocl_image_no_buff,
    ocl_sampler_t,
    ocl_coord
  ],
  [isCompatibleWithImageDim<Id<1>, Id<3>>],
  "VUnIMg">,
  OCLBuiltinVariant<[
    ocl_uint_4,
    ocl_image_no_buff,
    ocl_coord_int
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "VUnIn">,
  OCLBuiltinVariant<[
    ocl_uint_4,
    ocl_image1d_buffer_t,
    ocl_int
  ],
  [],
  "VUnDbICn">
]>;

class ImageBuiltin_VfIMn_VfIMf_VfIn_VfDbICn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_float_4,
    ocl_image_no_buff,
    ocl_sampler_t,
    ocl_coord_int
  ],
  [isCompatibleWithImageDim<Id<1>, Id<3>>],
  "VfIMn">,
  OCLBuiltinVariant<[
    ocl_float_4,
    ocl_image_no_buff,
    ocl_sampler_t,
    ocl_coord_float
  ],
  [isCompatibleWithImageDim<Id<1>, Id<3>>],
  "VfIMf">,
  OCLBuiltinVariant<[
    ocl_float_4,
    ocl_image_no_buff,
    ocl_coord_int
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "VfIn">,
  OCLBuiltinVariant<[
    ocl_float_4,
    ocl_image1d_buffer_t,
    ocl_int
  ],
  [],
  "VfDbICn">
]>;

class ImageBuiltin_vD12InVn_vD3IVnVn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_void,
    ocl_image_1d_2d,
    ocl_coord_int,
    ocl_int_4
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "vD12InVn">,
  OCLBuiltinVariant<[
    ocl_void,
    ocl_image3d_t,
    ocl_int_4,
    ocl_int_4
  ],
  [],
  "vD3IVnVn",
  [ocl_ext_cl_khr_3d_image_writes]>
]>;

class ImageBuiltin_vD12InVUn_vD3IVnVUn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_void,
    ocl_image_1d_2d,
    ocl_coord_int,
    ocl_uint_4
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "vD12InVUn">,
  OCLBuiltinVariant<[
    ocl_void,
    ocl_image3d_t,
    ocl_int_4,
    ocl_uint_4
  ],
  [],
  "vD3IVnVUn",
  [ocl_ext_cl_khr_3d_image_writes]>
]>;

class ImageBuiltin_vD12InVf_vD3IVnVf<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_void,
    ocl_image_1d_2d,
    ocl_coord_int,
    ocl_float_4
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "vD12InVf">,
  OCLBuiltinVariant<[
    ocl_void,
    ocl_image3d_t,
    ocl_int_4,
    ocl_float_4
  ],
  [],
  "vD3IVnVf",
  [ocl_ext_cl_khr_3d_image_writes]>
]>;

class ImageBuiltin_nI<string name> : OCLGenericBuiltinSimple<
  name,
  [ocl_int, ocl_image],
  []
>;

class ImageBuiltin_nD23I<string name> : OCLGenericBuiltinSimple<
  name,
  [ocl_int, ocl_image_2d_3d],
  []
>;

class ImageBuiltin_nD3I<string name> : OCLGenericBuiltinSimple<
  name,
  [ocl_int, ocl_image3d_t],
  []
>;

class ImageBuiltin_zDaI<string name> : OCLGenericBuiltinSimple<
  name,
  [ocl_size_t, ocl_image_array],
  []
>;

class ImageBuiltin_VnD2I_VnD3I<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int_2,
    ocl_image_2d
  ],
  [],
  "VnD2I">,
  OCLBuiltinVariant<[
    ocl_int_4,
    ocl_image3d_t
  ],
  [],
  "VnD3I">
]>;

}

//===----------------------------------------------------------------------===//
// Image builtin declarations
//===----------------------------------------------------------------------===//

def blt_read_imagei                 : ImageBuiltin_VnIMg_VnIn_VnDbICn<"read_imagei">;
def blt_read_imageui                : ImageBuiltin_VUnIMg_VUnIn_VUnDbICn<"read_imageui">;
def blt_read_imagef                 : ImageBuiltin_VfIMn_VfIMf_VfIn_VfDbICn<"read_imagef">;

def blt_write_imagei                : ImageBuiltin_vD12InVn_vD3IVnVn<"write_imagei">;
def blt_write_imageui               : ImageBuiltin_vD12InVUn_vD3IVnVUn<"write_imageui">;
def blt_write_imagef                : ImageBuiltin_vD12InVf_vD3IVnVf<"write_imagef">;

def blt_get_image_width             : ImageBuiltin_nI<"get_image_width">;
def blt_get_image_height            : ImageBuiltin_nD23I<"get_image_height">;
def blt_get_image_depth             : ImageBuiltin_nD3I<"get_image_depth">;
def blt_get_image_channel_data_type : ImageBuiltin_nI<"get_image_channel_data_type">;
def blt_get_image_channel_order     : ImageBuiltin_nI<"get_image_channel_order">;
def blt_get_image_dim               : ImageBuiltin_VnD2I_VnD3I<"get_image_dim">;
def blt_get_image_array_size        : ImageBuiltin_zDaI<"get_image_array_size">;
