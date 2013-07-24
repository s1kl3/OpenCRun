//===----------------------------------------------------------------------===//
// Work-Item builtin classes
//===----------------------------------------------------------------------===//

let Group = "WORKITEM" in {

class WorkItemBuiltin_CUn<string name> : OCLSimpleBuiltin<
  name,
  [ocl_uint],
  []
>;

class WorkItemBuiltin_zCUn<string name> : OCLSimpleBuiltin<
  name,
  [ocl_size_t, ocl_uint],
  []
>;

}

//===----------------------------------------------------------------------===//
// Work-Item builtin declarations
//===----------------------------------------------------------------------===//

def blt_get_work_dim        : WorkItemBuiltin_CUn<"get_work_dim">; 

def blt_get_global_size     : WorkItemBuiltin_zCUn<"get_global_size">; 
def blt_get_global_id       : WorkItemBuiltin_zCUn<"get_global_id">; 
def blt_get_local_size      : WorkItemBuiltin_zCUn<"get_local_size">; 
def blt_get_local_id        : WorkItemBuiltin_zCUn<"get_local_id">; 
def blt_get_num_groups      : WorkItemBuiltin_zCUn<"get_num_groups">; 
def blt_get_group_id        : WorkItemBuiltin_zCUn<"get_group_id">; 
def blt_get_global_offset   : WorkItemBuiltin_zCUn<"get_global_offset">; 
