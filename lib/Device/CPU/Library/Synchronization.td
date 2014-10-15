def synchronization_internal : OCLCodeBlock<[{
void __internal_barrier(cl_mem_fence_flags);
}]>;

let Requirements = [synchronization_internal] in
def : OCLTargetBuiltinImpl<blt_barrier, TemplateStrategy<[{
  __internal_barrier(param1);
}]>>;
