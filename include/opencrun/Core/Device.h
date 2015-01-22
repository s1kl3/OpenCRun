
#ifndef OPENCRUN_CORE_DEVICE_H
#define OPENCRUN_CORE_DEVICE_H

#include "opencrun/Core/Command.h"
#include "opencrun/Core/Profiler.h"
#include "opencrun/System/Env.h"
#include "opencrun/Util/Footprint.h"
#include "opencrun/Util/MTRefCounted.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/ADT/ArrayRef.h"

struct _cl_device_id { };

namespace llvm {
class PassManagerBuilder;
}

namespace opencrun {

class DeviceBuffer;
class DeviceImage;
class HostAccessibleBuffer;
class HostAccessibleImage;
class HostBuffer;
class HostImage;
class LLVMOptimizerParams;
template<class InterfaceTy> class LLVMOptimizerInterfaceTraits;
class MemoryObj;

class DeviceInfo {
public:
  enum DeviceType {
    CPUType = CL_DEVICE_TYPE_CPU,
    GPUType = CL_DEVICE_TYPE_GPU,
    AcceleratorType = CL_DEVICE_TYPE_ACCELERATOR,
    CustomType = CL_DEVICE_TYPE_CUSTOM
  };

  typedef llvm::SmallVector<size_t, 4> MaxWorkItemSizesContainer;

  enum {
    FPDenormalization = CL_FP_DENORM,
    FPInfNaN = CL_FP_INF_NAN,
    FPRoundToNearest = CL_FP_ROUND_TO_NEAREST,
    FPRoundToZero = CL_FP_ROUND_TO_ZERO,
    FPRoundToInf = CL_FP_ROUND_TO_INF,
    FPFusedMultiplyAdd = CL_FP_FMA,
    FPCorrectlyRoundedDivideSqrt = CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT,
    FPSoftFloat = CL_FP_SOFT_FLOAT
  };

  enum CacheType {
    NoCache = CL_NONE,
    ReadOnlyCache = CL_READ_ONLY_CACHE,
    ReadWriteCache = CL_READ_WRITE_CACHE
  };

  enum LocalMemoryType {
    PrivateLocal = CL_LOCAL,
    SharedLocal = CL_GLOBAL
  };

  enum {
    CanExecKernel = CL_EXEC_KERNEL,
    CanExecNativeKernel = CL_EXEC_NATIVE_KERNEL
  };

  enum {
    OutOfOrderExecMode = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
    ProfilingEnabled = CL_QUEUE_PROFILING_ENABLE
  };

  enum PartitionType {
    PartitionEqually = CL_DEVICE_PARTITION_EQUALLY,
    PartitionByCounts = CL_DEVICE_PARTITION_BY_COUNTS,
    PartitionByCountsListEnd = CL_DEVICE_PARTITION_BY_COUNTS_LIST_END,
    PartitionByAffinityDomain = CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN,
  };

  enum PartitionAffinity {
    AffinityDomainNUMA = CL_DEVICE_AFFINITY_DOMAIN_NUMA,
    AffinityDomainL4Cache = CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE,
    AffinityDomainL3Cache = CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE,
    AffinityDomainL2Cache = CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE,
    AffinityDomainL1Cache = CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE,
    AffinityDomainNext = CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE
  };

  class DevicePartition {
  public:
    explicit DevicePartition(const cl_device_partition_property *P = nullptr);

    PartitionType getType() const;

    PartitionAffinity getAffinityDomain() const;

    unsigned getNumComputeUnits() const;

    unsigned getCount(unsigned i) const;
    unsigned getNumCounts() const;
    unsigned getTotalComputeUnits() const;

