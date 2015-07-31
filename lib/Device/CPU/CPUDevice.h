
#ifndef OPENCRUN_DEVICE_CPU_CPUDEVICE_H
#define OPENCRUN_DEVICE_CPU_CPUDEVICE_H

#include "Memory.h"
#include "Multiprocessor.h"

#include "opencrun/Core/Device.h"

#include "llvm/ExecutionEngine/JIT.h"

namespace opencrun {
namespace cpu {

class CPUDevice : public Device {
public:
  static bool classof(const Device *Dev) { return true; }

public:
  typedef NDRangeKernelBlockCPUCommand::Signature BlockParallelEntryPoint;
  typedef NDRangeKernelBlockCPUCommand::IndexOffsetVector BlockParallelStaticLocalVector;

  typedef const KernelDescriptor *KernelID;
  typedef std::map<KernelID, BlockParallelEntryPoint> BlockParallelEntryPoints;
  typedef std::map<KernelID, unsigned> BlockParallelStaticLocalSizes;
  typedef std::map<KernelID, BlockParallelStaticLocalVector> BlockParallelStaticLocalVectors;
  typedef std::map<KernelID, Footprint> FootprintsContainer;

  typedef llvm::DenseMap<unsigned, void *> GlobalArgMappingsContainer;

  typedef llvm::SmallPtrSet<Multiprocessor *, 4> MultiprocessorsContainer;
  typedef llvm::SmallPtrSet<const sys::HardwareCPU *, 16> HardwareCPUsContainer;

public:
  CPUDevice(const sys::HardwareMachine &Machine);
  CPUDevice(CPUDevice &Parent,
            const DevicePartition &Part,
            const HardwareCPUsContainer &CPUs);
  ~CPUDevice();

protected:
  bool isPartitionSupported(const DevicePartition &Part) const override;
  bool createSubDevices(const DevicePartition &Part,
      llvm::SmallVectorImpl<std::unique_ptr<Device>> &Devs) override;

public:
  virtual bool ComputeGlobalWorkPartition(const WorkSizes &GW,
                                          WorkSizes &LW) const;

  virtual const Footprint &getKernelFootprint(const KernelDescriptor &Kern) const;

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

  virtual void UnregisterKernel(const KernelDescriptor &Kern);

  void NotifyDone(CPUServiceCommand *Cmd) { delete Cmd; }
  void NotifyDone(CPUExecCommand *Cmd, int ExitStatus);

protected:
  void addOptimizerExtensions(llvm::PassManagerBuilder &PMB,
                              LLVMOptimizerParams &Params) const override;

private:
  void InitDeviceInfo();
  void InitSubDeviceInfo(const HardwareCPUsContainer &CPUs);
  void InitJIT();
  void InitMultiprocessors();
  void InitMultiprocessors(const HardwareCPUsContainer &CPUs);

  void computeSubDeviceInfo(const HardwareCPUsContainer &CPUs);

  void DestroyJIT();
  void DestroyMultiprocessors();

  const sys::HardwareMachine &GetHardwareMachine() const { return Machine; }
  void GetPinnedCPUs(HardwareCPUsContainer &CPUs) const;

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
  bool Submit(EnqueueMarker &Cmd);
  bool Submit(EnqueueBarrier &Cmd);

  bool BlockParallelSubmit(EnqueueNDRangeKernel &Cmd,
                           GlobalArgMappingsContainer &GlobalArgs);

  CPUDevice::BlockParallelEntryPoint GetBlockParallelEntryPoint(const KernelDescriptor &KernDesc);
  unsigned GetBlockParallelStaticLocalSize(const KernelDescriptor &KernDesc);
  void GetBlockParallelStaticLocalVector(const KernelDescriptor &KernDesc,
                                         BlockParallelStaticLocalVector &SLVec);

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
  // A CPUDevice is associated to an hardware machine (i.e. a single node in
  // a cluster system). In general it uses a subset of its logical cores.
  const sys::HardwareMachine &Machine;
  MultiprocessorsContainer Multiprocessors;
  GlobalMemory Global;

  std::unique_ptr<llvm::ExecutionEngine> JIT;

  BlockParallelEntryPoints BlockParallelEntriesCache;
  BlockParallelStaticLocalSizes BlockParallelStaticLocalsCache;
  BlockParallelStaticLocalVectors BlockParallelStaticLocalVectorsCache;
  mutable FootprintsContainer KernelFootprints;

  friend void *LibLinker(const std::string &);
};

void *LibLinker(const std::string &Name);

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_CPUDEVICE_H
