#include "CPUPasses.h"

using namespace opencrun::cpu;

// Never called, however we reference all passes in order to force linking.
extern "C" void LinkInOpenCRunCPUPasses() {
  (void)createAutomaticLocalVariablesPass();
  (void)createGroupParallelStubPass();
}
