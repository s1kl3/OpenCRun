//===----------------------------------------------------------------------===//
// Atomic max and min builtin implementations
//===----------------------------------------------------------------------===//

def float_ptr : OCLCodeBlock<[{
typedef union {
  __global volatile float *g_ptr;
  __local volatile float *l_ptr;
  volatile float *ptr;
} float_ptr_t;
}]>;

let Requirements = [float_ptr] in {

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  TYPE old;
  float_ptr_t p;
  p.g_ptr = param1;

  __atomic_exchange(p.ptr, &param2, &old, __ATOMIC_RELAXED);

  return old;
}], [TypedefId<"TYPE", Id<0>>]>, "fP1ff">;

def : OCLBuiltinImpl<blt_atomic_xchg, TemplateStrategy<[{
  TYPE old;
  float_ptr_t p;
  p.l_ptr = param1;

  __atomic_exchange(p.ptr, &param2, &old, __ATOMIC_RELAXED);

  return old;
}], [TypedefId<"TYPE", Id<0>>]>, "fP2ff">;

}

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_min, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old > param2 &&
      !(__atomic_compare_exchange_n(param1, &old, param2,
                                    false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_max, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old < param2 && 
      !(__atomic_compare_exchange_n(param1, &old, param2,
                                    false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_min_32_local, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old > param2 &&
      !(__atomic_compare_exchange_n(param1, &old, param2,
                                    false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_max_32_local, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old < param2 &&
      !(__atomic_compare_exchange_n(param1, &old, param2,
                                    false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_min_32_global, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old > param2 &&
      !(__atomic_compare_exchange_n(param1, &old, param2,
                                    false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_max_32_global, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old < param2 &&
      !(__atomic_compare_exchange_n(param1, &old, param2,
                                    false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_min_64, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old > param2 &&
      !(__atomic_compare_exchange_n(param1, &old, param2,
                                    false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_max_64, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old < param2 &&
      !(__atomic_compare_exchange_n(param1, &old, param2,
                                    false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)));

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;


