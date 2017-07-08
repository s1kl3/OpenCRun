//===----------------------------------------------------------------------===//
// Atomic builtin classes
//===----------------------------------------------------------------------===//

let Group = "ATOMIC" in {

class AtomicBuiltin_nP1n_UnP1Un<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_global, [ocl_mod_volatile]>
  ],
  [],
  "nP1n">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_global, [ocl_mod_volatile]>
  ],
  [],
  "UnP1Un">
]>;

class AtomicBuiltin_nP2n_UnP2Un<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_local, [ocl_mod_volatile]>
  ],
  [],
  "nP2n">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_local, [ocl_mod_volatile]>
  ],
  [],
  "UnP2Un">
]>;

class AtomicBuiltin_nP1nn_UnP1UnUn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_global, [ocl_mod_volatile]>,
    ocl_int
  ],
  [],
  "nP1nn">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_global, [ocl_mod_volatile]>,
    ocl_uint
  ],
  [],
  "UnP1UnUn">
]>;

class AtomicBuiltin_nP2nn_UnP2UnUn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_local, [ocl_mod_volatile]>,
    ocl_int
  ],
  [],
  "nP2nn">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_local, [ocl_mod_volatile]>,
    ocl_uint
  ],
  [],
  "UnP2UnUn">
]>;

class AtomicBuiltin_nP1nnn_UnP1UnUnUn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_global, [ocl_mod_volatile]>,
    ocl_int,
    ocl_int
  ],
  [],
  "nP1nnn">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_global, [ocl_mod_volatile]>,
    ocl_uint,
    ocl_uint
  ],
  [],
  "UnP1UnUnUn">
]>;

class AtomicBuiltin_nP2nnn_UnP2UnUnUn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_local, [ocl_mod_volatile]>,
    ocl_int,
    ocl_int
  ],
  [],
  "nP2nnn">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_local, [ocl_mod_volatile]>,
    ocl_uint,
    ocl_uint
  ],
  [],
  "UnP2UnUnUn">
]>;

class AtomicBuiltin_nP1n_UnP1Un_nP2n_UnP2Un<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_global, [ocl_mod_volatile]>
  ],
  [],
  "nP1n">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_global, [ocl_mod_volatile]>
  ],
  [],
  "UnP1Un">,
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_local, [ocl_mod_volatile]>
  ],
  [],
  "nP2n">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_local, [ocl_mod_volatile]>
  ],
  [],
  "UnP2Un">
]>;

class AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_global, [ocl_mod_volatile]>,
    ocl_int
  ],
  [],
  "nP1nn">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_global, [ocl_mod_volatile]>,
    ocl_uint
  ],
  [],
  "UnP1UnUn">,
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_local, [ocl_mod_volatile]>,
    ocl_int
  ],
  [],
  "nP2nn">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_local, [ocl_mod_volatile]>,
    ocl_uint
  ],
  [],
  "UnP2UnUn">
]>;

class AtomicBuiltin_nP1nn_UnP1UnUn_fP1ff_nP2nn_UnP2UnUn_fP2ff<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_global, [ocl_mod_volatile]>,
    ocl_int
  ],
  [],
  "nP1nn">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_global, [ocl_mod_volatile]>,
    ocl_uint
  ],
  [],
  "UnP1UnUn">,
  OCLBuiltinVariant<[
    ocl_float,
    OCLPointerType<ocl_float, ocl_as_global, [ocl_mod_volatile]>,
    ocl_float
  ],
  [],
  "fP1ff">,
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_local, [ocl_mod_volatile]>,
    ocl_int
  ],
  [],
  "nP2nn">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_local, [ocl_mod_volatile]>,
    ocl_uint
  ],
  [],
  "UnP2UnUn">,
  OCLBuiltinVariant<[
    ocl_float,
    OCLPointerType<ocl_float, ocl_as_local, [ocl_mod_volatile]>,
    ocl_float
  ],
  [],
  "fP2ff">
]>;