    llvm::ArrayRef<cl_device_partition_property> getRawProperties() const {
      return Props;
    }
  private:
    llvm::SmallVector<cl_device_partition_property, 8> Props;
  };

public:
  DeviceInfo(DeviceType Ty);
  DeviceInfo(const DeviceInfo &DI);

public:
  // OpenCL properties.
  DeviceType GetType() const { return Type; }
  unsigned GetVendorID() const { return VendorID; }
  unsigned GetMaxComputeUnits() const { return MaxComputeUnits; }
  unsigned GetMaxWorkItemDimensions() const { return MaxWorkItemDimensions; }
  llvm::SmallVector<size_t, 4> &GetMaxWorkItemSizes() const {
    return *const_cast<MaxWorkItemSizesContainer *>(&MaxWorkItemSizes);
  }
  size_t GetMaxWorkGroupSize() const { return MaxWorkGroupSize; }
  unsigned GetPreferredCharVectorWidth() const {
    return PreferredCharVectorWidth;
  }
  unsigned GetPreferredShortVectorWidth() const {
    return PreferredShortVectorWidth;
  }
  unsigned GetPreferredIntVectorWidth() const {
    return PreferredIntVectorWidth;
  }
  unsigned GetPreferredLongVectorWidth() const {
    return PreferredLongVectorWidth;
  }
  unsigned GetPreferredFloatVectorWidth() const {
    return PreferredFloatVectorWidth;
  }
  unsigned GetPreferredDoubleVectorWidth() const {
    return PreferredDoubleVectorWidth;
  }
  unsigned GetPreferredHalfVectorWidth() const {
    return PreferredHalfVectorWidth;
  }
  unsigned GetNativeCharVectorWidth() const {
    return NativeCharVectorWidth;
  }
  unsigned GetNativeShortVectorWidth() const {
    return NativeShortVectorWidth;
  }
  unsigned GetNativeIntVectorWidth() const {
    return NativeIntVectorWidth;
  }
  unsigned GetNativeLongVectorWidth() const {
    return NativeLongVectorWidth;
  }
  unsigned GetNativeFloatVectorWidth() const {
    return NativeFloatVectorWidth;
  }
  unsigned GetNativeDoubleVectorWidth() const {
    return NativeDoubleVectorWidth;
  }
  unsigned GetNativeHalfVectorWidth() const {
    return NativeHalfVectorWidth;
  }
  unsigned GetMaxClockFrequency() const { return MaxClockFrequency; }
  unsigned GetAddressBits() const { return AddressBits; }

  size_t GetMaxMemoryAllocSize() const { return MaxMemoryAllocSize; }

  bool HasImageSupport() const { return SupportImages; }
  unsigned GetMaxReadableImages() const { return MaxReadableImages; }
  unsigned GetMaxWriteableImages() const { return MaxWriteableImages; }
  size_t GetImage2DMaxWidth() const { return Image2DMaxWidth; }
  size_t GetImage2DMaxHeight() const { return Image2DMaxHeight; }
  size_t GetImage3DMaxWidth() const { return Image3DMaxWidth; }
  size_t GetImage3DMaxHeight() const { return Image3DMaxHeight; }
  size_t GetImage3DMaxDepth() const { return Image3DMaxDepth; }
  size_t GetImageMaxBufferSize() const { return ImageMaxBufferSize; }
  size_t GetImageMaxArraySize() const { return ImageMaxArraySize; }
  unsigned GetMaxSamplers() const { return MaxSamplers; }

  llvm::ArrayRef<cl_image_format> GetSupportedImageFormats() const { 
    return llvm::ArrayRef<cl_image_format>(ImgFmts, NumImgFmts); 
  }

  size_t GetMaxParameterSize() const { return MaxParameterSize; }

  size_t GetPrintfBufferSize() const { return PrintfBufferSize; }

  unsigned GetMemoryBaseAddressAlignment() const {
    return MemoryBaseAddressAlignment;
  }
  unsigned GetMinimumDataTypeAlignment() const {
    return MinimumDataTypeAlignment;
  }

  unsigned GetSinglePrecisionFPCapabilities() const {
    return SinglePrecisionFPCapabilities;
  }

  unsigned GetDoublePrecisionFPCapabilities() const {
    return DoublePrecisionFPCapabilities;
  }

  unsigned GetHalfPrecisionFPCapabilities() const {
    return HalfPrecisionFPCapabilities;
  }

  CacheType GetGlobalMemoryCacheType() const { return GlobalMemoryCacheType; }
  size_t GetGlobalMemoryCachelineSize() const {
    return GlobalMemoryCachelineSize;
  }
  size_t GetGlobalMemoryCacheSize() const { return GlobalMemoryCacheSize; }
  size_t GetGlobalMemorySize() const { return GlobalMemorySize; }

  size_t GetMaxConstantBufferSize() const { return MaxConstantBufferSize; }
  unsigned GetMaxConstantArguments() const { return MaxConstantArguments; }

  LocalMemoryType GetLocalMemoryType() const { return LocalMemoryMapping; }
  size_t GetLocalMemorySize() const { return LocalMemorySize; }
  bool HasErrorCorrectionSupport() const { return SupportErrorCorrection; }

  bool HasHostUnifiedMemory() const { return HostUnifiedMemory; }

  bool IsPreferredInteropUserSync() const { return PreferredInteropUserSync; }

