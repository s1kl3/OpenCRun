#include "opencrun/Device/CPU/InternalCalls.h"
#include "opencrun/Device/CPU/Multiprocessor.h"
#include "opencrun/Device/CPU/AsyncCopyThread.h"
#include "opencrun/Device/CPU/WaitList.h"

// ===================================== Macros =================================================
#define ASYNCCOPY_DEF(gentype) \
  event_t opencrun::cpu::AsyncWorkGroupCopy_ ## gentype(gentype *dst, \
                                                        const gentype *src, \
                                                        size_t num_gentypes, \
                                                        event_t event) { \
    \
    event_t job_evt, out_evt; \
    \
    CPUThread &Thr = GetCurrentThread(); \
    const DimensionInfo::iterator &Cur = Thr.GetCurrentIndex(); \
    \
    bool IsFirst = true; \
    for(unsigned I = 0; I < Cur.GetWorkDim(); ++I) { \
      if(Cur.GetLocalId(I) != 0) { \
        IsFirst = false; \
        break; \
      } \
    } \
    \
    if(IsFirst) { \
      AsyncCopyThread *job = new AsyncCopyThread(); \
      \
      if(event != NULL) { \
        job_evt = GetEvent(event); \
        out_evt = event; \
      } else { \
        job_evt = GetEvent(); \
        out_evt = job_evt; \
      } \
      \
      AsyncCopyThread::AsyncCopyThreadData *data = new AsyncCopyThread::AsyncCopyThreadData(); \
      \
      data->dst = reinterpret_cast<unsigned char *>(dst); \
      data->src = reinterpret_cast<const unsigned char *>(src); \
      data->num_gentypes = num_gentypes; \
      data->sz_gentype = sizeof(gentype); \
      data->dst_stride = 0; \
      data->src_stride = 0; \
      data->event = job_evt; \
      \
      Run(job, data, true); \
    } else { \
      if(event != NULL) \
        out_evt = event; \
      else \
        out_evt = NULL; \
    } \
    \
    return out_evt; \
    \
  }

#define ASYNCSTRIDEDCOPY_DEF(gentype) \
  event_t opencrun::cpu::AsyncWorkGroupStridedCopy_ ## gentype(gentype *dst, \
                                                               const gentype *src, \
                                                               size_t num_gentypes, \
                                                               size_t stride, \
                                                               event_t event) { \
    \
    event_t job_evt, out_evt; \
    \
    CPUThread &Thr = GetCurrentThread(); \
    const DimensionInfo::iterator &Cur = Thr.GetCurrentIndex(); \
    \
    bool IsFirst = true; \
    for(unsigned I = 0; I < Cur.GetWorkDim(); ++I) { \
      if(Cur.GetLocalId(I) != 0) { \
        IsFirst = false; \
        break; \
      } \
    } \
    \
    if(IsFirst) { \
      AsyncCopyThread *job = new AsyncCopyThread(); \
      \
      if(event != NULL) { \
        job_evt = GetEvent(event); \
        out_evt = event; \
      } else { \
        job_evt = GetEvent(); \
        out_evt = job_evt; \
      } \
      \
      AsyncCopyThread::AsyncCopyThreadData *data = new AsyncCopyThread::AsyncCopyThreadData(); \
      \
      data->dst = reinterpret_cast<unsigned char *>(dst); \
      data->src = reinterpret_cast<const unsigned char *>(src); \
      data->num_gentypes = num_gentypes; \
      data->sz_gentype = sizeof(gentype); \
      \
      /*if() { */\
      /*  data->dst_stride = 0; */\
      /*  data->src_stride = stride; */\
      /*} else if () { */\
        data->dst_stride = stride; \
        data->src_stride = 0; \
      /*} */\
      \
      data->event = job_evt; \
      \
      Run(job, data, true); \
    } else { \
      if(event != NULL) \
        out_evt = event; \
      else \
        out_evt = NULL; \
    } \
    \
    return out_evt; \
    \
  }

