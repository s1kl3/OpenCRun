
#include "opencrun/Device/CPU/InternalCalls.h"

using namespace opencrun;
using namespace opencrun::cpu;

static volatile unsigned int mutex = 0;

void opencrun::cpu::AcquireGlobalLock() {
  while (true) {
    if (mutex == 0)
      if (!__sync_lock_test_and_set(&mutex, 1))
          return;
  }
}

void opencrun::cpu::ReleaseGlobalLock() {
  __sync_lock_release(&mutex);
}
