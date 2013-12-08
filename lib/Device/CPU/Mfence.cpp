
#include "opencrun/Device/CPU/InternalCalls.h"

using namespace opencrun;
using namespace opencrun::cpu;

void opencrun::cpu::MemFence(cl_mem_fence_flags flags) {
  __asm__ __volatile__("mfence" : : : "memory");
}

void opencrun::cpu::ReadMemFence(cl_mem_fence_flags flags) {
  __asm__ __volatile__("lfence" : : : "memory");
}

void opencrun::cpu::WriteMemFence(cl_mem_fence_flags flags) {
  __asm__ __volatile__("sfence" : : : "memory");
}