class AtomicBuiltin_nP1nnn_UnP1UnUnUn_nP2nnn_UnP2UnUnUn<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_global, [ocl_mod_volatile]>,
    ocl_int,
    ocl_int
  ],
  [],
  "nP1nnn">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_global, [ocl_mod_volatile]>,
    ocl_uint,
    ocl_uint
  ],
  [],
  "UnP1UnUnUn">,
  OCLBuiltinVariant<[
    ocl_int,
    OCLPointerType<ocl_int, ocl_as_local, [ocl_mod_volatile]>,
    ocl_int,
    ocl_int
  ],
  [],
  "nP2nnn">,
  OCLBuiltinVariant<[
    ocl_uint,
    OCLPointerType<ocl_uint, ocl_as_local, [ocl_mod_volatile]>,
    ocl_uint,
    ocl_uint
  ],
  [],
  "UnP2UnUnUn">
]>;

class AtomicBuiltin_lP1l_UlP1Ul_lP2l_UlP2Ul<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_long,
    OCLPointerType<ocl_long, ocl_as_global, [ocl_mod_volatile]>
  ],
  [],
  "lP1l">,
  OCLBuiltinVariant<[
    ocl_ulong,
    OCLPointerType<ocl_ulong, ocl_as_global, [ocl_mod_volatile]>
  ],
  [],
  "UlP1Ul">,
  OCLBuiltinVariant<[
    ocl_long,
    OCLPointerType<ocl_long, ocl_as_local, [ocl_mod_volatile]>
  ],
  [],
  "lP2l">,
  OCLBuiltinVariant<[
    ocl_ulong,
    OCLPointerType<ocl_ulong, ocl_as_local, [ocl_mod_volatile]>
  ],
  [],
  "UlP2Ul">
]>;

class AtomicBuiltin_lP1ll_UlP1UlUl_lP2ll_UlP2UlUl<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_long,
    OCLPointerType<ocl_long, ocl_as_global, [ocl_mod_volatile]>,
    ocl_long
  ],
  [],
  "lP1ll">,
  OCLBuiltinVariant<[
    ocl_ulong,
    OCLPointerType<ocl_ulong, ocl_as_global, [ocl_mod_volatile]>,
    ocl_ulong
  ],
  [],
  "UlP1UlUl">,
  OCLBuiltinVariant<[
    ocl_long,
    OCLPointerType<ocl_long, ocl_as_local, [ocl_mod_volatile]>,
    ocl_long
  ],
  [],
  "lP2ll">,
  OCLBuiltinVariant<[
    ocl_ulong,
    OCLPointerType<ocl_ulong, ocl_as_local, [ocl_mod_volatile]>,
    ocl_ulong
  ],
  [],
  "UlP2UlUl">
]>;

class AtomicBuiltin_lP1lll_UlP1UlUlUl_lP2lll_UlP2UlUlUl<string name> : OCLGenericBuiltin<name, [
  OCLBuiltinVariant<[
    ocl_long,
    OCLPointerType<ocl_long, ocl_as_global, [ocl_mod_volatile]>,
    ocl_long,
    ocl_long
  ],
  [],
  "lP1lll">,
  OCLBuiltinVariant<[
    ocl_ulong,
    OCLPointerType<ocl_ulong, ocl_as_global, [ocl_mod_volatile]>,
    ocl_ulong,
    ocl_ulong
  ],
  [],
  "UlP1UlUlUl">,
  OCLBuiltinVariant<[
    ocl_long,
    OCLPointerType<ocl_long, ocl_as_local, [ocl_mod_volatile]>,
    ocl_long,
    ocl_long
  ],
  [],
  "lP2lll">,
  OCLBuiltinVariant<[
    ocl_ulong,
    OCLPointerType<ocl_ulong, ocl_as_local, [ocl_mod_volatile]>,
    ocl_ulong,
    ocl_ulong
  ],
  [],
  "UlP2UlUlUl">
]>;

}

//===----------------------------------------------------------------------===//
// Atomic builtin declarations
//===----------------------------------------------------------------------===//

def blt_atomic_add : AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_add">;
def blt_atomic_sub : AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_sub">;

def blt_atomic_xchg : AtomicBuiltin_nP1nn_UnP1UnUn_fP1ff_nP2nn_UnP2UnUn_fP2ff<"atomic_xchg">;

def blt_atomic_inc : AtomicBuiltin_nP1n_UnP1Un_nP2n_UnP2Un<"atomic_inc">;
def blt_atomic_dec : AtomicBuiltin_nP1n_UnP1Un_nP2n_UnP2Un<"atomic_dec">;

def blt_atomic_cmpxchg : AtomicBuiltin_nP1nnn_UnP1UnUnUn_nP2nnn_UnP2UnUnUn<"atomic_cmpxchg">;

