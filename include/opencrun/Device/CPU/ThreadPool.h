
#ifndef OPENCRUN_DEVICE_CPU_THREADPOOL_H
#define OPENCRUN_DEVICE_CPU_THREADPOOL_H

#include "opencrun/System/Monitor.h"
#include "opencrun/System/Thread.h"
#include "opencrun/System/Hardware.h"

#include "llvm/ADT/SmallVector.h"

#include <deque>

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
	  Job(Type job_ty) : job_ty(job_ty),
						 completed(false) { }

	  virtual ~Job() { }

	public:
	  virtual void Run() = 0;

	public:
	  Type GetType() const { return job_ty; }
	  sys::Monitor &GetJobMonitor() { return job_mntr; };

	  bool IsCompleted() const { return completed; }

	  void SetCompleted(bool completed) {
		sys::ScopedMonitor lock(job_mntr);

		this->completed = completed;
		lock.Broadcast();
	  };

	private:  
	  // Job type.
	  Type job_ty;

	  bool completed;

	  sys::Monitor job_mntr; 
  };

public:
  // Construct a thread pool containing a thread for each
  // available core.
  ThreadPool();

  ~ThreadPool();

public:
  void EnqueueJob(Job *job);

private:
  // Number of threads in the pool.
  unsigned int num_thrds;

  // Array of threads.
  llvm::SmallVector<WorkerThread *, 12> thrds;

  // Job queue.
  std::deque<Job *> jobs;

  // Monitor for synchronizing access to the job queue.
  sys::Monitor pool_mntr;

  friend class WorkerThread;
};

// Functions to access the global thread pool.
ThreadPool &GetThreadPool();
void EnqueueJobInThreadPool(ThreadPool::Job *job);

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_THREADPOOL_H
