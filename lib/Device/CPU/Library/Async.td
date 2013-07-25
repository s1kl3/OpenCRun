//===----------------------------------------------------------------------===//
// Async Copy and Prefetch builtin implementations 
//===----------------------------------------------------------------------===//

def : OCLBuiltinImpl<blt_async_work_group_copy, TemplateStrategy<[{
  return __builtin_ocl_async_work_group_copy_impl(
    (uchar *) param1,
    (const uchar *) param2,
    param3,
    param4,
    sizeof(GENTYPE)
  );
}], [TypedefId<"GENTYPE", Pointee<1>>]>>;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_async_work_group_strided_copy, TemplateStrategy<[{
  return __builtin_ocl_async_work_group_strided_copy_impl(
    (uchar *) param1,
    (const uchar *) param2,
    param3,
    param4,
    param5,
    sizeof(GENTYPE),
    (uint) 1
  );
}], [TypedefId<"GENTYPE", Pointee<1>>]>, "eP2gP1gzze">;

def : OCLBuiltinImpl<blt_async_work_group_strided_copy, TemplateStrategy<[{
   return __builtin_ocl_async_work_group_strided_copy_impl(
    (uchar *) param1,
    (const uchar *) param2,
    param3,
    param4,
    param5,
    sizeof(GENTYPE),
    (uint) 0
  );
}], [TypedefId<"GENTYPE", Pointee<1>>]>, "eP1gP2gzze">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_prefetch, TemplateStrategy<[{
  for (uint i = 0; i < sizeof(GENTYPE) * param2; i++)
    __builtin_prefetch((const uchar *) param1[i]);
}], [TypedefId<"GENTYPE", Pointee<1>>]>>;
