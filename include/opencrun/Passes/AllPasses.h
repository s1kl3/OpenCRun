
#ifndef OPENCRUN_DEVICE_PASSES_ALLPASSES_H
#define OPENCRUN_DEVICE_PASSES_ALLPASSES_H

#include "llvm/ADT/StringRef.h"

namespace opencrun {

class FootprintEstimate;
class AggressiveInliner;
class AsyncCopySpecialization;

FootprintEstimate *CreateFootprintEstimatePass(llvm::StringRef Kernel = "");

AggressiveInliner *CreateAggressiveInlinerPass(llvm::StringRef Kernel = "");

AsyncCopySpecialization *CreateAsyncCopySpecializationPass(llvm::StringRef kernel = "");

} // End namespace opencrun.

extern "C" void LinkInOpenCRunPasses();

namespace llvm {

class PassRegistry;

void initializeFootprintEstimatePass(PassRegistry &Registry);

void initializeAggressiveInlinerPass(PassRegistry &Registry);

void initializeAsyncCopySpecializationPass(PassRegistry &Registry);

} // End namespace llvm.

#endif // OPENCRUN_DEVICE_PASSES_ALLPASSES_H
