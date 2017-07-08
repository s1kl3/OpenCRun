class OCLType;

class OCLBasicType : OCLType {
  list<OCLPredicate> Predicates = [];
}

class OCLOpaqueType<string name> : OCLBasicType {
  string Name = name;
}

class OCLOpaqueTypeDef<OCLOpaqueType ty, string define> {
  OCLOpaqueType Type = ty;
  string Define = define;
  list<OCLPredicate> Predicates = [];
  bit isTarget = 0;
}

class OCLTargetOpaqueTypeDef<OCLOpaqueType ty, string define>
 : OCLOpaqueTypeDef<ty, define> {
  let isTarget = 1;
}

def ocl_void : OCLOpaqueType<"void">;
def ocl_ptrdiff_t : OCLOpaqueType<"ptrdiff_t">;
def ocl_size_t : OCLOpaqueType<"size_t">;
def ocl_intptr_t : OCLOpaqueType<"intptr_t">;
def ocl_uintptr_t : OCLOpaqueType<"uintptr_t">;
def ocl_mem_fence_flags : OCLOpaqueType<"cl_mem_fence_flags">;
def ocl_event_t : OCLOpaqueType<"event_t">;
def ocl_sampler_t : OCLOpaqueType<"sampler_t">;
def ocl_image1d_t : OCLOpaqueType<"image1d_t">;
def ocl_image1d_buffer_t : OCLOpaqueType<"image1d_buffer_t">;
def ocl_image1d_array_t : OCLOpaqueType<"image1d_array_t">;
def ocl_image2d_t : OCLOpaqueType<"image2d_t">;
def ocl_image2d_array_t : OCLOpaqueType<"image2d_array_t">;
def ocl_image3d_t : OCLOpaqueType<"image3d_t">;

def : OCLOpaqueTypeDef<ocl_mem_fence_flags, "uint">;

let Predicates = [ocl_arch_32bit] in {
  def : OCLOpaqueTypeDef<ocl_intptr_t, "int">;
  def : OCLOpaqueTypeDef<ocl_uintptr_t, "uint">;
}

let Predicates = [ocl_arch_64bit] in {
  def : OCLOpaqueTypeDef<ocl_intptr_t, "long">;
  def : OCLOpaqueTypeDef<ocl_uintptr_t, "ulong">;
}

class OCLAddressSpace<string name>
 : OCLPredicate<"__opencrun_target_addrspace_", name>;

def ocl_as_private : OCLAddressSpace<"private">;
def ocl_as_global : OCLAddressSpace<"global">;
def ocl_as_local : OCLAddressSpace<"local">;
def ocl_as_constant : OCLAddressSpace<"constant">;

class OCLModifier;

def ocl_mod_const : OCLModifier;
def ocl_mod_restrict : OCLModifier;
def ocl_mod_volatile : OCLModifier;

class OCLPointerType<OCLType base, OCLAddressSpace as, 
                     list<OCLModifier> mod = []> 
 : OCLBasicType, Requires<[as]> {
  OCLType BaseType = base;
  OCLAddressSpace AddressSpace = as;
  list<OCLModifier> Modifiers = mod;
}

class OCLScalarType<int bitwidth> : OCLBasicType {
  int BitWidth = bitwidth;
}

class OCLIntegerType<int bitwidth, bit unsigned> : OCLScalarType<bitwidth> {
  bit Unsigned = unsigned;
}

class OCLRealType<int bitwidth> : OCLScalarType<bitwidth>;

class OCLVectorType<OCLScalarType base, int w> : OCLBasicType {
  OCLScalarType BaseType = base;
  int Width = w;
  let Predicates = base.Predicates;
}

class OCLGroupType<list<OCLType> members> : OCLType {
  list<OCLType> Members = members;
}

multiclass OCLVectors<OCLScalarType base> {
  def _2 : OCLVectorType<base, 2>;
  def _3 : OCLVectorType<base, 3>;
  def _4 : OCLVectorType<base, 4>;
  def _8 : OCLVectorType<base, 8>;
  def _16 : OCLVectorType<base, 16>;
}

