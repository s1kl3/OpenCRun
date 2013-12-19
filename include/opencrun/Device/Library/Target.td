class OCLPredicate<string prefix, string name> {
  string Prefix = prefix;
  string Name = name;
}

class OCLMacro<string macro> : OCLPredicate<"", macro>;

class OCLExtension<string name> : OCLPredicate<"__opencrun_target_ext_", name>;

def ocl_ext_cl_khr_fp16 : OCLExtension<"cl_khr_fp16">;
def ocl_ext_cl_khr_fp64 : OCLExtension<"cl_khr_fp64">;

def ocl_ext_cl_khr_local_int32_base_atomics : OCLExtension<"cl_khr_local_int32_base_atomics">;
def ocl_ext_cl_khr_global_int32_base_atomics : OCLExtension<"cl_khr_global_int32_base_atomics">;
def ocl_ext_cl_khr_local_int32_extended_atomics : OCLExtension<"cl_khr_local_int32_extended_atomics">;
def ocl_ext_cl_khr_global_int32_extended_atomics : OCLExtension<"cl_khr_global_int32_extended_atomics">;
def ocl_ext_cl_khr_int64_base_atomics : OCLExtension<"cl_khr_int64_base_atomics">;
def ocl_ext_cl_khr_int64_extended_atomics : OCLExtension<"cl_khr_int64_extended_atomics">;

def ocl_ext_cl_khr_3d_image_writes : OCLExtension<"cl_khr_3d_image_writes">;

class Requires<list<OCLPredicate> l> {
  list<OCLPredicate> Predicates = l;
}

class OCLTargetFeature<list<OCLPredicate> l> {
  list<OCLPredicate> Features = l;
  list<OCLPredicate> Predicates = [];
}
