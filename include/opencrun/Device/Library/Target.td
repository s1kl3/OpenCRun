class OCLPredicate<string prefix, string name> {
  string Prefix = prefix;
  string Name = name;
}

class OCLMacro<string macro> : OCLPredicate<"", macro>;

class OCLExtension<string name> : OCLPredicate<"__opencrun_target_ext_", name>;

def ocl_ext_cl_khr_fp16 : OCLExtension<"cl_khr_fp16">;
def ocl_ext_cl_khr_fp64 : OCLExtension<"cl_khr_fp64">;

class Requires<list<OCLPredicate> l> {
  list<OCLPredicate> Predicates = l;
}

class OCLTargetFeature<list<OCLPredicate> l> {
  list<OCLPredicate> Features = l;
}