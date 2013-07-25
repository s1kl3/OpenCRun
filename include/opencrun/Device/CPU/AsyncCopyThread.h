
#ifndef OPENCRUN_DEVICE_CPU_ASYNCCOPYTHREAD_H
#define OPENCRUN_DEVICE_CPU_ASYNCCOPYTHREAD_H

#include "opencrun/Device/CPU/InternalCalls.h"
#include "opencrun/Device/CPU/ThreadPool.h"

namespace opencrun {
namespace cpu {

class AsyncCopyThread : public ThreadPool::Job {
public:
  static bool classof(const ThreadPool::Job *job) {
    return job->GetType() == AsyncWGCopy;
  }

public:
  struct AsyncCopyThreadData {

    unsigned char *dst;         // Destination address.

    const unsigned char *src;   // Source address.

    size_t num_gentypes;        // Number of gentype elements to copy.

    size_t sz_gentype;          // Size of each gentype element.

    size_t dst_stride;          // Destination stride (expressed in gentype elements).
    size_t src_stride;          // Source stride (expressed in gentype elements).

    event_t event;              // Event to be notified of Job completion.

  };

public:
  AsyncCopyThread() : ThreadPool::Job(AsyncWGCopy) { }

  ~AsyncCopyThread() { delete thrd_data; }

public:
  void Run(void *data);

private:
  AsyncCopyThreadData *thrd_data;
};

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_ASYNCCOPYTHREAD_H