  unsigned long GetProfilingTimerResolution() const {
    return ProfilingTimerResolution;
  }

  bool IsLittleEndian() const { return LittleEndian; }
  virtual bool IsAvailable() const { return true; }

  bool IsCompilerAvailable() const { return CompilerAvailable; }
  bool IsLinkerAvailable() const { return LinkerAvailable; }

  unsigned GetExecutionCapabilities() const { return ExecutionCapabilities; }

  unsigned GetQueueProperties() const { return QueueProperties; }

  unsigned GetMaxSubDevices() const { return MaxSubDevices; }
  llvm::ArrayRef<PartitionType> GetSupportedPartitionTypes() const {
    return SupportedPartitionTypes;
  }

  unsigned GetSupportedAffinityDomains() const {
    return SupportedAffinityDomains;
  }

  llvm::StringRef GetVendor() const { return Vendor; }
  llvm::StringRef GetName() const { return Name; }
  llvm::StringRef GetVersion() const { return Version; }
  llvm::StringRef GetDriverVersion() const { return DriverVersion; }
  llvm::StringRef GetOpenCLCVersion() const { return OpenCLCVersion; }
  llvm::StringRef GetProfile() const { return Profile; }
  llvm::StringRef GetExtensions() const { return Extensions; }
  llvm::StringRef GetBuiltInKernels() const { return BuiltInKernels; }

  // Other, non OpenCL specific properties.

  unsigned long long GetSizeTypeMax() const {
    return SizeTypeMax;
  }
  size_t GetPrivateMemorySize() const { return PrivateMemorySize; }
  size_t GetPreferredWorkGroupSizeMultiple() const {
    return PreferredWorkGroupSizeMultiple;
  }

  // Derived properties.

  bool SupportsNativeKernels() const {
    return ExecutionCapabilities & CanExecNativeKernel;
  }

protected:
  DeviceType Type;
  unsigned VendorID;

  unsigned MaxComputeUnits;
  unsigned MaxWorkItemDimensions;
  MaxWorkItemSizesContainer MaxWorkItemSizes;
  unsigned MaxWorkGroupSize;

  unsigned PreferredCharVectorWidth;
  unsigned PreferredShortVectorWidth;
  unsigned PreferredIntVectorWidth;
  unsigned PreferredLongVectorWidth;
  unsigned PreferredFloatVectorWidth;
  unsigned PreferredDoubleVectorWidth;
  unsigned PreferredHalfVectorWidth;

  unsigned NativeCharVectorWidth;
  unsigned NativeShortVectorWidth;
  unsigned NativeIntVectorWidth;
  unsigned NativeLongVectorWidth;
  unsigned NativeFloatVectorWidth;
  unsigned NativeDoubleVectorWidth;
  unsigned NativeHalfVectorWidth;

  unsigned MaxClockFrequency;
  unsigned AddressBits;
  size_t MaxMemoryAllocSize;

  bool SupportImages;
  unsigned MaxReadableImages;
  unsigned MaxWriteableImages;
  size_t Image2DMaxWidth;
  size_t Image2DMaxHeight;
  size_t Image3DMaxWidth;
  size_t Image3DMaxHeight;
  size_t Image3DMaxDepth;
  size_t ImageMaxBufferSize;
  size_t ImageMaxArraySize;
  unsigned MaxSamplers;
  const cl_image_format *ImgFmts;
  size_t NumImgFmts;

  size_t MaxParameterSize;

  size_t PrintfBufferSize;

  unsigned MemoryBaseAddressAlignment;
  unsigned MinimumDataTypeAlignment;

  unsigned SinglePrecisionFPCapabilities;
  unsigned DoublePrecisionFPCapabilities;
  unsigned HalfPrecisionFPCapabilities;

  CacheType GlobalMemoryCacheType;
  size_t GlobalMemoryCachelineSize;
  size_t GlobalMemoryCacheSize;
  size_t GlobalMemorySize;

  size_t MaxConstantBufferSize;
  unsigned MaxConstantArguments;

  LocalMemoryType LocalMemoryMapping;
  size_t LocalMemorySize;
  bool SupportErrorCorrection;

  bool HostUnifiedMemory;

  bool PreferredInteropUserSync;

  unsigned long ProfilingTimerResolution;

  bool LittleEndian;
  
  // Available is a virtual property.
  bool CompilerAvailable;
  bool LinkerAvailable;

  unsigned ExecutionCapabilities;

  unsigned QueueProperties;

