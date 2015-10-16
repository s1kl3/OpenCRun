
#ifndef OPENCRUN_DEVICE_CPU_INTERNALCALLS_H
#define OPENCRUN_DEVICE_CPU_INTERNALCALLS_H

#include "llvm/ADT/SmallVector.h"

// With the CPU device the accellerator and the host are on the same computed
// device, so include CL/opencl.h to get defined the same data-types used by the
// host.

#include "CL/opencl.h"

// However, some types are used only inside the OpenCL C language and they are
// not available on the host. Define them here.
typedef cl_uint cl_mem_fence_flags;

namespace opencrun {
namespace cpu {

class AsyncCopyThread;

}
}

// Event type used to identify asynchronous copies from local to global memory
// and vice-versa.

struct _event_t {
  typedef llvm::SmallVector<opencrun::cpu::AsyncCopyThread *, 10>::iterator iterator;

  llvm::SmallVector<opencrun::cpu::AsyncCopyThread *, 10> async_copies;
};

typedef _event_t * event_t;

// The same holds for some macro.
#define CLK_LOCAL_MEM_FENCE  0x0
#define CLK_GLOBAL_MEM_FENCE 0x1

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

void Prefetch(const unsigned char *p,
              size_t num_gentypes,
              size_t sz_gentype);

// Explicit Memory Fence Functions.
void MemFence(cl_mem_fence_flags flags);
void ReadMemFence(cl_mem_fence_flags flags);
void WriteMemFence(cl_mem_fence_flags flags);

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_INTERNALCALLS_H
