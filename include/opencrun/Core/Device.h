
#ifndef OPENCRUN_CORE_DEVICE_H
#define OPENCRUN_CORE_DEVICE_H

#include "opencrun/Core/Command.h"
#include "opencrun/Core/Profiler.h"
#include "opencrun/System/Env.h"
#include "opencrun/Util/Footprint.h"

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

public:
  DeviceInfo() : PreferredCharVectorWidth(0),
                 PreferredShortVectorWidth(0),
                 PreferredIntVectorWidth(0),
                 PreferredLongVectorWidth(0),
                 PreferredFloatVectorWidth(0),
                 PreferredDoubleVectorWidth(0),
                 PreferredHalfVectorWidth(0),
                 NativeCharVectorWidth(0),
                 NativeShortVectorWidth(0),
                 NativeIntVectorWidth(0),
                 NativeLongVectorWidth(0),
                 NativeFloatVectorWidth(0),
                 NativeDoubleVectorWidth(0),
                 NativeHalfVectorWidth(0),
                 MaxClockFrequency(0),
                 AddressBits(0),
                
                 SupportImages(false),
                 MaxReadableImages(0),
                 MaxWriteableImages(0),
                 Image2DMaxWidth(0),
                 Image2DMaxHeight(0),
                 Image3DMaxWidth(0),
                 Image3DMaxHeight(0),
                 Image3DMaxDepth(0),
                 ImageMaxBufferSize(0),
                 ImageMaxArraySize(0),
                 MaxSamplers(0),
                 ImgFmts(NULL),
                 NumImgFmts(0),
                
                 MaxParameterSize(0),

                 PrintfBufferSize(0),
                
                 MemoryBaseAddressAlignment(0),
                 MinimumDataTypeAlignment(0),
                
                 SinglePrecisionFPCapabilities(FPRoundToNearest | FPInfNaN),
                 DoublePrecisionFPCapabilities(0),
                 HalfPrecisionFPCapabilities(0),
                
                 GlobalMemoryCacheType(NoCache),
                 GlobalMemoryCachelineSize(0),
                 GlobalMemoryCacheSize(0),
                 GlobalMemorySize(0),

                 MaxConstantBufferSize(64 * 1024),
                 MaxConstantArguments(8),

                 LocalMemoryMapping(PrivateLocal),
                 LocalMemorySize(32 * 1024),
                 SupportErrorCorrection(false),

                 HostUnifiedMemory(false),

                 PreferredInteropUserSync(true),

                 ProfilingTimerResolution(0),

                 LittleEndian(true),
                 // Available is a virtual attribute.

                 CompilerAvailable(false),
                 LinkerAvailable(false),

                 ExecutionCapabilities(CanExecKernel),

                 QueueProperties(ProfilingEnabled),

                 Vendor(""),
                 Name(""),
                 Version(""),
                 DriverVersion(""),
                 OpenCLCVersion(""),
                 Profile(""),
                 Extensions(""),
                 BuiltInKernels(""),

                 // Other, non OpenCL specific properties.
                 SizeTypeMax((1ull << 32) - 1),
                 PrivateMemorySize(0),
                 PreferredWorkGroupSizeMultiple(1)
                 { }

public:
  // OpenCL properties.

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

class Device : public _cl_device_id, public DeviceInfo {
public:
  typedef llvm::SmallVector<size_t, 4> WorkSizes;

public:
  static bool classof(const _cl_device_id *Dev) { return true; }

protected:
  Device(llvm::StringRef Name, llvm::StringRef Triple);

  virtual ~Device() { }

public:
  virtual bool ComputeGlobalWorkPartition(const WorkSizes &GW, 
                                          WorkSizes &LW) const {
    return false;
  }

  virtual const Footprint &ComputeKernelFootprint(Kernel &Kern) = 0;

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

  virtual void RegisterKernel(Kernel &Kern) { }
  virtual void UnregisterKernel(Kernel &Kern) { }

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

protected:
  llvm::sys::Mutex ThisLock;

  llvm::LLVMContext LLVMCtx;
  llvm::OwningPtr<llvm::Module> BitCodeLibrary;

private:
  std::string EnvCompilerOpts;

  std::string Triple;
  std::string SystemResourcePath;

  friend class LLVMOptimizerInterfaceTraits<Device>;
  friend class DeviceBuiltinInfo;
};

class GPUDevice : public Device {
public:
  static bool classof(const Device *Dev) { return true; }
};

class AcceleratorDevice : public Device {
public:
  static bool classof(const Device *Dev) { return true; }
};

template <>
class ProfilerTraits<Device> {
public:
  static sys::Time ReadTime(Device &Profilable);
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_DEVICE_H