def ocl_char : OCLIntegerType<8, 0>;
def ocl_uchar : OCLIntegerType<8, 1>;
def ocl_short : OCLIntegerType<16, 0>;
def ocl_ushort : OCLIntegerType<16, 1>;
def ocl_int : OCLIntegerType<32, 0>;
def ocl_uint : OCLIntegerType<32, 1>;
def ocl_long : OCLIntegerType<64, 0>;
def ocl_ulong : OCLIntegerType<64, 1>;

def ocl_half : OCLRealType<16>, Requires<[ocl_ext_cl_khr_fp16]>;
def ocl_float : OCLRealType<32>;
def ocl_double : OCLRealType<64>, Requires<[ocl_ext_cl_khr_fp64]>;

defm ocl_char : OCLVectors<ocl_char>;
defm ocl_uchar : OCLVectors<ocl_uchar>;
defm ocl_short : OCLVectors<ocl_short>;
defm ocl_ushort : OCLVectors<ocl_ushort>;
defm ocl_int : OCLVectors<ocl_int>;
defm ocl_uint : OCLVectors<ocl_uint>;
defm ocl_long : OCLVectors<ocl_long>;
defm ocl_ulong : OCLVectors<ocl_ulong>;

defm ocl_half : OCLVectors<ocl_half>;
defm ocl_float : OCLVectors<ocl_float>;
defm ocl_double : OCLVectors<ocl_double>;

// Integer vector groups

def ocl_vec_s_char : OCLGroupType<[
  ocl_char_2, ocl_char_3, ocl_char_4, ocl_char_8, ocl_char_16 
]>;

def ocl_vec_u_char : OCLGroupType<[
  ocl_uchar_2, ocl_uchar_3, ocl_uchar_4, ocl_uchar_8, ocl_uchar_16
]>;

def ocl_vec_char : OCLGroupType<[ocl_vec_s_char, ocl_vec_u_char]>;

def ocl_vec_s_short : OCLGroupType<[
  ocl_short_2, ocl_short_3, ocl_short_4, ocl_short_8, ocl_short_16 
]>;

def ocl_vec_u_short : OCLGroupType<[
  ocl_ushort_2, ocl_ushort_3, ocl_ushort_4, ocl_ushort_8, ocl_ushort_16 
]>;

def ocl_vec_short : OCLGroupType<[ocl_vec_s_short, ocl_vec_u_short]>;

def ocl_vec_s_int : OCLGroupType<[
  ocl_int_2, ocl_int_3, ocl_int_4, ocl_int_8, ocl_int_16 
]>;

def ocl_vec_u_int : OCLGroupType<[
  ocl_uint_2, ocl_uint_3, ocl_uint_4, ocl_uint_8, ocl_uint_16 
]>;

def ocl_vec_int : OCLGroupType<[ocl_vec_s_int, ocl_vec_u_int]>;

def ocl_vec_s_long : OCLGroupType<[
  ocl_long_2, ocl_long_3, ocl_long_4, ocl_long_8, ocl_long_16 
]>;

def ocl_vec_u_long : OCLGroupType<[
  ocl_ulong_2, ocl_ulong_3, ocl_ulong_4, ocl_ulong_8, ocl_ulong_16 
]>;

def ocl_vec_long : OCLGroupType<[ocl_vec_s_long, ocl_vec_u_long]>;

// Real vector groups

def ocl_vec_float : OCLGroupType<[
  ocl_float_2, ocl_float_3, ocl_float_4, ocl_float_8, ocl_float_16
]>;

def ocl_vec_double : OCLGroupType<[
  ocl_double_2, ocl_double_3, ocl_double_4, ocl_double_8, ocl_double_16
]>;

// Integer scalar gentype

def ocl_sgentype_s_integer : OCLGroupType<[
  ocl_char, ocl_short, ocl_int, ocl_long
]>;

def ocl_sgentype_u_integer : OCLGroupType<[
  ocl_uchar, ocl_ushort, ocl_uint, ocl_ulong
]>;

def ocl_sgentype_integer : OCLGroupType<[
  ocl_sgentype_s_integer, ocl_sgentype_u_integer
]>;

