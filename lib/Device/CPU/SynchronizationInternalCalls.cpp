
#include "InternalCalls.h"
#include "Multiprocessor.h"

using namespace opencrun;
using namespace opencrun::cpu;

void opencrun::cpu::Barrier(cl_mem_fence_flags Flag) {
  if(Flag & CLK_GLOBAL_MEM_FENCE)
    __asm__ __volatile__("mfence" : : : "memory");

  CPUThread &CurThread = GetCurrentThread();

  CurThread.SwitchToNextWorkItem();
}
