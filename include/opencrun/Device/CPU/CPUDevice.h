
#ifndef OPENCRUN_DEVICE_CPU_CPUDEVICE_H
#define OPENCRUN_DEVICE_CPU_CPUDEVICE_H

#include "opencrun/Core/Device.h"
#include "opencrun/Device/CPU/Memory.h"
#include "opencrun/Device/CPU/Multiprocessor.h"

#include "llvm/ExecutionEngine/JIT.h"

using namespace opencrun::cpu;

namespace opencrun {

class CPUDevice : public Device {
public:
  static bool classof(const Device *Dev) { return true; }

public:
  typedef NDRangeKernelBlockCPUCommand::Signature BlockParallelEntryPoint;

  typedef llvm::Function *KernelID;
  typedef std::map<KernelID, BlockParallelEntryPoint> BlockParallelEntryPoints;
  typedef std::map<KernelID, unsigned> BlockParallelStaticLocalSizes;
  typedef std::map<KernelID, Footprint> FootprintsContainer;

  typedef llvm::DenseMap<unsigned, void *> GlobalArgMappingsContainer;

public:
  typedef llvm::SmallPtrSet<Multiprocessor *, 4> MultiprocessorsContainer;

public:
  CPUDevice(sys::HardwareMachine &Machine);
  ~CPUDevice();

public:
  virtual bool ComputeGlobalWorkPartition(const WorkSizes &GW,
                                          WorkSizes &LW) const;

  virtual const Footprint &ComputeKernelFootprint(Kernel &Kern);

  virtual bool CreateHostBuffer(HostBuffer &Buf);
  virtual bool CreateHostAccessibleBuffer(HostAccessibleBuffer &Buf);
  virtual bool CreateDeviceBuffer(DeviceBuffer &Buf);
  
  virtual bool CreateHostImage(HostImage &Img);
  virtual bool CreateHostAccessibleImage(HostAccessibleImage &Img);
  virtual bool CreateDeviceImage(DeviceImage &Img);

  virtual void DestroyMemoryObj(MemoryObj &MemObj);

  virtual bool MappingDoesAllocation(MemoryObj::Type MemObjTy);
  virtual void *CreateMapBuffer(MemoryObj &MemObj, 
                                MemoryObj::MappingInfo &MapInfo);
  virtual void FreeMapBuffer(void *MapBuf);

  virtual bool Submit(Command &Cmd);

  virtual void UnregisterKernel(Kernel &Kern);

  void NotifyDone(CPUServiceCommand *Cmd) { delete Cmd; }
  void NotifyDone(CPUExecCommand *Cmd, int ExitStatus);

protected:
  void addOptimizerExtensions(llvm::PassManagerBuilder &PMB,
                              LLVMOptimizerParams &Params) const LLVM_OVERRIDE;

private:
  void InitDeviceInfo(sys::HardwareMachine &Machine);
  void InitJIT();
  void InitMultiprocessors(sys::HardwareMachine &Machine);

  void DestroyJIT();
  void DestroyMultiprocessors();

  bool Submit(EnqueueReadBuffer &Cmd);
  bool Submit(EnqueueWriteBuffer &Cmd);
  bool Submit(EnqueueCopyBuffer &Cmd);
  bool Submit(EnqueueReadImage &Cmd);
  bool Submit(EnqueueWriteImage &Cmd);
  bool Submit(EnqueueCopyImage &Cmd);
  bool Submit(EnqueueCopyImageToBuffer &Cmd);
  bool Submit(EnqueueCopyBufferToImage &Cmd);
  bool Submit(EnqueueMapBuffer &Cmd);
  bool Submit(EnqueueMapImage &Cmd);
  bool Submit(EnqueueUnmapMemObject &Cmd);
  bool Submit(EnqueueReadBufferRect &Cmd);
  bool Submit(EnqueueWriteBufferRect &Cmd);
  bool Submit(EnqueueCopyBufferRect &Cmd);
  bool Submit(EnqueueFillBuffer &Cmd);
  bool Submit(EnqueueFillImage &Cmd);
  bool Submit(EnqueueNDRangeKernel &Cmd);
  bool Submit(EnqueueNativeKernel &Cmd);

  bool BlockParallelSubmit(EnqueueNDRangeKernel &Cmd,
                           GlobalArgMappingsContainer &GlobalArgs);

  CPUDevice::BlockParallelEntryPoint GetBlockParallelEntryPoint(Kernel &Kern);
  unsigned GetBlockParallelStaticLocalSize(Kernel &Kern);

  void *LinkLibFunction(const std::string &Name);

  void LocateMemoryObjArgAddresses(Kernel &Kern,
                                   GlobalArgMappingsContainer &GlobalArgs);

  std::string MangleBlockParallelKernelName(llvm::StringRef Name) {
    return MangleKernelName("_GroupParallelStub_", Name);
  }

  std::string MangleKernelName(llvm::StringRef Prefix, llvm::StringRef Name) {
    return Prefix.str() + Name.str();
  }

private:
  MultiprocessorsContainer Multiprocessors;
  GlobalMemory Global;

  llvm::OwningPtr<llvm::ExecutionEngine> JIT;

  BlockParallelEntryPoints BlockParallelEntriesCache;
  BlockParallelStaticLocalSizes BlockParallelStaticLocalsCache;
  FootprintsContainer KernelFootprints;

  friend void *LibLinker(const std::string &);
};

void *LibLinker(const std::string &Name);

template <>
class ProfilerTraits<CPUDevice> {
public:
  static sys::Time ReadTime(CPUDevice &Profilable) {
    return sys::Time::GetWallClock();
  }
};

} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_CPUDEVICE_H