// Real scalar gentype

def ocl_sgentype_real : OCLGroupType<[
  ocl_float, ocl_double
]>;

// All scalar gentype

def ocl_sgentype : OCLGroupType<[
  ocl_sgentype_integer, ocl_sgentype_real
]>;

// Integer vectorial gentype

def ocl_vgentype_s_integer : OCLGroupType<[
  ocl_vec_s_char, ocl_vec_s_short, ocl_vec_s_int, ocl_vec_s_long
]>;

def ocl_vgentype_u_integer : OCLGroupType<[
  ocl_vec_u_char, ocl_vec_u_short, ocl_vec_u_int, ocl_vec_u_long
]>;

def ocl_vgentype_integer : OCLGroupType<[
  ocl_vgentype_s_integer, ocl_vgentype_u_integer
]>;

// Real vectorial gentype

def ocl_vgentype_float : OCLGroupType<[
  ocl_vec_float
]>;

def ocl_vgentype_double : OCLGroupType<[
  ocl_vec_double
]>;

def ocl_vgentype_real : OCLGroupType<[
  ocl_vgentype_float, ocl_vgentype_double
]>;

// All vectorial gentype

def ocl_vgentype : OCLGroupType<[
  ocl_vgentype_integer, ocl_vgentype_real
]>;

// Integer gentype

def ocl_gentype_s_integer : OCLGroupType<[
  ocl_sgentype_s_integer, ocl_vgentype_s_integer
]>;

def ocl_gentype_u_integer : OCLGroupType<[
  ocl_sgentype_u_integer, ocl_vgentype_u_integer
]>;

def ocl_gentype_integer : OCLGroupType<[
  ocl_gentype_s_integer, ocl_gentype_u_integer
]>;

// Real gentype

def ocl_gentype_float : OCLGroupType<[
  ocl_float, ocl_vec_float
]>;

def ocl_gentype_double : OCLGroupType<[
  ocl_double, ocl_vec_double
]>;

def ocl_gentype_real : OCLGroupType<[
  ocl_gentype_float, ocl_gentype_double
]>;

// Global gentype

def ocl_gentype : OCLGroupType<[
  ocl_gentype_integer, ocl_gentype_real
]>;

// Scalar and vectorial integer types

def ocl_s_char : OCLGroupType<[
  ocl_char, ocl_vec_s_char
]>;

def ocl_u_char : OCLGroupType<[
  ocl_uchar, ocl_vec_u_char
]>;

def ocl_s_short : OCLGroupType<[
  ocl_short, ocl_vec_s_short
]>;

def ocl_u_short : OCLGroupType<[
  ocl_ushort, ocl_vec_u_short
]>;

def ocl_s_int : OCLGroupType<[
  ocl_int, ocl_vec_s_int
]>;

def ocl_u_int : OCLGroupType<[
  ocl_uint, ocl_vec_u_int
]>;

def ocl_s_long : OCLGroupType<[
  ocl_long, ocl_vec_s_long
]>;

def ocl_u_long : OCLGroupType<[
  ocl_ulong, ocl_vec_u_long
]>;

// Integer and real vectorial gentypes for each specific size

def ocl_vgentype_2 : OCLGroupType<[
  ocl_char_2, 
  ocl_uchar_2, 
  ocl_short_2, 
  ocl_ushort_2,
  ocl_int_2,
  ocl_uint_2,
  ocl_long_2,
  ocl_ulong_2,
  ocl_float_2,
  ocl_double_2
]>;

def ocl_vgentype_3 : OCLGroupType<[
  ocl_char_3, 
  ocl_uchar_3, 
  ocl_short_3, 
  ocl_ushort_3,
  ocl_int_3,
  ocl_uint_3,
  ocl_long_3,
  ocl_ulong_3,
  ocl_float_3,
  ocl_double_3
]>;

def ocl_vgentype_4 : OCLGroupType<[
  ocl_char_4, 
  ocl_uchar_4, 
  ocl_short_4, 
  ocl_ushort_4,
  ocl_int_4,
  ocl_uint_4,
  ocl_long_4,
  ocl_ulong_4,
  ocl_float_4,
  ocl_double_4
]>;

