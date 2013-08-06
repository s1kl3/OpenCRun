//===----------------------------------------------------------------------===//
// Atomic builtin classes
//===----------------------------------------------------------------------===//

let Group = "ATOMIC" in {

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

}

//===----------------------------------------------------------------------===//
// Atomic builtin declarations
//===----------------------------------------------------------------------===//

def blt_atomic_add : AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_add">;
def blt_atomic_sub : AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_sub">;

def blt_atomic_xchg : AtomicBuiltin_nP1nn_UnP1UnUn_fP1ff_nP2nn_UnP2UnUn_fP2ff<"atomic_xchg">;

def blt_atomic_inc : AtomicBuiltin_nP1n_UnP1Un_nP2n_UnP2Un<"atomic_inc">;
def blt_atomic_dec : AtomicBuiltin_nP1n_UnP1Un_nP2n_UnP2Un<"atomic_dec">;

def blt_atomic_cmpxchg : AtomicBuiltin_nP1nnn_UnP1UnUnUn_nP2nnn_UnP2UnUnUn<"atomic_cmpgxchg">;

def blt_atomic_min	: AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_min">;
def blt_atomic_max	: AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_max">;

def blt_atomic_and	: AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_and">;
def blt_atomic_or	: AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_or">;
def blt_atomic_xor	: AtomicBuiltin_nP1nn_UnP1UnUn_nP2nn_UnP2UnUn<"atomic_xor">;

//===----------------------------------------------------------------------===//
// Atomic builtin implementations
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_atomic_add, TemplateStrategy<[{
  return __sync_fetch_and_add(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_sub, TemplateStrategy<[{
  return __sync_fetch_and_sub(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set(param1, param2);
}]>, "nP1nn">;

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set(param1, param2);
}]>, "UnP1UnUn">;

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set(
				(volatile uint __global*)param1, 
				(uint) param2
			  );
}]>, "fP1ff">;

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set(param1, param2);
}]>, "nP2nn">;

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set(param1, param2);
}]>, "UnP2UnUn">;

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  return __sync_lock_test_and_set(
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
  return __sync_val_compare_and_swap(param1, param2, param3);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_min, TemplateStrategy<[{
  TYPE old = *param1;
  if (old <= param2)
	return old;
  
  __sync_lock_test_and_set(param1, *param1);
  {
	old = *param1;
	if (old > param2)
	  *param1 = param2;
  }
  __sync_lock_release(param1); 
  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_max, TemplateStrategy<[{
  TYPE old = *param1;
  if (old >= param2)
	return old;
  
  __sync_lock_test_and_set(param1, *param1);
  {
	old = *param1;
	if (old < param2)
	  *param1 = param2;
  }
  __sync_lock_release(param1); 
  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_and, TemplateStrategy<[{
  return __sync_fetch_and_and(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_or, TemplateStrategy<[{
  return __sync_fetch_and_or(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_xor, TemplateStrategy<[{
  return __sync_fetch_and_xor(param1, param2);
}]>>;

//----------------------------------------------------------------------------//


