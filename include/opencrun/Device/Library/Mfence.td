//===----------------------------------------------------------------------===//
// Explicit Memory Fence builtin classes
//===----------------------------------------------------------------------===//

let Group = "MFENCE" in {

class MfenceBuiltin_vCUn<string name> : OCLGenericBuiltinSimple<
  name,
  [ocl_void, ocl_mem_fence_flags],
  []
>;

}

//===----------------------------------------------------------------------===//
// Explicit Memory Fence builtin declarations
//===----------------------------------------------------------------------===//

def blt_mem_fence : MfenceBuiltin_vCUn<"mem_fence">; 
def blt_read_mem_fence : MfenceBuiltin_vCUn<"read_mem_fence">; 
def blt_write_mem_fence : MfenceBuiltin_vCUn<"write_mem_fence">; 
