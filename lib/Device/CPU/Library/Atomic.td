//===----------------------------------------------------------------------===//
// Atomic max and min builtin implementations
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_atomic_min, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old > param2 && __sync_val_compare_and_swap(param1, old, param2) != old);

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atomic_max, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old < param2 && __sync_val_compare_and_swap(param1, old, param2) != old);

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_min_32_local, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old > param2 && __sync_val_compare_and_swap(param1, old, param2) != old);

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_max_32_local, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old < param2 && __sync_val_compare_and_swap(param1, old, param2) != old);

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_min_32_global, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old > param2 && __sync_val_compare_and_swap(param1, old, param2) != old);

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_max_32_global, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old < param2 && __sync_val_compare_and_swap(param1, old, param2) != old);

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_min_64, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old > param2 && __sync_val_compare_and_swap(param1, old, param2) != old);

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_atom_max_64, TemplateStrategy<[{
  TYPE old;

  do {
    old = *param1;
  } while(old < param2 && __sync_val_compare_and_swap(param1, old, param2) != old);

  return old;
}], [TypedefId<"TYPE", Id<0>>]>>;


