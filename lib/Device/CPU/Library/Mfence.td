def memfence_internal : OCLCodeBlock<[{
void __internal_mem_fence(cl_mem_fence_flags flags);
void __internal_read_mem_fence(cl_mem_fence_flags flags);
void __internal_write_mem_fence(cl_mem_fence_flags flags);
}]>;

let Requirements = [memfence_internal] in {

def : OCLTargetBuiltinImpl<blt_mem_fence, TemplateStrategy<[{
  __internal_mem_fence(param1);
}]>>;

def : OCLTargetBuiltinImpl<blt_read_mem_fence, TemplateStrategy<[{
  __internal_read_mem_fence(param1);
}]>>;

def : OCLTargetBuiltinImpl<blt_write_mem_fence, TemplateStrategy<[{
  __internal_write_mem_fence(param1);
}]>>;

}