def ocl_vgentype_8 : OCLGroupType<[
  ocl_char_8, 
  ocl_uchar_8, 
  ocl_short_8, 
  ocl_ushort_8,
  ocl_int_8,
  ocl_uint_8,
  ocl_long_8,
  ocl_ulong_8,
  ocl_float_8,
  ocl_double_8
]>;

def ocl_vgentype_16 : OCLGroupType<[
  ocl_char_16, 
  ocl_uchar_16, 
  ocl_short_16, 
  ocl_ushort_16,
  ocl_int_16,
  ocl_uint_16,
  ocl_long_16,
  ocl_ulong_16,
  ocl_float_16,
  ocl_double_16
]>;

// Geometric builtins custom types

def ocl_gentype_float_geom : OCLGroupType<[
  ocl_float, ocl_float_2, ocl_float_3, ocl_float_4
]>;

def ocl_gentype_double_geom : OCLGroupType<[
  ocl_double, ocl_double_2, ocl_double_3, ocl_double_4
]>;

def ocl_gentype_real_geom : OCLGroupType<[
  ocl_gentype_float_geom, ocl_gentype_double_geom
]>;

// Integer builtins custom types

def ocl_gentype_integer_int : OCLGroupType<[
  ocl_int, ocl_vec_s_int,
  ocl_uint, ocl_vec_u_int
]>;

// Relational builtins custom types

def ocl_s_long_rel : OCLGroupType<[
  ocl_int, ocl_vec_s_long
]>;

// Misc. Vector builtins custom types

def ocl_vgentype_vmisc : OCLGroupType<[
  ocl_vgentype_2,
  ocl_vgentype_4,
  ocl_vgentype_8,
  ocl_vgentype_16
]>;

def ocl_gentype_vmisc : OCLGroupType<[
  ocl_sgentype_integer, ocl_sgentype_real, ocl_vgentype_vmisc  
]>;

def ocl_gentype_uinteger_vmisc : OCLGroupType<[
  ocl_uchar_2, ocl_uchar_4, ocl_uchar_8, ocl_uchar_16,
  ocl_ushort_2, ocl_ushort_4, ocl_ushort_8, ocl_ushort_16,
  ocl_uint_2, ocl_uint_4, ocl_uint_8, ocl_uint_16,
  ocl_ulong_2, ocl_ulong_4, ocl_ulong_8, ocl_ulong_16
]>;

// Vector Data Load and Store builtins custom types

def ocl_sgentype_vldst : OCLGroupType<[
  ocl_sgentype_integer, ocl_sgentype_real
]>;

// Image builtins custom types

def ocl_image_array : OCLGroupType<[
  ocl_image1d_array_t, ocl_image2d_array_t
]>;

def ocl_image_1d : OCLGroupType<[
  ocl_image1d_t, ocl_image1d_buffer_t, ocl_image1d_array_t
]>;

def ocl_image_2d : OCLGroupType<[
  ocl_image2d_t, ocl_image2d_array_t
]>;

def ocl_image_1d_2d : OCLGroupType<[
  ocl_image1d_t, ocl_image1d_buffer_t, ocl_image1d_array_t,
  ocl_image2d_t, ocl_image2d_array_t
]>;

def ocl_image_2d_3d : OCLGroupType<[
  ocl_image2d_t, ocl_image2d_array_t,
  ocl_image3d_t
]>;

def ocl_image : OCLGroupType<[
  ocl_image_1d,
  ocl_image_2d,
  ocl_image3d_t
]>;

def ocl_image_no_buff : OCLGroupType<[
  ocl_image1d_t, ocl_image1d_array_t,
  ocl_image_2d,
  ocl_image3d_t
]>;

def ocl_coord_int : OCLGroupType<[
  ocl_int, ocl_int_2, ocl_int_4
]>;

def ocl_coord_float : OCLGroupType<[
  ocl_float, ocl_float_2, ocl_float_4
]>;

def ocl_coord : OCLGroupType<[
  ocl_coord_int, ocl_coord_float
]>;
