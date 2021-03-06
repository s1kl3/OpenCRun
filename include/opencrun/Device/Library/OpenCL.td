include "opencrun/Device/Library/Target.td"
include "opencrun/Device/Library/Constants.td"
include "opencrun/Device/Library/Type.td"
include "opencrun/Device/Library/Builtin.td"

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
//      e       event_t
//      M       sampler_t
//      I       image types
//
// Occasionally preceded by the following modifiers:
//
//      U       unsigned only OCLGroupType
//      S       signed only OCLGroupType
//      V       vector or vectorial only OCLGroupType
//      C       scalar or scalar only OCLGroupType
//      Px      pointer (x = 1|2|3|4 => AS = global|local|constant|private)
//      Dx      image type (x = 1 | 2 | 3 | 12 | 23 | a | b 
//                          => 1D | 2D | 1&2D | 2&3D | array | buffer)
//      Qx      access-qualified type (x = r | w)
//
//===----------------------------------------------------------------------===//

include "opencrun/Device/Library/Async.td"
include "opencrun/Device/Library/Atomic.td"
include "opencrun/Device/Library/Common.td"
include "opencrun/Device/Library/Convert.td"
include "opencrun/Device/Library/Geometric.td"
include "opencrun/Device/Library/Image.td"
include "opencrun/Device/Library/Integer.td"
include "opencrun/Device/Library/Math.td"
include "opencrun/Device/Library/Mfence.td"
include "opencrun/Device/Library/Reinterpret.td"
include "opencrun/Device/Library/Relational.td"
include "opencrun/Device/Library/Synchronization.td"
include "opencrun/Device/Library/Vloadstore.td"
include "opencrun/Device/Library/Vmisc.td"
include "opencrun/Device/Library/WorkItem.td"
