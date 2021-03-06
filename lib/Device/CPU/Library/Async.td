//===----------------------------------------------------------------------===//
// Async Copy and Prefetch builtin implementations 
//===----------------------------------------------------------------------===//

def generic_ptr : OCLCodeBlock<[{
typedef union {
  __global void *g_ptr;
  __local void *l_ptr;
  void *ptr;
} gen_ptr_t;

typedef union {
  const __global void *g_ptr;
  const __local void *l_ptr;
  const void *ptr;
} const_gen_ptr_t;
}]>;

def async_internal : OCLCodeBlock<[{
event_t __internal_async_work_group_copy(uchar *, const uchar*, size_t,
                                         event_t, size_t);
event_t __internal_async_work_group_strided_copy(uchar *, const uchar*, size_t,
                                                 size_t, event_t, size_t, uint);
void __internal_wait_group_events(int, event_t *);
void __internal_prefetch(const uchar*, size_t, size_t);
}]>;

let Requirements = [generic_ptr, async_internal] in {

def : OCLTargetBuiltinImpl<blt_async_work_group_copy, TemplateStrategy<[{
  gen_ptr_t dest_ptr;
  const_gen_ptr_t src_ptr;
  dest_ptr.g_ptr = param1;
  src_ptr.l_ptr = param2;

  return __internal_async_work_group_copy(
    dest_ptr.ptr,
    src_ptr.ptr,
    param3,
    param4,
    sizeof(GENTYPE)
  );
}], [TypedefId<"GENTYPE", Pointee<1>>]>, "eP1gP2gze">;

def : OCLTargetBuiltinImpl<blt_async_work_group_copy, TemplateStrategy<[{
  gen_ptr_t dest_ptr;
  const_gen_ptr_t src_ptr;
  dest_ptr.l_ptr = param1;
  src_ptr.g_ptr = param2;

  return __internal_async_work_group_copy(
    dest_ptr.ptr,
    src_ptr.ptr,
    param3,
    param4,
    sizeof(GENTYPE)
  );
}], [TypedefId<"GENTYPE", Pointee<1>>]>, "eP2gP1gze">;

//----------------------------------------------------------------------------//

def : OCLTargetBuiltinImpl<blt_async_work_group_strided_copy, TemplateStrategy<[{
  gen_ptr_t dest_ptr;
  const_gen_ptr_t src_ptr;
  dest_ptr.l_ptr = param1;
  src_ptr.g_ptr = param2;

  return __internal_async_work_group_strided_copy(
    dest_ptr.ptr,
    src_ptr.ptr,
    param3,
    param4,
    param5,
    sizeof(GENTYPE),
    (uint) 1
  );
}], [TypedefId<"GENTYPE", Pointee<1>>]>, "eP2gP1gzze">;

def : OCLTargetBuiltinImpl<blt_async_work_group_strided_copy, TemplateStrategy<[{
  gen_ptr_t dest_ptr;
  const_gen_ptr_t src_ptr;
  dest_ptr.g_ptr = param1;
  src_ptr.l_ptr = param2;

   return __internal_async_work_group_strided_copy(
    dest_ptr.ptr,
    src_ptr.ptr,
    param3,
    param4,
    param5,
    sizeof(GENTYPE),
    (uint) 0
  );
}], [TypedefId<"GENTYPE", Pointee<1>>]>, "eP1gP2gzze">;

}

//----------------------------------------------------------------------------//

def : OCLTargetBuiltinImpl<blt_wait_group_events, TemplateStrategy<[{
  __internal_wait_group_events(param1, param2);
}]>>;

//----------------------------------------------------------------------------//

let Requirements = [generic_ptr] in
def : OCLTargetBuiltinImpl<blt_prefetch, TemplateStrategy<[{
  const_gen_ptr_t chg_ptr;
  chg_ptr.g_ptr = param1;
  
  __internal_prefetch(chg_ptr.ptr, param2, sizeof(GENTYPE));
}], [TypedefId<"GENTYPE", Pointee<1>>]>>;
