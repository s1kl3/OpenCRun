
#ifndef OPENCRUN_DEVICE_CPU_INTERNALCALLS_H
#define OPENCRUN_DEVICE_CPU_INTERNALCALLS_H

#include "opencrun/Device/CPU/AsyncCopyThread.h"

#include "llvm/ADT/SmallVector.h"

// With the CPU device the accellerator and the host are on the same computed
// device, so include CL/opencl.h to get defined the same data-types used by the
// host.

#include "CL/opencl.h"

// However, some types are used only inside the OpenCL C language and they are
// not available on the host. Define them here.
typedef cl_uint cl_mem_fence_flags;

// Event type used to identify asynchronous copies from local to global memory
// and vice-versa.

struct _event_t {
  typedef llvm::SmallVector<opencrun::cpu::AsyncCopyThread *, 10>::iterator iterator;

  llvm::SmallVector<opencrun::cpu::AsyncCopyThread *, 10> async_copies;
};

typedef _event_t * event_t;

// The same holds for some macro.
#define CLK_LOCAL_MEM_FENCE  1 << 0
#define CLK_GLOBAL_MEM_FENCE 1 << 1

#define CLK_NORMALIZED_COORDS_FALSE 0x0000
#define CLK_NORMALIZED_COORDS_TRUE  0x0010

#define CLK_ADDRESS_NONE            0x0000
#define CLK_ADDRESS_MIRRORED_REPEAT 0x0001
#define CLK_ADDRESS_REPEAT          0x0002
#define CLK_ADDRESS_CLAMP_TO_EDGE   0x0003
#define CLK_ADDRESS_CLAMP           0x0004

#define CLK_FILTER_NEAREST          0x0000
#define CLK_FILTER_LINEAR           0x0100

#define CLK_R                       0x10B0
#define CLK_A                       0x10B1
#define CLK_RG                      0x10B2
#define CLK_RA                      0x10B3
#define CLK_RGB                     0x10B4
#define CLK_RGBA                    0x10B5
#define CLK_BGRA                    0x10B6
#define CLK_ARGB                    0x10B7
#define CLK_INTENSITY               0x10B8
#define CLK_LUMINANCE               0x10B9
#define CLK_Rx                      0x10BA
#define CLK_RGx                     0x10BB
#define CLK_RGBx                    0x10BC
#define CLK_DEPTH                   0x10BD
#define CLK_DEPTH_STENCIL           0x10BE

#define CLK_SNORM_INT8              0x10D0
#define CLK_SNORM_INT16             0x10D1
#define CLK_UNORM_INT8              0x10D2
#define CLK_UNORM_INT16             0x10D3
#define CLK_UNORM_SHORT_565         0x10D4
#define CLK_UNORM_SHORT_555         0x10D5
#define CLK_UNORM_INT_101010        0x10D6
#define CLK_SIGNED_INT8             0x10D7
#define CLK_SIGNED_INT16            0x10D8
#define CLK_SIGNED_INT32            0x10D9
#define CLK_UNSIGNED_INT8           0x10DA
#define CLK_UNSIGNED_INT16          0x10DB
#define CLK_UNSIGNED_INT32          0x10DC
#define CLK_HALF_FLOAT              0x10DD
#define CLK_FLOAT                   0x10DE
#define CLK_UNORM_INT24             0x10DF

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

// Asynchronous Copy Functions.
event_t AsyncWorkGroupCopy(unsigned char *dst,
                           const unsigned char *src,
                           size_t num_gentypes,
                           event_t event,
                           size_t sz_gentype);

event_t AsyncWorkGroupStridedCopy(unsigned char *dst,
                                  const unsigned char *src,
                                  size_t num_gentypes,
                                  size_t stride,
                                  event_t event,
                                  size_t sz_gentype,
                                  unsigned int is_src_stride);

void WaitGroupEvents(int num_events, event_t *event_list);

// Explicit Memory Fence Functions.
void MemFence(cl_mem_fence_flags flags);
void ReadMemFence(cl_mem_fence_flags flags);
void WriteMemFence(cl_mem_fence_flags flags);

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_INTERNALCALLS_H
