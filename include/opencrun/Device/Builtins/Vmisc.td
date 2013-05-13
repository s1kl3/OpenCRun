//===----------------------------------------------------------------------===//
// Misc. Vector builtin classes
//===----------------------------------------------------------------------===//

class VmiscBuiltin_Cng_CnVg<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    ocl_gentype_vmisc
  ],
  []>,
  OCLBuiltinVariant<[
    ocl_int,
    ocl_vgentype_3
  ],
  []>
]>;

class VmiscBuiltin_ggUi_gggUi<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
     ocl_gentype_vmisc,
     ocl_gentype_vmisc,
     ocl_gentype_uinteger_vmisc
  ],
  [
     isSameBaseAs<Id<0>, Id<1>>, 
     isSameDimAs<Id<0>, Id<2>>, 
     isSameBaseSizeAs<Id<0>, Id<2>>
  ]>,
  OCLBuiltinVariant<[
     ocl_gentype_vmisc,
     ocl_gentype_vmisc,
     ocl_gentype_vmisc,
     ocl_gentype_uinteger_vmisc
  ],
  [
     isSameAs<Id<1>, Id<2>>,
     isSameBaseAs<Id<0>, Id<1>>, 
     isSameDimAs<Id<0>, Id<3>>,
     isSameBaseSizeAs<Id<0>, Id<3>>
  ]>
]>;

//===----------------------------------------------------------------------===//
// Misc. Vector builtin declarations
//===----------------------------------------------------------------------===//

def blt_vec_step    : VmiscBuiltin_Cng_CnVg<"vec_step">;

def blt_shuffle     : VmiscBuiltin_ggUi_gggUi<"shuffle">;
def blt_shuffle2    : VmiscBuiltin_ggUi_gggUi<"shuffle2">;
