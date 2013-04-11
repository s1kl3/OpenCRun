
#ifndef OPENCRUN_DEVICE_CPU_INTERNALCALLS_H
#define OPENCRUN_DEVICE_CPU_INTERNALCALLS_H

#include "opencrun/Device/CPU/OCLTypes.h"

namespace opencrun {
namespace cpu {

// Work-Item Functions.
cl_uint GetWorkDim();
size_t GetGlobalSize(cl_uint I);
size_t GetGlobalId(cl_uint I);
size_t GetLocalSize(cl_uint I);
size_t GetLocalId(cl_uint I);
size_t GetNumGroups(cl_uint I);
size_t GetGroupId(cl_uint I);
size_t GetGlobalOffset(cl_uint I);

// Synchronization Functions.
void Barrier(cl_mem_fence_flags Flags);

// Asynchronous Copy Functions and Prefetch.
#define ASYNCCOPY_PROTO(gentype) \
  event_t AsyncWorkGroupCopy_ ## gentype(gentype *dst, \
                                         const gentype *src, \
                                         size_t num_gentypes, \
                                         event_t event);

#define ASYNCSTRIDEDCOPY_PROTO(gentype) \
  event_t AsyncWorkGroupStridedCopy_ ## gentype (gentype *dst, \
                                                 const gentype *src, \
                                                 size_t num_gentypes, \
                                                 size_t stride, \
                                                 event_t event);

#define PREFETCH_PROTO(gentype) \
  void Prefetch_ ## gentype(const gentype *p, size_t num_gentypes);

#define BUILD_ASYNC_LIB_PROTO(gentype) \
  ASYNCCOPY_PROTO(gentype) \
  ASYNCSTRIDEDCOPY_PROTO(gentype) \
  PREFETCH_PROTO(gentype)

void WaitGroupEvents(int num_events, event_t *event_list);

BUILD_ASYNC_LIB_PROTO(char)
BUILD_ASYNC_LIB_PROTO(char2)
BUILD_ASYNC_LIB_PROTO(char3)
BUILD_ASYNC_LIB_PROTO(char4)
BUILD_ASYNC_LIB_PROTO(char8)
BUILD_ASYNC_LIB_PROTO(char16)
BUILD_ASYNC_LIB_PROTO(uchar)
BUILD_ASYNC_LIB_PROTO(uchar2)
BUILD_ASYNC_LIB_PROTO(uchar3)
BUILD_ASYNC_LIB_PROTO(uchar4)
BUILD_ASYNC_LIB_PROTO(uchar8)
BUILD_ASYNC_LIB_PROTO(uchar16)
BUILD_ASYNC_LIB_PROTO(short)
BUILD_ASYNC_LIB_PROTO(short2)
BUILD_ASYNC_LIB_PROTO(short3)
BUILD_ASYNC_LIB_PROTO(short4)
BUILD_ASYNC_LIB_PROTO(short8)
BUILD_ASYNC_LIB_PROTO(short16)
BUILD_ASYNC_LIB_PROTO(ushort)
BUILD_ASYNC_LIB_PROTO(ushort2)
BUILD_ASYNC_LIB_PROTO(ushort3)
BUILD_ASYNC_LIB_PROTO(ushort4)
BUILD_ASYNC_LIB_PROTO(ushort8)
BUILD_ASYNC_LIB_PROTO(ushort16)
BUILD_ASYNC_LIB_PROTO(int)
BUILD_ASYNC_LIB_PROTO(int2)
BUILD_ASYNC_LIB_PROTO(int3)
BUILD_ASYNC_LIB_PROTO(int4)
BUILD_ASYNC_LIB_PROTO(int8)
BUILD_ASYNC_LIB_PROTO(int16)
BUILD_ASYNC_LIB_PROTO(uint)
BUILD_ASYNC_LIB_PROTO(uint2)
BUILD_ASYNC_LIB_PROTO(uint3)
BUILD_ASYNC_LIB_PROTO(uint4)
BUILD_ASYNC_LIB_PROTO(uint8)
BUILD_ASYNC_LIB_PROTO(uint16)
BUILD_ASYNC_LIB_PROTO(long)
BUILD_ASYNC_LIB_PROTO(long2)
BUILD_ASYNC_LIB_PROTO(long3)
BUILD_ASYNC_LIB_PROTO(long4)
BUILD_ASYNC_LIB_PROTO(long8)
BUILD_ASYNC_LIB_PROTO(long16)
BUILD_ASYNC_LIB_PROTO(ulong)
BUILD_ASYNC_LIB_PROTO(ulong2)
BUILD_ASYNC_LIB_PROTO(ulong3)
BUILD_ASYNC_LIB_PROTO(ulong4)
BUILD_ASYNC_LIB_PROTO(ulong8)
BUILD_ASYNC_LIB_PROTO(ulong16)
BUILD_ASYNC_LIB_PROTO(float)
BUILD_ASYNC_LIB_PROTO(float2)
BUILD_ASYNC_LIB_PROTO(float3)
BUILD_ASYNC_LIB_PROTO(float4)
BUILD_ASYNC_LIB_PROTO(float8)
BUILD_ASYNC_LIB_PROTO(float16)

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_INTERNALCALLS_H