  unsigned MaxSubDevices;
  llvm::SmallVector<PartitionType, 4> SupportedPartitionTypes;
  unsigned SupportedAffinityDomains;

  llvm::StringRef Vendor;
  llvm::StringRef Name;
  llvm::StringRef Version;
  llvm::StringRef DriverVersion;
  llvm::StringRef OpenCLCVersion;
  llvm::StringRef Profile;
  llvm::StringRef Extensions;
  llvm::StringRef BuiltInKernels;

  // Other, non OpenCL specific properties.

  long long SizeTypeMax;
  size_t PrivateMemorySize;
  size_t PreferredWorkGroupSizeMultiple;
};

class Device : public _cl_device_id,
               public DeviceInfo,
               public MTRefCountedBaseVPTR<Device> {
public:
  typedef llvm::SmallVector<size_t, 4> WorkSizes;

public:
  static bool classof(const _cl_device_id *Dev) { return true; }

protected:
  Device(DeviceType Ty, llvm::StringRef Name, llvm::StringRef Triple);
  Device(Device &Parent, const DevicePartition &Part); 

public:
  virtual ~Device();

public:
  bool IsSubDevice() const { return Parent; }
  Device *GetParent() const { return Parent; }
  const DevicePartition &getPartition() const { return Partition; }

  virtual bool isPartitionSupported(const DevicePartition &P) const {
    return false;
  }

  virtual bool createSubDevices(const DevicePartition &P,
      llvm::SmallVectorImpl<std::unique_ptr<Device>> &Devs) {
    return false;
  }

public:
  virtual bool ComputeGlobalWorkPartition(const WorkSizes &GW, 
                                          WorkSizes &LW) const {
    return false;
  }

  virtual const Footprint &getKernelFootprint(const KernelDescriptor &KD) const = 0;

  virtual bool CreateHostBuffer(HostBuffer &Buf) = 0;
  virtual bool CreateHostAccessibleBuffer(HostAccessibleBuffer &Buf) = 0;
  virtual bool CreateDeviceBuffer(DeviceBuffer &Buf) = 0;

  virtual bool CreateHostImage(HostImage &Img) = 0;
  virtual bool CreateHostAccessibleImage(HostAccessibleImage &Img) = 0;
  virtual bool CreateDeviceImage(DeviceImage &Img) = 0;

  virtual void DestroyMemoryObj(MemoryObj &MemObj) = 0;

  virtual bool MappingDoesAllocation(MemoryObj::Type MemObjTy) = 0;
  virtual void *CreateMapBuffer(MemoryObj &MemObj,
                                MemoryObj::MappingInfo &MapInfo) = 0;
  virtual void FreeMapBuffer(void *MapBuf) = 0;
  
  virtual bool Submit(Command &Cmd) = 0;

  bool TranslateToBitCode(llvm::StringRef Opts,
                          clang::DiagnosticConsumer &Diag,
                          llvm::MemoryBuffer &Src,
                          llvm::Module *&Mod);

  virtual void RegisterKernel(const KernelDescriptor &Kern) { }
  virtual void UnregisterKernel(const KernelDescriptor &Kern) { }

protected:
  virtual void addOptimizerExtensions(llvm::PassManagerBuilder &PMB,
                                      LLVMOptimizerParams &Params) const {}

private:
  void InitLibrary();
  void InitCompiler();

  void BuildCompilerInvocation(llvm::StringRef UserOpts,
                               llvm::MemoryBuffer &Src,
                               clang::CompilerInvocation &Invocation,
                               clang::DiagnosticsEngine &Diags);

public:
  llvm::LLVMContext &GetContext() { return LLVMCtx; }
  llvm::StringRef GetTriple() const { return Triple; }
  llvm::StringRef GetEnvCompilerOpts() const { return EnvCompilerOpts; }
  llvm::Module *GetBitCodeLibrary() const { return BitCodeLibrary.get(); }

protected:
  mutable llvm::sys::Mutex ThisLock;

  llvm::LLVMContext LLVMCtx;
  std::unique_ptr<llvm::Module> BitCodeLibrary;

  Device *Parent;
  DevicePartition Partition;

private:
  std::string EnvCompilerOpts;

  std::string Triple;
  std::string SystemResourcePath;


  friend class LLVMOptimizerInterfaceTraits<Device>;
  friend class DeviceBuiltinInfo;
};

template <>
class ProfilerTraits<Device> {
public:
  static sys::Time ReadTime(Device &Profilable);
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_DEVICE_H