def blt_atomic_min	: AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_min">;
def blt_atomic_max	: AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_max">;

def blt_atomic_and	: AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_and">;
def blt_atomic_or	  : AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_or">;
def blt_atomic_xor  : AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_xor">;

let Predicates = [ocl_ext_cl_khr_local_int32_base_atomics] in {
  def blt_atom_add_32_local : AtomicBuiltin_nP2nn_UnP2UnUn<"atom_add">;
  def blt_atom_sub_32_local : AtomicBuiltin_nP2nn_UnP2UnUn<"atom_sub">;
  def blt_atom_xchg_32_local : AtomicBuiltin_nP2nn_UnP2UnUn<"atom_xchg">;
  def blt_atom_inc_32_local : AtomicBuiltin_nP2n_UnP2Un<"atom_inc">;
  def blt_atom_dec_32_local : AtomicBuiltin_nP2n_UnP2Un<"atom_dec">;
  def blt_atom_cmpxchg_32_local : AtomicBuiltin_nP2nnn_UnP2UnUnUn<"atom_cmpxchg">;
}

let Predicates = [ocl_ext_cl_khr_global_int32_base_atomics] in {
  def blt_atom_add_32_global : AtomicBuiltin_nP1nn_UnP1UnUn<"atom_add">;
  def blt_atom_sub_32_global : AtomicBuiltin_nP1nn_UnP1UnUn<"atom_sub">;
  def blt_atom_xchg_32_global : AtomicBuiltin_nP1nn_UnP1UnUn<"atom_xchg">;
  def blt_atom_inc_32_global : AtomicBuiltin_nP1n_UnP1Un<"atom_inc">;
  def blt_atom_dec_32_global : AtomicBuiltin_nP1n_UnP1Un<"atom_dec">;
  def blt_atom_cmpxchg_32_global : AtomicBuiltin_nP1nnn_UnP1UnUnUn<"atom_cmpxchg">;
}

let Predicates = [ocl_ext_cl_khr_local_int32_extended_atomics] in {
  def blt_atom_min_32_local : AtomicBuiltin_nP2nn_UnP2UnUn<"atom_min">;
  def blt_atom_max_32_local : AtomicBuiltin_nP2nn_UnP2UnUn<"atom_max">;
  def blt_atom_and_32_local : AtomicBuiltin_nP2nn_UnP2UnUn<"atom_and">;
  def blt_atom_or_32_local : AtomicBuiltin_nP2nn_UnP2UnUn<"atom_or">;
  def blt_atom_xor_32_local : AtomicBuiltin_nP2nn_UnP2UnUn<"atom_xor">;
}

let Predicates = [ocl_ext_cl_khr_global_int32_extended_atomics] in {
  def blt_atom_min_32_global : AtomicBuiltin_nP1nn_UnP1UnUn<"atom_min">;
  def blt_atom_max_32_global : AtomicBuiltin_nP1nn_UnP1UnUn<"atom_max">;
  def blt_atom_and_32_global : AtomicBuiltin_nP1nn_UnP1UnUn<"atom_and">;
  def blt_atom_or_32_global : AtomicBuiltin_nP1nn_UnP1UnUn<"atom_or">;
  def blt_atom_xor_32_global : AtomicBuiltin_nP1nn_UnP1UnUn<"atom_xor">;
}

let Predicates = [ocl_ext_cl_khr_int64_base_atomics] in {
  def blt_atom_add_64 : AtomicBuiltin_lP1ll_UlP1UlUl_lP2ll_UlP2UlUl<"atom_add">;
  def blt_atom_sub_64 : AtomicBuiltin_lP1ll_UlP1UlUl_lP2ll_UlP2UlUl<"atom_sub">;
  def blt_atom_xchg_64 : AtomicBuiltin_lP1ll_UlP1UlUl_lP2ll_UlP2UlUl<"atom_xchg">;
  def blt_atom_inc_64 : AtomicBuiltin_lP1l_UlP1Ul_lP2l_UlP2Ul<"atom_inc">;
  def blt_atom_dec_64 : AtomicBuiltin_lP1l_UlP1Ul_lP2l_UlP2Ul<"atom_dec">;
  def blt_atom_cmpxchg_64 : AtomicBuiltin_lP1lll_UlP1UlUlUl_lP2lll_UlP2UlUlUl<"atom_cmpxchg">;
}

