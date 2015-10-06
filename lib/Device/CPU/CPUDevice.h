
#ifndef OPENCRUN_DEVICE_CPU_CPUDEVICE_H
#define OPENCRUN_DEVICE_CPU_CPUDEVICE_H

#include "Multiprocessor.h"

#include "opencrun/Core/Device.h"

namespace opencrun {
namespace cpu {

class CPUDevice : public Device {
public:
  static bool classof(const Device *Dev) { return true; }

public:
  typedef const KernelDescriptor *KernelID;
  typedef std::map<KernelID, unsigned> BlockParallelStaticLocalSizes;
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

  std::unique_ptr<MemoryDescriptor>
    createMemoryDescriptor(const MemoryObject &Obj) override;

  virtual bool Submit(Command &Cmd);

  virtual void UnregisterKernel(const KernelDescriptor &Kern);

  void NotifyDone(CPUServiceCommand *Cmd) { delete Cmd; }
  void NotifyDone(CPUExecCommand *Cmd, int ExitStatus);

private:
  void InitDeviceInfo();
  void InitSubDeviceInfo(const HardwareCPUsContainer &CPUs);
  void InitMultiprocessors();
  void InitMultiprocessors(const HardwareCPUsContainer &CPUs);
  void InitCompiler();

  void computeSubDeviceInfo(const HardwareCPUsContainer &CPUs);

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

  unsigned GetBlockParallelStaticLocalSize(const KernelDescriptor &KernDesc);

  void LocateMemoryObjArgAddresses(Kernel &Kern,
                                   GlobalArgMappingsContainer &GlobalArgs);

  MemoryDescriptor &getMemoryDescriptor(const MemoryObject &Obj);

private:
  // A CPUDevice is associated to an hardware machine (i.e. a single node in
  // a cluster system). In general it uses a subset of its logical cores.
  const sys::HardwareMachine &Machine;
  MultiprocessorsContainer Multiprocessors;

  BlockParallelStaticLocalSizes BlockParallelStaticLocalsCache;
  mutable FootprintsContainer KernelFootprints;
};

class CPUMemoryDescriptor : public MemoryDescriptor {
public:
  CPUMemoryDescriptor(const Device &Dev, const MemoryObject &Obj)
    : MemoryDescriptor(Dev, Obj), Ptr(nullptr) {}
  virtual ~CPUMemoryDescriptor();

  bool allocate() override;

  bool aliasWithHostPtr() const override;

  void *map() override;
  void unmap() override;

  void *ptr() override { return map(); }

private:
  void *Ptr;
};

} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_CPUDEVICE_H
