include "opencrun/Device/Builtins/Type.td"
include "opencrun/Device/Builtins/Builtin.td"

//===----------------------------------------------------------------------===//
// OpenCL builtins classes
//===----------------------------------------------------------------------===//
// Class names for builtin functions have the following suffixes; for each 
// return/argument type one of the following letters is specified:
//
//      g       integer + real types
//      i       integer types
//      c       char types
//      s       short types
//      n       int types
//      l       long types
//      r       real (float + double) types
//      f       float types
//      d       double types
//      z       size_t
//      v       void
//
// Each one may be preceded by the following modifiers:
//
//      U       unsigned only OCLGroupType
//      S       signed only OCLGroupType
//      V       vectorial or vectorial only OCLGroupType
//      C       scalar or scalar only OCLGroupType
//      P       pointer
//
//===----------------------------------------------------------------------===//

include "opencrun/Device/Builtins/Common.td"
include "opencrun/Device/Builtins/Geometric.td"
include "opencrun/Device/Builtins/Integer.td"
include "opencrun/Device/Builtins/Math.td"
include "opencrun/Device/Builtins/Relational.td"
include "opencrun/Device/Builtins/Vloadstore.td"
include "opencrun/Device/Builtins/Vmisc.td"
