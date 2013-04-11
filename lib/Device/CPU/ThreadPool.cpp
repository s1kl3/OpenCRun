
#include "opencrun/Device/CPU/ThreadPool.h"

#include "llvm/Support/ManagedStatic.h"

namespace opencrun {
namespace cpu {

// WorkerThread class represents a single thread inside the
// pool.
class WorkerThread : public sys::Thread {
public:
  WorkerThread(ThreadPool *pool) : sys::Thread(),
                                 pool(pool),
                                 job(NULL),
                                 data(NULL),
                                 del_job(false),
                                 end(false) { }

  WorkerThread(ThreadPool *pool, 
             const sys::HardwareCPU &CPU) : sys::Thread(CPU),
                                            pool(pool),
                                            job(NULL),
                                            data(NULL),
                                            del_job(false),
                                            end(false) { }

  ~WorkerThread() { { sys::ScopedMonitor del_lock(del_mntr); } }

public:
  void Run() {

    sys::ScopedMonitor del_lock(del_mntr);

    while(!end) {

      // Append thread to idle list.
      pool->AppendIdle(this);
      
      {
        sys::ScopedMonitor work_lock(work_mntr);

        while((job == NULL) && !end)
          work_lock.Wait();
      }

      // Check for a job
      if(job != NULL) {

        {
          sys::ScopedMonitor job_lock(job->GetSyncMonitor());

          // Execute the job.
          job->Run(data);
          job->SetWorkerThread(NULL);
        }

        // Delete it if necessary.
        if(del_job) 
          delete job;
        
        {
          sys::ScopedMonitor work_lock(work_mntr);

          job = NULL;
          data = NULL;
        }
        
      } // if
     
    } // while

  }

  void Quit() {

    sys::ScopedMonitor work_lock(work_mntr);

    SetEnd(true);
    SetJob(NULL, NULL);

    work_lock.Signal();

  }

public:
  ThreadPool *GetPool() { return pool; }
  ThreadPool::Job *GetJob() { return job; }
  void *GetData() { return data; }

  sys::Monitor &GetWorkMonitor() { return work_mntr; }
  sys::Monitor &GetDelMonitor() { return del_mntr; }

  void SetJob(ThreadPool::Job *job, void *data, const bool del_job = false) {

    sys::ScopedMonitor work_lock(work_mntr);

    this->job = job;
    this->data = data;
    this->del_job = del_job;

    work_lock.Signal();

  }

  void SetEnd(bool end) { this->end = end; }

private:
  // The containing pool.
  ThreadPool    *pool;

  // Job to run and attached data.
  ThreadPool::Job   *job;
  void              *data;

  // Delete upon completion.
  bool          del_job;

  // End of thread.
  bool          end;

  // Monitor for waiting for job.
  sys::Monitor      work_mntr; 
  
  // Monitor for synchronization with destructor.
  sys::Monitor      del_mntr;

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

  // Fetch a reference to statically allocated Hardware class instance.
  opencrun::sys::Hardware &HW = opencrun::sys::GetHardware();

  unsigned int num_cpus = 0;
  for(opencrun::sys::Hardware::cpu_iterator I = HW.cpu_begin(),
                                            E = HW.cpu_end();
                                            I != E;
                                            ++I, ++num_cpus);

  num_thrds = num_cpus;
  thrds = new WorkerThread*[num_thrds];

  unsigned int K = 0;
  for(opencrun::sys::Hardware::cpu_iterator I = HW.cpu_begin(),
                                            E = HW.cpu_end();
                                            I != E;
                                            ++I, ++K) {
    thrds[K] = new WorkerThread(this, *I);
    // AppendIdle(thrds[K]);
    thrds[K]->Start();
  }

}

ThreadPool::ThreadPool(const unsigned int num_thrds) {

  this->num_thrds = num_thrds;
  thrds = new WorkerThread*[num_thrds];

  for(unsigned int I = 0; I < num_thrds; I++) {
    thrds[I] = new WorkerThread(this);
    // AppendIdle(thrds[I]);
    thrds[I]->Start();
  }

}

ThreadPool::~ThreadPool() {

  // Wait for all threads.
  SyncAll();

  // Terminate all threads (exiting their Run method).
  for(unsigned int I = 0; I < num_thrds; I++)
    thrds[I]->Quit();

  // Destroy all WorkerThread objects in pool.
  for(unsigned int I = 0; I < num_thrds; I++) 
    delete thrds[I];

  delete[] thrds;

}

void ThreadPool::Run(Job *job, void *ptr, const bool del) {

  // No job associated with thread.
  if(job == NULL)
    return;

  // Job is assigned to an idle thread as soon as is available and
  // executed asynchronously.
  WorkerThread *thrd = GetIdle();

  // The following lock will be released once WorkerThread has finished
  // its job.
  // job->EnterSyncMonitor();
  job->SetWorkerThread(thrd);

  thrd->SetJob(job, ptr, del);

}

void ThreadPool::Sync(Job *job) {

  if(job == NULL)
    return;

  {
    sys::ScopedMonitor lock(job->GetSyncMonitor());
  }
}

void ThreadPool::SyncAll() {

  while(true) {
    sys::ScopedMonitor lock(idle_mntr);

    if(idle_thrds.size() < num_thrds) 
      lock.Wait();
    else 
      break;
  }

}

WorkerThread *ThreadPool::GetIdle() {

  while(true) {
    // Wait for an idle thread.
    sys::ScopedMonitor lock(idle_mntr);

    while(idle_thrds.empty()) 
      lock.Wait();

    // Fetch the first idle thread.
    if(!idle_thrds.empty()) {
      WorkerThread *thrd = idle_thrds.front();
      idle_thrds.pop_front();
      return thrd;
    }
  }

}

void ThreadPool::AppendIdle(WorkerThread *thr) {

  sys::ScopedMonitor lock(idle_mntr);

  for(std::list<WorkerThread *>::iterator I = idle_thrds.begin(), 
                                        E = idle_thrds.end(); 
                                        I != E; 
                                        ++I) {

    // Check if thr is already present in idle list.
    if((*I) == thr)
      return;

  }
    // Add thr to the end of the idle list.
    idle_thrds.push_back(thr);

    // Signal a thread waiting for job execution.
    idle_mntr.Signal();

}

/*
 * Global thread pool.
 */
llvm::ManagedStatic<ThreadPool> thread_pool;

ThreadPool &opencrun::cpu::GetThreadPool() {
  return *thread_pool;
}

void opencrun::cpu::Run(ThreadPool::Job *job, void *ptr, const bool del) {

  if(job == NULL)
    return;

  thread_pool->Run(job, ptr, del);

}

void opencrun::cpu::Sync(ThreadPool::Job *job) {

  thread_pool->Sync(job);

}

void opencrun::cpu::SyncAll() {

  thread_pool->SyncAll();

}
