#include "opencrun/Device/CPU/InternalCalls.h"
#include "opencrun/Device/CPU/Multiprocessor.h"
#include "opencrun/Device/CPU/AsyncCopyThread.h"
#include "opencrun/Device/CPU/WaitList.h"

using namespace opencrun;
using namespace opencrun::cpu;

event_t opencrun::cpu::AsyncWorkGroupCopy(unsigned char *dst,
                                          const unsigned char *src,
                                          size_t num_gentypes,
                                          event_t event,
                                          size_t sz_gentype) {
    event_t job_evt, out_evt;
    
    CPUThread &Thr = GetCurrentThread();
    const DimensionInfo::iterator &Cur = Thr.GetCurrentIndex();
    
    bool IsFirst = true;
    for(unsigned I = 0; I < Cur.GetWorkDim(); ++I) {
      if(Cur.GetLocalId(I) != 0) {
        IsFirst = false;
        break;
      }
    }
    
    if(IsFirst) {
      AsyncCopyThread *job = new AsyncCopyThread();
      
      if(event != NULL) {
        job_evt = GetEvent(event);
        out_evt = event;
      } else {
        job_evt = GetEvent();
        out_evt = job_evt;
      }
      
      AsyncCopyThread::AsyncCopyThreadData *data = new AsyncCopyThread::AsyncCopyThreadData();
      
      data->dst = dst;
      data->src = src;
      data->num_gentypes = num_gentypes;
      data->sz_gentype = sz_gentype;
      data->dst_stride = 0;
      data->src_stride = 0;
      data->event = job_evt;
      
      Run(job, data, true);
    } else {
      if(event != NULL)
        out_evt = event;
      else
        out_evt = NULL;
    }
    
    return out_evt;
}

event_t opencrun::cpu::AsyncWorkGroupStridedCopy(unsigned char *dst,
                                                 const unsigned char *src,
                                                 size_t num_gentypes,
                                                 size_t stride,
                                                 event_t event,
                                                 size_t sz_gentype,
                                                 unsigned int is_src_stride
                                                ) {
    event_t job_evt, out_evt;
    
    CPUThread &Thr = GetCurrentThread();
    const DimensionInfo::iterator &Cur = Thr.GetCurrentIndex();
    
    bool IsFirst = true;
    for(unsigned I = 0; I < Cur.GetWorkDim(); ++I) {
      if(Cur.GetLocalId(I) != 0) {
        IsFirst = false;
        break;
      }
    }
    
    if(IsFirst) {
      AsyncCopyThread *job = new AsyncCopyThread();
      
      if(event != NULL) {
        job_evt = GetEvent(event);
        out_evt = event;
      } else {
        job_evt = GetEvent();
        out_evt = job_evt;
      }
      
      AsyncCopyThread::AsyncCopyThreadData *data = new AsyncCopyThread::AsyncCopyThreadData();
      
      data->dst = dst;
      data->src = src;
      data->num_gentypes = num_gentypes;
      data->sz_gentype = sz_gentype;

      if(is_src_stride == 0) {
        // We have a dst_stride.
        data->src_stride = 0;
        data->dst_stride = stride;
      } else {
        // We have an src_stride.
        data->src_stride = stride;
        data->dst_stride = 0;
      }

      data->event = job_evt;
      
      Run(job, data, true);
    } else {
      if(event != NULL)
        out_evt = event;
      else
        out_evt = NULL;
    }
    
    return out_evt;
}

void opencrun::cpu::WaitGroupEvents(int num_events, event_t *event_list) {
  
  if(!num_events || !event_list)
    return;

  for(int I = 0; I < num_events; ++I)
    opencrun::cpu::WaitEventCompletion(event_list[I]);

}