def wi_internal : OCLCodeBlock<[{
uint __internal_get_work_dim();
size_t __internal_get_global_size(uint);
size_t __internal_get_global_id(uint);
size_t __internal_get_local_size(uint);
size_t __internal_get_local_id(uint);
size_t __internal_get_num_groups(uint);
size_t __internal_get_group_id(uint);
size_t __internal_get_global_offset(uint);
}]>;

let Requirements = [wi_internal] in {

def : OCLTargetBuiltinImpl<blt_get_work_dim, TemplateStrategy<[{
  return __internal_get_work_dim();
}]>>;

def : OCLTargetBuiltinImpl<blt_get_global_size, TemplateStrategy<[{
  return __internal_get_global_size(param1);
}]>>;

def : OCLTargetBuiltinImpl<blt_get_global_id, TemplateStrategy<[{
  return __internal_get_global_id(param1);
}]>>;

def : OCLTargetBuiltinImpl<blt_get_local_size, TemplateStrategy<[{
  return __internal_get_local_size(param1);
}]>>;

def : OCLTargetBuiltinImpl<blt_get_local_id, TemplateStrategy<[{
  return __internal_get_local_id(param1);
}]>>;

def : OCLTargetBuiltinImpl<blt_get_num_groups, TemplateStrategy<[{
  return __internal_get_num_groups(param1);
}]>>;

def : OCLTargetBuiltinImpl<blt_get_group_id, TemplateStrategy<[{
  return __internal_get_group_id(param1);
}]>>;

def : OCLTargetBuiltinImpl<blt_get_global_offset, TemplateStrategy<[{
  return __internal_get_global_offset(param1);
}]>>;

}
