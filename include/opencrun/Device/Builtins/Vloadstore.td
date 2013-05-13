//===----------------------------------------------------------------------===//
// Vector Data Load and Store builtin classes
//===----------------------------------------------------------------------===//

class VloadstoreBuiltinVgzPSg_2<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_2, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_2,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_2, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_constant, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_2, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>
]>;

class VloadstoreBuiltinVgzPSg_3<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_3, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_3,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_3, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_constant, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_3, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>
]>;

class VloadstoreBuiltinVgzPSg_4<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_4, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_4,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_4, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_constant, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_4, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>
]>;

class VloadstoreBuiltinVgzPSg_8<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_8, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_8,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_8, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_constant, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_8, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>
]>;

class VloadstoreBuiltinVgzPSg_16<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_16, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_16,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_16, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_constant, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>,
  OCLBuiltinVariant<[
     ocl_vgentype_16, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>]>
]>;

class VloadstoreBuiltin_vVgzPSg_2<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_void,
     ocl_vgentype_2,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_2,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_2, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>
]>;

class VloadstoreBuiltin_vVgzPSg_3<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_void,
     ocl_vgentype_3,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_3,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_3, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>
]>;

class VloadstoreBuiltin_vVgzPSg_4<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_void,
     ocl_vgentype_4,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_4,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_4, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>
]>;

class VloadstoreBuiltin_vVgzPSg_8<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_void,
     ocl_vgentype_8,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_8,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_8, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>
]>;

class VloadstoreBuiltin_vVgzPSg_16<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_void,
     ocl_vgentype_16,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_16,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_16, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>]>
]>;

//===----------------------------------------------------------------------===//
// Vector Data Load and Store builtin declarations
//===----------------------------------------------------------------------===//

def blt_vload2      : VloadstoreBuiltinVgzPSg_2<"vload2">;

def blt_vload3      : VloadstoreBuiltinVgzPSg_3<"vload3">;

def blt_vload4      : VloadstoreBuiltinVgzPSg_4<"vload4">;

def blt_vload8      : VloadstoreBuiltinVgzPSg_8<"vload8">;

def blt_vload16     : VloadstoreBuiltinVgzPSg_16<"vload16">;

def blt_vstore2     : VloadstoreBuiltin_vVgzPSg_2<"vstore2">;

def blt_vstore3     : VloadstoreBuiltin_vVgzPSg_3<"vstore3">;

def blt_vstore4     : VloadstoreBuiltin_vVgzPSg_4<"vstore4">;

def blt_vstore8     : VloadstoreBuiltin_vVgzPSg_8<"vstore8">;

def blt_vstore16    : VloadstoreBuiltin_vVgzPSg_16<"vstore16">;
