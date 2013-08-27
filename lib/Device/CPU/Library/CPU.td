include "opencrun/Device/Library/OpenCL.td"

//===----------------------------------------------------------------------===//
// Target-dependent builtin implementations
//===----------------------------------------------------------------------===//

include "Async.td"
include "Atomic.td"

//===----------------------------------------------------------------------===//

def : OCLTargetFeature<[ocl_ext_cl_khr_fp64]>;
def : OCLTargetFeature<[ocl_ext_cl_khr_int64_base_atomics]>;
def : OCLTargetFeature<[ocl_ext_cl_khr_int64_extended_atomics]>;

def IsX86_32 : OCLMacro<"__i386__">;
def IsX86_64 : OCLMacro<"__x86_64__">;

def : OCLTargetOpaqueTypeDef<ocl_mem_fence_flags, "uint">, Requires<[IsX86_32]>;
def : OCLTargetOpaqueTypeDef<ocl_mem_fence_flags, "ulong">, Requires<[IsX86_64]>;
