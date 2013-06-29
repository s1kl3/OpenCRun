//===----------------------------------------------------------------------===//
// Async Copy and Prefetch builtin classes
//===----------------------------------------------------------------------===//

let Group = "ASYNC" in {

class AsyncBuiltin_vP1gz<string name> : OCLSimpleBuiltin<
  name,
  [ocl_void, OCLPointerType<ocl_gentype, ocl_as_global, [ocl_mod_const]>],
  []
>;

class AsyncBuiltin_vCnPe<string name> : OCLSimpleBuiltin<
  name,
  [ocl_void, ocl_int, OCLPointerType<ocl_event_t, ocl_as_private, []>],
  []
>;

class AsyncBuiltin_eP2gP1gze_eP1gP2gze<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_event_t,
    OCLPointerType<ocl_gentype, ocl_as_local, []>,
    OCLPointerType<ocl_gentype, ocl_as_global, [ocl_mod_const]>,
    ocl_size_t,
    ocl_event_t
  ],
  [isSameAs<Pointee<1>, Pointee<2>>],
  "eP2gP1gze">,
  OCLBuiltinVariant<[
    ocl_event_t,
    OCLPointerType<ocl_gentype, ocl_as_global, []>,
    OCLPointerType<ocl_gentype, ocl_as_local, [ocl_mod_const]>,
    ocl_size_t,
    ocl_event_t
  ],
  [isSameAs<Pointee<1>, Pointee<2>>],
  "eP1gP2gze">
]>;

class AsyncBuiltin_eP2gP1gzze_eP1gP2gzze<string name> : OCLMultiBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_event_t,
    OCLPointerType<ocl_gentype, ocl_as_local, []>,
    OCLPointerType<ocl_gentype, ocl_as_global, [ocl_mod_const]>,
    ocl_size_t,
    ocl_size_t,
    ocl_event_t
  ],
  [isSameAs<Pointee<1>, Pointee<2>>],
  "eP2gP1gze">,
  OCLBuiltinVariant<[
    ocl_event_t,
    OCLPointerType<ocl_gentype, ocl_as_global, []>,
    OCLPointerType<ocl_gentype, ocl_as_local, [ocl_mod_const]>,
    ocl_size_t,
    ocl_size_t,
    ocl_event_t
  ],
  [isSameAs<Pointee<1>, Pointee<2>>],
  "eP1gP2gze">
]>;

}

//===----------------------------------------------------------------------===//
// Async Copy and Prefetch builtin declarations
//===----------------------------------------------------------------------===//

def blt_async_work_group_copy           : AsyncBuiltin_eP2gP1gze_eP1gP2gze<"async_work_group_copy">; 
def blt_async_work_group_strided_copy   : AsyncBuiltin_eP2gP1gzze_eP1gP2gzze<"async_work_group_strided_copy">; 
def blt_wait_group_events               : AsyncBuiltin_vCnPe<"wait_group_events">; 
def blt_prefetch                        : AsyncBuiltin_vP1gz<"prefetch">; 
