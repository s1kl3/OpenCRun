//===----------------------------------------------------------------------===//
// Atomic max and min builtin implementations
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_atomic_min, TemplateStrategy<[{
  __builtin_ocl_acquire_global_lock();

  TYPE old = *param1;

  if (old > param2)
    *param1 = param2;

  __builtin_ocl_release_global_lock();

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_max, TemplateStrategy<[{
  __builtin_ocl_acquire_global_lock();

  TYPE old = *param1;

  if (old < param2)
    *param1 = param2;

  __builtin_ocl_release_global_lock();

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_min_32_local, TemplateStrategy<[{
  __builtin_ocl_acquire_global_lock();

  TYPE old = *param1;

  if (old > param2)
    *param1 = param2;

  __builtin_ocl_release_global_lock();

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_max_32_local, TemplateStrategy<[{
  __builtin_ocl_acquire_global_lock();

  TYPE old = *param1;

  if (old < param2)
    *param1 = param2;

  __builtin_ocl_release_global_lock();

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_min_32_global, TemplateStrategy<[{
  __builtin_ocl_acquire_global_lock();

  TYPE old = *param1;

  if (old > param2)
    *param1 = param2;

  __builtin_ocl_release_global_lock();

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_max_32_global, TemplateStrategy<[{
  __builtin_ocl_acquire_global_lock();

  TYPE old = *param1;

  if (old < param2)
    *param1 = param2;

  __builtin_ocl_release_global_lock();

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_min_64, TemplateStrategy<[{
  __builtin_ocl_acquire_global_lock();

  TYPE old = *param1;

  if (old > param2)
    *param1 = param2;

  __builtin_ocl_release_global_lock();

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_max_64, TemplateStrategy<[{
  __builtin_ocl_acquire_global_lock();

  TYPE old = *param1;

  if (old < param2)
    *param1 = param2;

  __builtin_ocl_release_global_lock();

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;


