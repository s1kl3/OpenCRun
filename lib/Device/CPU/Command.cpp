
#include "opencrun/Device/CPU/Command.h"
#include "opencrun/Device/CPU/InternalCalls.h"
#include "opencrun/System/OS.h"
#include "CPUKernelInfo.h"

using namespace opencrun;
using namespace opencrun::cpu;

NDRangeKernelBlockCPUCommand::NDRangeKernelBlockCPUCommand(
                                EnqueueNDRangeKernel &Cmd,
                                Signature Entry,
                                ArgsMappings &GlobalArgs,
                                DimensionInfo::iterator I,
                                DimensionInfo::iterator E,
                                size_t StaticLocalSize,
                                IndexOffsetVector &StaticLocalInfos,
                                CPUCommand::ResultRecorder &Result) :
  CPUMultiExecCommand(CPUCommand::NDRangeKernelBlock,
                      Cmd,
                      Result,
                      I.GetWorkGroup()),
  Entry(Entry),
  StaticLocalSize(StaticLocalSize),
  StaticLocalInfos(StaticLocalInfos),
  Start(I),
  End(E) {
  Kernel &Kern = GetKernel();

  unsigned ArgsCount = Kern.GetArgCount() + (StaticLocalSize > 0);

  // Hold arguments to be passed to stubs.
  Args = static_cast<void **>(sys::CAlloc(ArgsCount, sizeof(void *)));

  // We can start filling some arguments: global/constant buffers, images and arguments
  // passed by value.
  unsigned J = 0;
  for(Kernel::arg_iterator I = Kern.arg_begin(),
                           E = Kern.arg_end();
                           I != E;
                           ++I) {
    // A buffer can be allocated by the CPUDevice.
    if(BufferKernelArg *Arg = llvm::dyn_cast<BufferKernelArg>(*I)) {
      // Only global and constant buffers are handled here. Local buffers are
      // handled later.
      if(Arg->OnGlobalAddressSpace() || Arg->OnConstantAddressSpace())
        Args[J] = GlobalArgs[J];

    } 
    
    // An image can be allocated by the CPUDevice.
    else if(ImageKernelArg *Arg = llvm::dyn_cast<ImageKernelArg>(*I)) {
      Image *Img = Arg->GetImage();
      DeviceImage *DevImg = new DeviceImage(*Img, GlobalArgs[J]);
      DevImgs.push_back(DevImg);

      // Store image descriptor address.
      Args[J] = DevImg; 

    } else if(SamplerKernelArg *Arg = llvm::dyn_cast<SamplerKernelArg>(*I)) {
      Sampler *Smplr = Arg->GetSampler();
      DeviceSampler *DevSmplr = new DeviceSampler(GetDeviceSampler(*Smplr));
      DevSmplrs.push_back(DevSmplr);

      // Store sampler address.
      Args[J] = DevSmplr;

    // For arguments passed by copy, we need to setup a pointer for the stub.
    } else if(ByValueKernelArg *Arg = llvm::dyn_cast<ByValueKernelArg>(*I))
      Args[J] = Arg->GetArg();

    ++J;
  }
}

NDRangeKernelBlockCPUCommand::~NDRangeKernelBlockCPUCommand() {
  // Image descriptors can be freed.
  llvm::DeleteContainerPointers(DevImgs);
  // Sampler addresses can be freed.
  llvm::DeleteContainerPointers(DevSmplrs);
  sys::Free(Args);
}

void NDRangeKernelBlockCPUCommand::SetLocalParams(
                                    LocalMemory &Local,
                                    StaticLocalPointers &StaticLocalPtrs) {
  Kernel &Kern = GetKernel();

  unsigned J = 0;
  for(Kernel::arg_iterator I = Kern.arg_begin(),
                           E = Kern.arg_end();
                           I != E;
                           ++I) {
    BufferKernelArg *Arg = llvm::dyn_cast<BufferKernelArg>(*I);
    if(Arg && Arg->OnLocalAddressSpace())
      Args[J] = Local.Alloc(*Arg->GetBuffer());

    ++J;
  }

  if (StaticLocalSize) {
    CPUKernelInfo Info(Kern.getDescriptor().getKernelInfo());

    StaticLocalPtrs.clear();
    StaticLocalPtrs.resize(Info.getNumStaticLocalStructs());
    for (IndexOffsetVector::const_iterator I = StaticLocalInfos.begin(),
                                           E = StaticLocalInfos.end();
                                           I != E;
                                           ++I) {
      uintptr_t StaticLocalsAddr = reinterpret_cast<uintptr_t>(Local.GetStaticPtr()) + I->second;
      StaticLocalPtrs[I->first] = reinterpret_cast<void *>(StaticLocalsAddr);
    }

    Args[J] = StaticLocalPtrs.data();
  }
}

cl_uint NDRangeKernelBlockCPUCommand::GetDeviceSampler(const Sampler &Smplr) {
  cl_uint flags = 0;

  if(Smplr.GetNormalizedCoords() == true)
    flags |= CLK_NORMALIZED_COORDS_TRUE;
  else if(Smplr.GetNormalizedCoords() == false)
    flags |= CLK_NORMALIZED_COORDS_FALSE;

  switch(Smplr.GetAddressingMode()) {
    case Sampler::AddressNone:
      flags |= CLK_ADDRESS_NONE;
      break;
    case Sampler::AddressClampToEdge:
      flags |= CLK_ADDRESS_CLAMP_TO_EDGE;
      break;
    case Sampler::AddressClamp:
      flags |= CLK_ADDRESS_CLAMP;
      break;
    case Sampler::AddressRepeat:
      flags |= CLK_ADDRESS_REPEAT;
      break;
    case Sampler::AddressMirroredRepeat:
      flags |= CLK_ADDRESS_MIRRORED_REPEAT;
      break;
    case Sampler::NoAddressing:
      break;
  }

  switch(Smplr.GetFilterMode()) {
    case Sampler::FilterNearest:
      flags |= CLK_FILTER_NEAREST;
      break;
    case Sampler::FilterLinear:
      flags |= CLK_FILTER_LINEAR;
      break;
    case Sampler::NoFilter:
      break;
  }

  return flags;
}
