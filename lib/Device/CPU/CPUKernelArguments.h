#ifndef CPUKERNELARGUMENTS_H
#define CPUKERNELARGUMENTS_H

#include "ImageSupport.h"

#include <vector>
#include <memory>

namespace opencrun {
namespace cpu {

class LocalMemory;

class CPUKernelArguments {
public:
  CPUKernelArguments(std::vector<void*> Args, size_t AutoLocalsSize,
                     std::vector<std::pair<size_t, size_t>> LocalBuffers,
                     std::vector<cpu_sampler_t> Samplers)
   : Args(std::move(Args)), AutoLocalsSize(AutoLocalsSize),
     LocalBuffers(std::move(LocalBuffers)),
     Samplers(std::move(Samplers)) {}
  CPUKernelArguments(CPUKernelArguments &&Args) noexcept = default;

  std::unique_ptr<void*[]> prepareArgumentsBuffer(LocalMemory &LM) const;

private:
  std::vector<void*> Args;
  size_t AutoLocalsSize;
  std::vector<std::pair<size_t, size_t>> LocalBuffers;
  std::vector<cpu_sampler_t> Samplers;
};

}
}

#endif
