//===----------------------------------------------------------------------===//
// Reinterpret builtin classes
//===----------------------------------------------------------------------===//

let Group = "REINTERPRET" in {

class ReinterpretBuiltin_gg : OCLReinterpretBuiltinSimple<
  [ocl_gentype, ocl_gentype],
  [isSameBitSizeAs<Id<0>, Id<1>>]>;

}

def blt_reinterpret_all_to_all : ReinterpretBuiltin_gg;

def : OCLBuiltinImpl<blt_reinterpret_all_to_all, TemplateStrategy<[{
  union {
    SRCTYPE src;
    DSTTYPE dst;
  } Tmp;
  Tmp.src = param1;
  return Tmp.dst;
}], [TypedefId<"SRCTYPE", Id<1>>, TypedefId<"DSTTYPE", Id<0>>]>>;
