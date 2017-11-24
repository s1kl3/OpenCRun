//===----------------------------------------------------------------------===//
// Image builtin classes
//===----------------------------------------------------------------------===//

let Group = "IMAGE" in {

class ImageBuiltin_VnQrIMg_VnQrIn_VnQrDbICn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int_4,
    OCLQualifiedType<ocl_image_no_buff, ocl_qual_readonly>,
    ocl_sampler_t,
    ocl_coord
  ],
  [isCompatibleWithImageDim<Id<1>, Id<3>>],
  "VnQrIMg">,
  OCLBuiltinVariant<[
    ocl_int_4,
    OCLQualifiedType<ocl_image_no_buff, ocl_qual_readonly>,
    ocl_coord_int
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "VnQrIn">,
  OCLBuiltinVariant<[
    ocl_int_4,
    OCLQualifiedType<ocl_image1d_buffer_t, ocl_qual_readonly>,
    ocl_int
  ],
  [],
  "VnQrDbICn">
]>;

class ImageBuiltin_VUnQrIMg_VUnQrIn_VUnQrDbICn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_uint_4,
    OCLQualifiedType<ocl_image_no_buff, ocl_qual_readonly>,
    ocl_sampler_t,
    ocl_coord
  ],
  [isCompatibleWithImageDim<Id<1>, Id<3>>],
  "VUnQrIMg">,
  OCLBuiltinVariant<[
    ocl_uint_4,
    OCLQualifiedType<ocl_image_no_buff, ocl_qual_readonly>,
    ocl_coord_int
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "VUnQrIn">,
  OCLBuiltinVariant<[
    ocl_uint_4,
    OCLQualifiedType<ocl_image1d_buffer_t, ocl_qual_readonly>,
    ocl_int
  ],
  [],
  "VUnQrDbICn">
]>;

class ImageBuiltin_VfQrIMn_VfQrIMf_VfQrIn_VfQrDbICn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_float_4,
    OCLQualifiedType<ocl_image_no_buff, ocl_qual_readonly>,
    ocl_sampler_t,
    ocl_coord_int
  ],
  [isCompatibleWithImageDim<Id<1>, Id<3>>],
  "VfQrIMn">,
  OCLBuiltinVariant<[
    ocl_float_4,
    OCLQualifiedType<ocl_image_no_buff, ocl_qual_readonly>,
    ocl_sampler_t,
    ocl_coord_float
  ],
  [isCompatibleWithImageDim<Id<1>, Id<3>>],
  "VfQrIMf">,
  OCLBuiltinVariant<[
    ocl_float_4,
    OCLQualifiedType<ocl_image_no_buff, ocl_qual_readonly>,
    ocl_coord_int
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "VfQrIn">,
  OCLBuiltinVariant<[
    ocl_float_4,
    OCLQualifiedType<ocl_image1d_buffer_t, ocl_qual_readonly>,
    ocl_int
  ],
  [],
  "VfQrDbICn">
]>;

class ImageBuiltin_vQwD12InVn_vQwD3IVnVn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_void,
    OCLQualifiedType<ocl_image_1d_2d, ocl_qual_writeonly>,
    ocl_coord_int,
    ocl_int_4
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "vQwD12InVn">,
  OCLBuiltinVariant<[
    ocl_void,
    OCLQualifiedType<ocl_image3d_t, ocl_qual_writeonly>,
    ocl_int_4,
    ocl_int_4
  ],
  [],
  "vQwD3IVnVn",
  [ocl_ext_cl_khr_3d_image_writes]>
]>;

class ImageBuiltin_vQwD12InVUn_vQwD3IVnVUn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_void,
    OCLQualifiedType<ocl_image_1d_2d, ocl_qual_writeonly>,
    ocl_coord_int,
    ocl_uint_4
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "vQwD12InVUn">,
  OCLBuiltinVariant<[
    ocl_void,
    OCLQualifiedType<ocl_image3d_t, ocl_qual_writeonly>,
    ocl_int_4,
    ocl_uint_4
  ],
  [],
  "vQwD3IVnVUn",
  [ocl_ext_cl_khr_3d_image_writes]>
]>;

class ImageBuiltin_vQwD12InVf_vQwD3IVnVf<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_void,
    OCLQualifiedType<ocl_image_1d_2d, ocl_qual_writeonly>,
    ocl_coord_int,
    ocl_float_4
  ],
  [isCompatibleWithImageDim<Id<1>, Id<2>>],
  "vQwD12InVf">,
  OCLBuiltinVariant<[
    ocl_void,
    OCLQualifiedType<ocl_image3d_t, ocl_qual_writeonly>,
    ocl_int_4,
    ocl_float_4
  ],
  [],
  "vQwD3IVnVf",
  [ocl_ext_cl_khr_3d_image_writes]>
]>;

