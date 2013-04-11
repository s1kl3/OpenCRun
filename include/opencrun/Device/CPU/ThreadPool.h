#ifndef OPENCRUN_DEVICE_CPU_THREADPOOL_H
#define OPENCRUN_DEVICE_CPU_THREADPOOL_H

#include "opencrun/System/Monitor.h"
#include "opencrun/System/Thread.h"
#include "opencrun/System/Hardware.h"

#include <list>

namespace opencrun {
namespace cpu {

class WorkerThread;

class ThreadPool {
public:
    // Job class represents a job that can be handled by the pool.
    class Job {
    public:
      enum Type {
        AsyncWGCopy  
      };

    public:
      Job(Type job_ty) :    job_ty(job_ty),
                            pool_thrd(NULL) { }

      virtual ~Job() { }

    public:
      // Code to be executed by the thread goes here.
      virtual void Run(void *data) = 0;

    public:
      Type GetType() const { return job_ty; }
      sys::Monitor &GetSyncMonitor() { return sync_mntr; }
      WorkerThread *GetWorkerThread() { return pool_thrd; }

      void SetWorkerThread(WorkerThread *pool_thrd) { this->pool_thrd = pool_thrd; }

    private:  
      // Job type.
      Type job_ty;
      
      // Associated thread in thread pool.
      WorkerThread *pool_thrd;

      // Monitor for synchronization.
      sys::Monitor sync_mntr;
    };


public:
  // Construct a thread pool containing a thread for each
  // available core.
  ThreadPool();

  // Construct a thread pool containing num_thrds threads.
  ThreadPool(const unsigned int num_thrds);

  // Wait for all thread to finish and destroy thread pool.
  ~ThreadPool();

public:
  // Get the number of internal threads.
  unsigned int GetNumThreads() const { return num_thrds; }

  // Enqueue a job in thread pool.
  void Run(Job *job, void *ptr = NULL, const bool del = false);

  // To synchronize with the termination of a job.
  void Sync(Job *job);

  // To synchronize with all running threads.
  void SyncAll();

public:
  // Return an idle thread from pool
  WorkerThread *GetIdle();

  // Insert an idle thread into pool.
  void AppendIdle(WorkerThread *thrd);

private:
  // Number of threads in the pool.
  unsigned int num_thrds;

  // Array of threads.
  WorkerThread **thrds;

  // List of idle threads.
  std::list<WorkerThread *> idle_thrds;

  // Monitor for synchronization of idle list.
  sys::Monitor idle_mntr;

  friend class WorkerThread;
};

// Functions to access the global thread pool.
ThreadPool &GetThreadPool();
void Run(ThreadPool::Job *job, void *ptr = NULL, const bool del = false);
void Sync(ThreadPool::Job *job);
void SyncAll();

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_THREADPOOL_H
