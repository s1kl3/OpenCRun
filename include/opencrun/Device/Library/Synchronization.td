//===----------------------------------------------------------------------===//
// Synchronization builtin classes
//===----------------------------------------------------------------------===//

let Group = "SYNCHRONIZATION" in {

class SynchronizationBuiltin_vCUn<string name> : OCLGenericBuiltinSimple<
  name,
  [ocl_void, ocl_mem_fence_flags],
  []
>;

}

//===----------------------------------------------------------------------===//
// Synchronization builtin declarations
//===----------------------------------------------------------------------===//

def blt_barrier : SynchronizationBuiltin_vCUn<"barrier">; 
