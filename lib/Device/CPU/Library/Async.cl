
//
// The Common library is auto-generated.
//

#define OPENCRUN_BUILTIN_ASYNC

event_t
__builtin_ocl_async_work_group_copy_impl(
    uchar *,
    const uchar*,
    size_t,
    event_t,
    size_t
);

event_t
__builtin_ocl_async_work_group_strided_copy_impl(
    uchar *,
    const uchar*,
    size_t,
    size_t,
    event_t,
    size_t,
    uint
);

#include "Builtins.CPU.inc"
#undef OPENCRUN_BUILTIN_ASYNC 
