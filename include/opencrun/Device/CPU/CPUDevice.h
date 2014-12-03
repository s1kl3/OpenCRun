
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
  typedef NDRangeKernelBlockCPUCommand::IndexOffsetVector BlockParallelStaticLocalVector;

  typedef const KernelDescriptor *KernelID;
  typedef std::map<KernelID, BlockParallelEntryPoint> BlockParallelEntryPoints;
  typedef std::map<KernelID, unsigned> BlockParallelStaticLocalSizes;
  typedef std::map<KernelID, BlockParallelStaticLocalVector> BlockParallelStaticLocalVectors;
  typedef std::map<KernelID, Footprint> FootprintsContainer;

  typedef llvm::DenseMap<unsigned, void *> GlobalArgMappingsContainer;

  typedef llvm::SmallPtrSet<Multiprocessor *, 4> MultiprocessorsContainer;
  typedef llvm::SmallPtrSet<const sys::HardwareCPU *, 16> HardwareCPUsContainer;
  typedef std::map<cl_device_affinity_domain, llvm::SmallVector<HardwareCPUsContainer, 4> > PartitionsContainer;

public:
  CPUDevice(const sys::HardwareMachine &Machine);
  CPUDevice(CPUDevice &Parent,
            const PartitionPropertiesContainer &PartProps,
            const HardwareCPUsContainer &CPUs);
  ~CPUDevice();

protected:
  virtual bool IsSupportedPartitionSchema(const PartitionPropertiesContainer &PartProps,
                                          cl_int &ErrCode) const;

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
  void InitDeviceInfo(const sys::HardwareMachine &Machine,
                      const HardwareCPUsContainer *CPUs = NULL);
  void InitJIT();
  void InitMultiprocessors(const sys::HardwareMachine &Machine);
  void InitMultiprocessors(const HardwareCPUsContainer &CPUs);

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

  PartitionsContainer Partitions;

  friend void *LibLinker(const std::string &);
  friend class CPUSubDevicesBuilder;
};

class CPUSubDevicesBuilder : public SubDevicesBuilder {
public:
  static bool classof(const SubDevicesBuilder *Bld) {
    return Bld->GetType() == SubDevicesBuilder::CPUSubDevicesBuilder;
  }

public:
  CPUSubDevicesBuilder(CPUDevice &Parent,
                       const DeviceInfo::PartitionPropertiesContainer &PartProps) :
    SubDevicesBuilder(SubDevicesBuilder::CPUSubDevicesBuilder, Parent, PartProps) { }

  virtual ~CPUSubDevicesBuilder() { }

public:
  virtual unsigned Create(SubDevicesContainer *SubDevs, cl_int &ErrCode);
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
