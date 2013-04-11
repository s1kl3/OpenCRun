
#ifndef OPENCRUN_DEVICE_CPU_OCLTYPES_H
#define OPENCRUN_DEVICE_CPU_OCLTYPES_H

// With the CPU device the accellerator and the host are on the same computed
// device, so include CL/opencl.h to get defined the same data-types used by the
// host.

#include "CL/opencl.h"

// However, some types are used only inside the OpenCL C language and they are
// not available on the host. Define them here.
typedef cl_ulong cl_mem_fence_flags;

// Event type used to identify asynchronous copies from local to global memory
// and vice-versa.
typedef struct _event_t { } * event_t;

// OpenCL C scalar builtin data types.
typedef cl_uchar    uchar;
typedef cl_ushort   ushort;
typedef cl_uint     uint;
typedef cl_ulong    ulong;

typedef cl_half     half;

// OpenCL C vector builtin data types.
typedef cl_char2        char2;
typedef cl_char3        char3;
typedef cl_char4        char4;
typedef cl_char8        char8;
typedef cl_char16       char16;
typedef cl_uchar2       uchar2;
typedef cl_uchar3       uchar3;
typedef cl_uchar4       uchar4;
typedef cl_uchar8       uchar8;
typedef cl_uchar16      uchar16;
typedef cl_short2       short2;
typedef cl_short3       short3;
typedef cl_short4       short4;
typedef cl_short8       short8;
typedef cl_short16      short16;
typedef cl_ushort2      ushort2;
typedef cl_ushort3      ushort3;
typedef cl_ushort4      ushort4;
typedef cl_ushort8      ushort8;
typedef cl_ushort16     ushort16;
typedef cl_int2         int2;
typedef cl_int3         int3;
typedef cl_int4         int4;
typedef cl_int8         int8;
typedef cl_int16        int16;
typedef cl_uint2        uint2;
typedef cl_uint3        uint3;
typedef cl_uint4        uint4;
typedef cl_uint8        uint8;
typedef cl_uint16       uint16;
typedef cl_long2        long2;
typedef cl_long3        long3;
typedef cl_long4        long4;
typedef cl_long8        long8;
typedef cl_long16       long16;
typedef cl_ulong2       ulong2;
typedef cl_ulong3       ulong3;
typedef cl_ulong4       ulong4;
typedef cl_ulong8       ulong8;
typedef cl_ulong16      ulong16;
typedef cl_float2       float2;
typedef cl_float3       float3;
typedef cl_float4       float4;
typedef cl_float8       float8;
typedef cl_float16      float16;
typedef cl_double2      double2;
typedef cl_double3      double3;
typedef cl_double4      double4;
typedef cl_double8      double8;
typedef cl_double16     double16;


// The same holds for some macro.
#define CLK_LOCAL_MEM_FENCE  1 << 0
#define CLK_GLOBAL_MEM_FENCE 1 << 1

#endif
