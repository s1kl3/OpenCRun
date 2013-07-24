//===----------------------------------------------------------------------===//
// Misc. Vector builtin classes
//===----------------------------------------------------------------------===//

let Group = "VMISC" in {

// class VmiscBuiltin_Cng_CnVg<string name> : OCLMultiBuiltin<name, [
//   OCLBuiltinVariant<[
//     ocl_int,
//     ocl_gentype_vmisc
//   ],
//   [], "Cng">,
//   OCLBuiltinVariant<[
//     ocl_int,
//     ocl_vgentype_3
//   ],
//   [], "CnVg">
// ]>;

class VmiscBuiltin_ggUi<string name> : OCLSimpleBuiltin<name,
  [
     ocl_vgentype_vmisc,
     ocl_vgentype_vmisc,
     ocl_gentype_uinteger_vmisc
  ],
  [
     isSameBaseAs<Id<0>, Id<1>>, 
     isSameDimAs<Id<0>, Id<2>>, 
     isSameBaseSizeAs<Id<0>, Id<2>>
  ]
>;

class VmiscBuiltin_gggUi<string name> : OCLSimpleBuiltin<name,
  [
     ocl_vgentype_vmisc,
     ocl_vgentype_vmisc,
     ocl_vgentype_vmisc,
     ocl_gentype_uinteger_vmisc
  ],
  [
     isSameAs<Id<1>, Id<2>>,
     isSameBaseAs<Id<0>, Id<1>>, 
     isSameDimAs<Id<0>, Id<3>>,
     isSameBaseSizeAs<Id<0>, Id<3>>
  ]
>;

}

//===----------------------------------------------------------------------===//
// Misc. Vector builtin declarations
//===----------------------------------------------------------------------===//

// vec_step is supported since Clang 3.0
// def blt_vec_step    : VmiscBuiltin_Cng_CnVg<"vec_step">;

def blt_shuffle     : VmiscBuiltin_ggUi<"shuffle">;
def blt_shuffle2    : VmiscBuiltin_gggUi<"shuffle2">;

//===----------------------------------------------------------------------===//
// Misc. Vector builtin implementations
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_shuffle, TemplateStrategy<[{
  GENTYPE_N RetValue;
  uint bit_mask;

  switch (vec_step(param1)) {
    case 2:
      bit_mask = 0x1U;
      break;
    case 4:
      bit_mask = 0x3U;
      break;
    case 8:
      bit_mask = 0x7U;
      break;
    case 16:
      bit_mask = 0xFU;
      break;
  }

  for (uint i = 0; i < vec_step(param2); i++)
    RetValue[i] = param1[param2[i] & bit_mask];

  return RetValue;
}], [TypedefId<"GENTYPE_N", Id<0>>]>>;

def : OCLBuiltinImpl<blt_shuffle2, TemplateStrategy<[{
  GENTYPE_N RetValue;
  uint bit_mask, bit_mask_hi;

  switch (vec_step(param1)) {
    case 2:
      bit_mask = 0x1U;
      break;
    case 4:
      bit_mask = 0x3U;
      break;
    case 8:
      bit_mask = 0x7U;
      break;
    case 16:
      bit_mask = 0xFU;
      break;
  }

  bit_mask_hi = bit_mask + 0x1U;

  for (uint i = 0; i < vec_step(param3); i++)
    RetValue[i] = (param3[i] & bit_mask_hi) ? param2[param3[i] & bit_mask]
                                            : param1[param3[i] & bit_mask];

  return RetValue;
}], [TypedefId<"GENTYPE_N", Id<0>>]>>;
