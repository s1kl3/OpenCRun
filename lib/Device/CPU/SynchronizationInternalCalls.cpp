
#include "opencrun/Device/CPU/InternalCalls.h"
#include "opencrun/Device/CPU/Multiprocessor.h"

#include "llvm/Support/Atomic.h"

using namespace opencrun;
using namespace opencrun::cpu;

void opencrun::cpu::Barrier(cl_mem_fence_flags Flag) {
  if(Flag & CLK_GLOBAL_MEM_FENCE)
    llvm::sys::MemoryFence();

  CPUThread &CurThread = GetCurrentThread();

  CurThread.SwitchToNextWorkItem();
}