#define PREFETCH_DEF(gentype) \
  void opencrun::cpu::Prefetch_ ## gentype(const gentype *p, size_t num_gentypes) { \
    \
  }

#define BUILD_ASYNCLIB_DEF(gentype) \
  ASYNCCOPY_DEF(gentype) \
  ASYNCSTRIDEDCOPY_DEF(gentype) \
  PREFETCH_DEF(gentype)

// ==============================================================================================

using namespace opencrun;
using namespace opencrun::cpu;

void opencrun::cpu::WaitGroupEvents(int num_events, event_t *event_list) {
  
  if(!num_events || !event_list)
    return;

  for(int I = 0; I < num_events; ++I)
    opencrun::cpu::WaitEventCompletion(event_list[I]);

}

BUILD_ASYNCLIB_DEF(char)
BUILD_ASYNCLIB_DEF(char2)
BUILD_ASYNCLIB_DEF(char3)
BUILD_ASYNCLIB_DEF(char4)
BUILD_ASYNCLIB_DEF(char8)
BUILD_ASYNCLIB_DEF(char16)
BUILD_ASYNCLIB_DEF(uchar)
BUILD_ASYNCLIB_DEF(uchar2)
BUILD_ASYNCLIB_DEF(uchar3)
BUILD_ASYNCLIB_DEF(uchar4)
BUILD_ASYNCLIB_DEF(uchar8)
BUILD_ASYNCLIB_DEF(uchar16)
BUILD_ASYNCLIB_DEF(short)
BUILD_ASYNCLIB_DEF(short2)
BUILD_ASYNCLIB_DEF(short3)
BUILD_ASYNCLIB_DEF(short4)
BUILD_ASYNCLIB_DEF(short8)
BUILD_ASYNCLIB_DEF(short16)
BUILD_ASYNCLIB_DEF(ushort)
BUILD_ASYNCLIB_DEF(ushort2)
BUILD_ASYNCLIB_DEF(ushort3)
BUILD_ASYNCLIB_DEF(ushort4)
BUILD_ASYNCLIB_DEF(ushort8)
BUILD_ASYNCLIB_DEF(ushort16)
BUILD_ASYNCLIB_DEF(int)
BUILD_ASYNCLIB_DEF(int2)
BUILD_ASYNCLIB_DEF(int3)
BUILD_ASYNCLIB_DEF(int4)
BUILD_ASYNCLIB_DEF(int8)
BUILD_ASYNCLIB_DEF(int16)
BUILD_ASYNCLIB_DEF(uint)
BUILD_ASYNCLIB_DEF(uint2)
BUILD_ASYNCLIB_DEF(uint3)
BUILD_ASYNCLIB_DEF(uint4)
BUILD_ASYNCLIB_DEF(uint8)
BUILD_ASYNCLIB_DEF(uint16)
BUILD_ASYNCLIB_DEF(long)
BUILD_ASYNCLIB_DEF(long2)
BUILD_ASYNCLIB_DEF(long3)
BUILD_ASYNCLIB_DEF(long4)
BUILD_ASYNCLIB_DEF(long8)
BUILD_ASYNCLIB_DEF(long16)
BUILD_ASYNCLIB_DEF(ulong)
BUILD_ASYNCLIB_DEF(ulong2)
BUILD_ASYNCLIB_DEF(ulong3)
BUILD_ASYNCLIB_DEF(ulong4)
BUILD_ASYNCLIB_DEF(ulong8)
BUILD_ASYNCLIB_DEF(ulong16)
BUILD_ASYNCLIB_DEF(float)
BUILD_ASYNCLIB_DEF(float2)
BUILD_ASYNCLIB_DEF(float3)
BUILD_ASYNCLIB_DEF(float4)
BUILD_ASYNCLIB_DEF(float8)
BUILD_ASYNCLIB_DEF(float16)
