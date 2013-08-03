
#include "opencrun/Device/CPU/ThreadPool.h"

#include "llvm/Support/ManagedStatic.h"
#include "llvm/ADT/STLExtras.h"

namespace opencrun {
namespace cpu {

// WorkerThread class represents a single thread inside the
// pool.
class WorkerThread : public sys::Thread {
public:
  WorkerThread(ThreadPool *pool, 
             const sys::HardwareCPU &CPU) : sys::Thread(CPU),
                                            pool(pool),
                                            job(NULL),
                                            end(false) { }

  ~WorkerThread() {  }

public:
  void Run() {
	while(true) {
	  pool->pool_mntr.Enter();

	  // Wait for a job in the queue.
	  while(!end && pool->jobs.empty())
		pool->pool_mntr.Wait();

	  if(end) {
		pool->pool_mntr.Exit();
		break;
	  }

	  job = pool->jobs.front();
	  pool->jobs.pop_front();

	  pool->pool_mntr.Exit();

	  // Execute job.
	  job->Run();
	  job->SetCompleted(true);
	  job = NULL;
	}
  }

  void Quit() {
	end = true;
	job = NULL;

	sys::ScopedMonitor pool_lock(pool->pool_mntr);
	pool->pool_mntr.Signal();
  }

private:
  // The containing pool.
  ThreadPool *pool;

  // Job to run.
  ThreadPool::Job *job;

  // End of thread.
  bool end;
  
  friend class ThreadPool;
};


} // End namespace cpu.
} // End namespace opencrun.


using namespace opencrun;
using namespace opencrun::cpu;

/*
 * ThreadPool methods.
 */
ThreadPool::ThreadPool() {
  opencrun::sys::Hardware &HW = opencrun::sys::GetHardware();

  unsigned int K = 0;
  for(opencrun::sys::Hardware::cpu_iterator I = HW.cpu_begin(),
											E = HW.cpu_end();
											I != E;
  											++I, ++K) {
	thrds.push_back(new WorkerThread(this, *I));
	thrds[K]->Start();
  }

  num_thrds = K;
}

ThreadPool::~ThreadPool() {
  // Terminate all threads (exiting their Run method).
  for(unsigned int I = 0; I < num_thrds; I++) {
    thrds[I]->Quit();
	thrds[I]->Join();
  }

  // Wake up worker thread still waiting.
  pool_mntr.Broadcast();

  llvm::DeleteContainerPointers(thrds);
}

void ThreadPool::EnqueueJob(Job *job) {
  if(job == NULL) return;

  sys::ScopedMonitor lock(pool_mntr);

  jobs.push_back(job);

  // Wake up a thread waiting for a job to be available.
  pool_mntr.Signal();
}

/*
 * Global thread pool.
 */
llvm::ManagedStatic<ThreadPool> thread_pool;

ThreadPool &opencrun::cpu::GetThreadPool() {
  return *thread_pool;
}

void opencrun::cpu::EnqueueJobInThreadPool(ThreadPool::Job *job) {
  if(job == NULL)
    return;

  thread_pool->EnqueueJob(job);
}
