class OCLPredicate;

class OCLExtension : OCLPredicate;

def ocl_ext_cl_khr_fp16 : OCLExtension;
def ocl_ext_cl_khr_fp64 : OCLExtension;

class Requires<list<OCLPredicate> l> {
  list<OCLPredicate> Predicates = l;
}

class OCLTargetFeature<list<OCLPredicate> l> {
  list<OCLPredicate> Features = l;
}
