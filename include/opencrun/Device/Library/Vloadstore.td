//===----------------------------------------------------------------------===//
// Vector Data Load and Store builtin classes
//===----------------------------------------------------------------------===//

let Group = "VLOADSTORE" in {

class VloadstoreBuiltin_VgzP1Sg_VgzP2Sg_VgzP3Sg_VgzP4Sg_2<string name> 
: OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_2, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP1Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_2,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP2Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_2, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_constant, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP3Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_2, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP4Sg">
]>;

class VloadstoreBuiltin_VgzP1Sg_VgzP2Sg_VgzP3Sg_VgzP4Sg_3<string name> 
: OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_3, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP1Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_3,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP2Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_3, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_constant, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP3Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_3, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP4Sg">
]>;

class VloadstoreBuiltin_VgzP1Sg_VgzP2Sg_VgzP3Sg_VgzP4Sg_4<string name> 
: OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_4, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP1Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_4,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP2Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_4, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_constant, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP3Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_4, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP4Sg">
]>;

class VloadstoreBuiltin_VgzP1Sg_VgzP2Sg_VgzP3Sg_VgzP4Sg_8<string name> 
: OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_8, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP1Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_8,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP2Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_8, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_constant, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP3Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_8, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP4Sg">
]>;

class VloadstoreBuiltin_VgzP1Sg_VgzP2Sg_VgzP3Sg_VgzP4Sg_16<string name> 
: OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_vgentype_16, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP1Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_16,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP2Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_16, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_constant, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP3Sg">,
  OCLBuiltinVariant<[
     ocl_vgentype_16, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<0>,Pointee<2>>], "VgzP4Sg">
]>;

class VloadstoreBuiltin_vVgzP1Sg_vVgzP2Sg_vVgzP4Sg_2<string name> 
: OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_void,
     ocl_vgentype_2,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP1Sg">,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_2,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP2Sg">,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_2, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP4Sg">
]>;

class VloadstoreBuiltin_vVgzP1Sg_vVgzP2Sg_vVgzP4Sg_3<string name> 
: OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_void,
     ocl_vgentype_3,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP1Sg">,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_3,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP2Sg">,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_3, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP4Sg">
]>;

class VloadstoreBuiltin_vVgzP1Sg_vVgzP2Sg_vVgzP4Sg_4<string name> 
: OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_void,
     ocl_vgentype_4,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP1Sg">,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_4,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP2Sg">,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_4, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP4Sg">
]>;

class VloadstoreBuiltin_vVgzP1Sg_vVgzP2Sg_vVgzP4Sg_8<string name> 
: OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_void,
     ocl_vgentype_8,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP1Sg">,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_8,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP2Sg">,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_8, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP4Sg">
]>;

class VloadstoreBuiltin_vVgzP1Sg_vVgzP2Sg_vVgzP4Sg_16<string name> 
: OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_void,
     ocl_vgentype_16,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_global, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP1Sg">,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_16,
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_local, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP2Sg">,
  OCLBuiltinVariant<[
     ocl_void, 
     ocl_vgentype_16, 
     ocl_size_t, 
     OCLPointerType<ocl_sgentype_vldst, ocl_as_private, []>
  ], [isSameBaseAs<Id<1>,Pointee<3>>], "vVgzP4Sg">
]>;

}

//===----------------------------------------------------------------------===//
// Vector Data Load and Store builtin declarations
//===----------------------------------------------------------------------===//

def blt_vload2      : VloadstoreBuiltin_VgzP1Sg_VgzP2Sg_VgzP3Sg_VgzP4Sg_2<"vload2">;

def blt_vload3      : VloadstoreBuiltin_VgzP1Sg_VgzP2Sg_VgzP3Sg_VgzP4Sg_3<"vload3">;

def blt_vload4      : VloadstoreBuiltin_VgzP1Sg_VgzP2Sg_VgzP3Sg_VgzP4Sg_4<"vload4">;

def blt_vload8      : VloadstoreBuiltin_VgzP1Sg_VgzP2Sg_VgzP3Sg_VgzP4Sg_8<"vload8">;

def blt_vload16     : VloadstoreBuiltin_VgzP1Sg_VgzP2Sg_VgzP3Sg_VgzP4Sg_16<"vload16">;

def blt_vstore2     : VloadstoreBuiltin_vVgzP1Sg_vVgzP2Sg_vVgzP4Sg_2<"vstore2">;

def blt_vstore3     : VloadstoreBuiltin_vVgzP1Sg_vVgzP2Sg_vVgzP4Sg_3<"vstore3">;

def blt_vstore4     : VloadstoreBuiltin_vVgzP1Sg_vVgzP2Sg_vVgzP4Sg_4<"vstore4">;

def blt_vstore8     : VloadstoreBuiltin_vVgzP1Sg_vVgzP2Sg_vVgzP4Sg_8<"vstore8">;

def blt_vstore16    : VloadstoreBuiltin_vVgzP1Sg_vVgzP2Sg_vVgzP4Sg_16<"vstore16">;