let Predicates = [ocl_ext_cl_khr_int64_extended_atomics] in {
  def blt_atom_min_64 : AtomicBuiltin_lP1ll_UlP1UlUl_lP2ll_UlP2UlUl<"atom_min">;
  def blt_atom_max_64 : AtomicBuiltin_lP1ll_UlP1UlUl_lP2ll_UlP2UlUl<"atom_max">;
  def blt_atom_xor_64 : AtomicBuiltin_lP1ll_UlP1UlUl_lP2ll_UlP2UlUl<"atom_xor">;
  def blt_atom_and_64 : AtomicBuiltin_lP1ll_UlP1UlUl_lP2ll_UlP2UlUl<"atom_and">;
  def blt_atom_or_64 : AtomicBuiltin_lP1ll_UlP1UlUl_lP2ll_UlP2UlUl<"atom_or">;
}

//===----------------------------------------------------------------------===//
// Atomic builtin implementations
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_atomic_add, TemplateStrategy<[{
  return __sync_fetch_and_add_4(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_sub, TemplateStrategy<[{
  return __sync_fetch_and_sub_4(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set_4(param1, param2);
}]>, "nP1nn">;

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set_4(param1, param2);
}]>, "UnP1UnUn">;

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set_4(
        (volatile uint __global*)param1, 
        (uint) param2
        );
}]>, "fP1ff">;

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set_4(param1, param2);
}]>, "nP2nn">;

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set_4(param1, param2);
}]>, "UnP2UnUn">;

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set_4(
        (volatile uint __local *) param1, 
        (uint) param2
        );
}]>, "fP2ff">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_inc, TemplateStrategy<[{
  return __builtin_ocl_atomic_add(param1, 1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_dec, TemplateStrategy<[{
  return __builtin_ocl_atomic_sub(param1, 1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_cmpxchg, TemplateStrategy<[{
  return __sync_val_compare_and_swap_4(param1, param2, param3);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_and, TemplateStrategy<[{
  return __sync_fetch_and_and_4(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_or, TemplateStrategy<[{
  return __sync_fetch_and_or_4(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_xor, TemplateStrategy<[{
  return __sync_fetch_and_xor_4(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_add_32_local, TemplateStrategy<[{
  return __sync_fetch_and_add_4(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_sub_32_local, TemplateStrategy<[{
  return __sync_fetch_and_sub_4(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_xchg_32_local, TemplateStrategy<[{
  return __sync_lock_test_and_set_4(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_inc_32_local, TemplateStrategy<[{
  return __builtin_ocl_atom_add(param1, 1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_dec_32_local, TemplateStrategy<[{
  return __builtin_ocl_atom_sub(param1, 1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_cmpxchg_32_local, TemplateStrategy<[{
  return __sync_val_compare_and_swap_4(param1, param2, param3);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_add_32_global, TemplateStrategy<[{
  return __sync_fetch_and_add_4(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_sub_32_global, TemplateStrategy<[{
  return __sync_fetch_and_sub_4(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_xchg_32_global, TemplateStrategy<[{
  return __sync_lock_test_and_set_4(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_inc_32_global, TemplateStrategy<[{
  return __builtin_ocl_atom_add(param1, 1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_dec_32_global, TemplateStrategy<[{
  return __builtin_ocl_atom_sub(param1, 1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_cmpxchg_32_global, TemplateStrategy<[{
  return __sync_val_compare_and_swap_4(param1, param2, param3);
}]>>;

//----------------------------------------------------------------------------//
def : OCLBuiltinImpl<blt_atom_add_64, TemplateStrategy<[{
  return __sync_fetch_and_add_8(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_sub_64, TemplateStrategy<[{
  return __sync_fetch_and_sub_8(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_xchg_64, TemplateStrategy<[{
  return __sync_lock_test_and_set_8(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_inc_64, TemplateStrategy<[{
  return __builtin_ocl_atom_add(param1, 1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_dec_64, TemplateStrategy<[{
  return __builtin_ocl_atom_sub(param1, 1);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_cmpxchg_64, TemplateStrategy<[{
  return __sync_val_compare_and_swap_8(param1, param2, param3);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_xor_64, TemplateStrategy<[{
  return __sync_fetch_and_xor_8(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_and_64, TemplateStrategy<[{
  return __sync_fetch_and_and_8(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_or_64, TemplateStrategy<[{
  return __sync_fetch_and_or_8(param1, param2);
}]>>;