class ImageBuiltin_nQrI_nQwD12I_nQwD3I<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLQualifiedType<ocl_image, ocl_qual_readonly>
  ],
  [],
  "nQrI">,
  OCLBuiltinVariant<[
    ocl_int,
    OCLQualifiedType<ocl_image_1d_2d, ocl_qual_writeonly>
  ],
  [],
  "nQwD12I">,
  OCLBuiltinVariant<[
    ocl_int,
    OCLQualifiedType<ocl_image3d_t, ocl_qual_writeonly>
  ],
  [],
  "nQwD3I",
  [ocl_ext_cl_khr_3d_image_writes]>
]>;

class ImageBuiltin_nQrD23I_nQwD2I_nQwD3I<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLQualifiedType<ocl_image_2d_3d, ocl_qual_readonly>
  ],
  [],
  "nQrD23I">,
  OCLBuiltinVariant<[
    ocl_int,
    OCLQualifiedType<ocl_image_2d, ocl_qual_writeonly>
  ],
  [],
  "nQwD2I">,
  OCLBuiltinVariant<[
    ocl_int,
    OCLQualifiedType<ocl_image3d_t, ocl_qual_writeonly>
  ],
  [],
  "nQwD3I",
  [ocl_ext_cl_khr_3d_image_writes]>
]>;

class ImageBuiltin_nQrD3I_nQwD3I<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLQualifiedType<ocl_image3d_t, ocl_qual_readonly>
  ],
  [],
  "nQrD3I">,
  OCLBuiltinVariant<[
    ocl_int,
    OCLQualifiedType<ocl_image3d_t, ocl_qual_writeonly>
  ],
  [],
  "nQwD3I",
  [ocl_ext_cl_khr_3d_image_writes]>
]>;

class ImageBuiltin_zQrDaI_zQwDaI<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_size_t,
    OCLQualifiedType<ocl_image_array, ocl_qual_readonly>
  ],
  [],
  "zQrDaI">,
  OCLBuiltinVariant<[
    ocl_size_t,
    OCLQualifiedType<ocl_image_array, ocl_qual_writeonly>
  ],
  [],
  "zQwDaI">
]>;

class ImageBuiltin_VnQrD2I_VnQwD2I_VnQrD3I_VnQwD3I<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int_2,
    OCLQualifiedType<ocl_image_2d, ocl_qual_readonly>
  ],
  [],
  "VnQrD2I">,
  OCLBuiltinVariant<[
    ocl_int_2,
    OCLQualifiedType<ocl_image_2d, ocl_qual_writeonly>
  ],
  [],
  "VnQwD2I">,
  OCLBuiltinVariant<[
    ocl_int_4,
    OCLQualifiedType<ocl_image3d_t, ocl_qual_readonly>
  ],
  [],
  "VnQrD3I">,
  OCLBuiltinVariant<[
    ocl_int_4,
    OCLQualifiedType<ocl_image3d_t, ocl_qual_writeonly>
  ],
  [],
  "VnQwD3I",
  [ocl_ext_cl_khr_3d_image_writes]>
]>;

}

//===----------------------------------------------------------------------===//
// Image builtin declarations
//===----------------------------------------------------------------------===//

def blt_read_imagei                 : ImageBuiltin_VnQrIMg_VnQrIn_VnQrDbICn<"read_imagei">;
def blt_read_imageui                : ImageBuiltin_VUnQrIMg_VUnQrIn_VUnQrDbICn<"read_imageui">;
def blt_read_imagef                 : ImageBuiltin_VfQrIMn_VfQrIMf_VfQrIn_VfQrDbICn<"read_imagef">;

def blt_write_imagei                : ImageBuiltin_vQwD12InVn_vQwD3IVnVn<"write_imagei">;
def blt_write_imageui               : ImageBuiltin_vQwD12InVUn_vQwD3IVnVUn<"write_imageui">;
def blt_write_imagef                : ImageBuiltin_vQwD12InVf_vQwD3IVnVf<"write_imagef">;

def blt_get_image_width             : ImageBuiltin_nQrI_nQwD12I_nQwD3I<"get_image_width">;
def blt_get_image_height            : ImageBuiltin_nQrD23I_nQwD2I_nQwD3I<"get_image_height">;
def blt_get_image_depth             : ImageBuiltin_nQrD3I_nQwD3I<"get_image_depth">;
def blt_get_image_channel_data_type : ImageBuiltin_nQrI_nQwD12I_nQwD3I<"get_image_channel_data_type">;
def blt_get_image_channel_order     : ImageBuiltin_nQrI_nQwD12I_nQwD3I<"get_image_channel_order">;
def blt_get_image_dim               : ImageBuiltin_VnQrD2I_VnQwD2I_VnQrD3I_VnQwD3I<"get_image_dim">;
def blt_get_image_array_size        : ImageBuiltin_zQrDaI_zQwDaI<"get_image_array_size">;
