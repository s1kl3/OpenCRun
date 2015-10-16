
#ifndef OPENCRUN_DEVICE_CPU_CPUDEVICE_H
#define OPENCRUN_DEVICE_CPU_CPUDEVICE_H

#include "Command.h"
#include "CPUKernelArguments.h"

#include "opencrun/Core/Device.h"
#include "opencrun/System/Hardware.h"

namespace opencrun {
namespace cpu {

class CPUThread;

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

protected:
  void *allocateStorage();

protected:
  void *Ptr;
};

class CPUImageDescriptor : public CPUMemoryDescriptor {
public:
  CPUImageDescriptor(const Device &Dev, const Image &Img)
   : CPUMemoryDescriptor(Dev, Img), ImgHeader(nullptr) {}
  virtual ~CPUImageDescriptor();

  bool allocate() override;

  void *imageHeader() {
    tryAllocate();

    return ImgHeader;
  }

private:
  void *ImgHeader;
};

class CPUDevice : public Device {
public:
  static bool classof(const Device *Dev) { return true; }

public:
  typedef const KernelDescriptor *KernelID;
  typedef std::map<KernelID, Footprint> FootprintsContainer;

  typedef llvm::DenseMap<unsigned, void *> GlobalArgMappingsContainer;

public:
  CPUDevice(const sys::HardwareMachine &Machine);
  CPUDevice(CPUDevice &Parent, const DevicePartition &Part,
            llvm::ArrayRef<const sys::HardwareCPU*> CPUs);
  ~CPUDevice();

protected:
  bool isPartitionSupported(const DevicePartition &Part) const override;
  bool createSubDevices(const DevicePartition &Part,
      llvm::SmallVectorImpl<std::unique_ptr<Device>> &Devs) override;

  bool ComputeGlobalWorkPartition(const WorkSizes &GW,
                                  WorkSizes &LW) const override;

  const Footprint &getKernelFootprint(const KernelDescriptor &K) const override;

  std::unique_ptr<MemoryDescriptor>
    createMemoryDescriptor(const MemoryObject &Obj) override;

  bool Submit(Command &Cmd) override;

  void UnregisterKernel(const KernelDescriptor &Kern) override;

private:
  void InitDeviceInfo();
  void InitSubDeviceInfo();
  void InitThreads();
  void InitCompiler();

  CPUThread &pickThread();
  CPUThread &pickLeastLoadedThread();

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

  bool BlockParallelSubmit(EnqueueNDRangeKernel &Cmd);

  CPUKernelArguments createKernelArguments(Kernel &Kern, size_t ALS);

  CPUMemoryDescriptor &getMemoryDescriptor(const MemoryObject &Obj);
  CPUImageDescriptor &getMemoryDescriptor(const Image &Obj);

private:
  std::vector<std::unique_ptr<CPUThread>> Threads;

  // A CPUDevice is associated to an hardware machine (i.e. a single node in
  // a cluster system). In general it uses a subset of its logical cores.
  const sys::HardwareMachine &Machine;
  llvm::SmallVector<const sys::HardwareCPU*, 16> HWCPUs;

  mutable FootprintsContainer KernelFootprints;
};


} // End namespace cpu.
} // End namespace opencrun.

#endif // OPENCRUN_DEVICE_CPU_CPUDEVICE_H
