#include "opencrun/Core/Device.h"
#include "opencrun/Core/DeviceCompiler.h"

using namespace opencrun;

DeviceInfo::DeviceInfo(DeviceType Ty) : Type(Ty) {
  Vendor = "";
  Name = "";
  Version = "";
  DriverVersion = "";
  OpenCLCVersion = "";
  Profile = "";
  Extensions = "";
  BuiltInKernels = "";

  AddressBits = 0;
  ExecutionCapabilities = CanExecKernel;
  HostUnifiedMemory = false;
  LittleEndian = true;
  SupportErrorCorrection = false;

  MemoryBaseAddressAlignment = 0;
  MinimumDataTypeAlignment = 0;

  SinglePrecisionFPCapabilities = FPRoundToNearest | FPInfNaN;
  DoublePrecisionFPCapabilities = 0;
  HalfPrecisionFPCapabilities = 0;

  CompilerAvailable = false;
  LinkerAvailable = false;

  GlobalMemoryCacheType = NoCache;
  GlobalMemoryCachelineSize = 0;
  GlobalMemoryCacheSize = 0;
  GlobalMemorySize = 0;

  LocalMemoryMapping = PrivateLocal;
  LocalMemorySize = 32 * 1024;

  MaxClockFrequency = 0;
  MaxComputeUnits = 0;
  MaxConstantBufferSize = 64 * 1024;
  MaxConstantArguments = 8;
  MaxParameterSize = 0;

  // Native vector widths
  NativeCharVectorWidth = 0;
  NativeShortVectorWidth = 0;
  NativeIntVectorWidth = 0;
  NativeLongVectorWidth = 0;
  NativeFloatVectorWidth = 0;
  NativeDoubleVectorWidth = 0;
  NativeHalfVectorWidth = 0;

  // Preferred vector widths
  PreferredCharVectorWidth = 0;
  PreferredShortVectorWidth = 0;
  PreferredIntVectorWidth = 0;
  PreferredLongVectorWidth = 0;
  PreferredFloatVectorWidth = 0;
  PreferredDoubleVectorWidth = 0;
  PreferredHalfVectorWidth = 0;

  PrintfBufferSize = 0;

  PreferredInteropUserSync = true;

  ProfilingTimerResolution = 0;

  QueueProperties = ProfilingEnabled;

  // Images support
  SupportImages = false;
  MaxReadableImages = 0;
  MaxWriteableImages = 0;
  Image2DMaxWidth = 0;
  Image2DMaxHeight = 0;
  Image3DMaxWidth = 0;
  Image3DMaxHeight = 0;
  Image3DMaxDepth = 0;
  ImageMaxBufferSize = 0;
  ImageMaxArraySize = 0;
  MaxSamplers = 0;
  ImgFmts = nullptr;
  NumImgFmts = 0;

  // SubDevices support
  MaxSubDevices = 0;
  SupportedAffinityDomains = 0;

  // Other, non OpenCL specific properties.
  SizeTypeMax = (1ull << 32) - 1;
  PrivateMemorySize = 0;
  PreferredWorkGroupSizeMultiple = 1;
}

DeviceInfo::DeviceInfo(const DeviceInfo &DI)
 : Type(DI.Type),
   VendorID(DI.VendorID),

   MaxComputeUnits(DI.MaxComputeUnits),
   MaxWorkItemDimensions(DI.MaxWorkItemDimensions),
   MaxWorkItemSizes(DI.MaxWorkItemSizes),
   MaxWorkGroupSize(DI.MaxWorkGroupSize),

   PreferredCharVectorWidth(DI.PreferredCharVectorWidth),
   PreferredShortVectorWidth(DI.PreferredShortVectorWidth),
   PreferredIntVectorWidth(DI.PreferredIntVectorWidth),
   PreferredLongVectorWidth(DI.PreferredLongVectorWidth),
   PreferredFloatVectorWidth(DI.PreferredFloatVectorWidth),
   PreferredDoubleVectorWidth(DI.PreferredDoubleVectorWidth),
   PreferredHalfVectorWidth(DI.PreferredHalfVectorWidth),

   NativeCharVectorWidth(DI.NativeCharVectorWidth),
   NativeShortVectorWidth(DI.NativeShortVectorWidth),
   NativeIntVectorWidth(DI.NativeIntVectorWidth),
   NativeLongVectorWidth(DI.NativeLongVectorWidth),
   NativeFloatVectorWidth(DI.NativeFloatVectorWidth),
   NativeDoubleVectorWidth(DI.NativeDoubleVectorWidth),
   NativeHalfVectorWidth(DI.NativeHalfVectorWidth),

   MaxClockFrequency(DI.MaxClockFrequency),
   AddressBits(DI.AddressBits),
   MaxMemoryAllocSize(DI.MaxMemoryAllocSize),

   SupportImages(DI.SupportImages),
   MaxReadableImages(DI.MaxReadableImages),
   MaxWriteableImages(DI.MaxWriteableImages),
   Image2DMaxWidth(DI.Image2DMaxWidth),
   Image2DMaxHeight(DI.Image2DMaxHeight),
   Image3DMaxWidth(DI.Image3DMaxWidth),
   Image3DMaxHeight(DI.Image3DMaxHeight),
   Image3DMaxDepth(DI.Image3DMaxDepth),
   ImageMaxBufferSize(DI.ImageMaxBufferSize),
   ImageMaxArraySize(DI.ImageMaxArraySize),
   MaxSamplers(DI.MaxSamplers),
   ImgFmts(DI.ImgFmts),
   NumImgFmts(DI.NumImgFmts),

   MaxParameterSize(DI.MaxParameterSize),

   PrintfBufferSize(DI.PrintfBufferSize),

   MemoryBaseAddressAlignment(DI.MemoryBaseAddressAlignment),
   MinimumDataTypeAlignment(DI.MinimumDataTypeAlignment),

   SinglePrecisionFPCapabilities(DI.SinglePrecisionFPCapabilities),
   DoublePrecisionFPCapabilities(DI.DoublePrecisionFPCapabilities),
   HalfPrecisionFPCapabilities(DI.HalfPrecisionFPCapabilities),

   GlobalMemoryCacheType(DI.GlobalMemoryCacheType),
   GlobalMemoryCachelineSize(DI.GlobalMemoryCachelineSize),
   GlobalMemoryCacheSize(DI.GlobalMemoryCacheSize),
   GlobalMemorySize(DI.GlobalMemorySize),

   MaxConstantBufferSize(DI.MaxConstantBufferSize),
   MaxConstantArguments(DI.MaxConstantArguments),

   LocalMemoryMapping(DI.LocalMemoryMapping),
   LocalMemorySize(DI.LocalMemorySize),
   SupportErrorCorrection(DI.SupportErrorCorrection),

   HostUnifiedMemory(DI.HostUnifiedMemory),

   PreferredInteropUserSync(DI.PreferredInteropUserSync),

   ProfilingTimerResolution(DI.ProfilingTimerResolution),

   LittleEndian(DI.LittleEndian),

   CompilerAvailable(DI.CompilerAvailable),
   LinkerAvailable(DI.LinkerAvailable),

   ExecutionCapabilities(DI.ExecutionCapabilities),

   QueueProperties(DI.QueueProperties),

   MaxSubDevices(DI.MaxSubDevices),
   SupportedPartitionTypes(DI.SupportedPartitionTypes),
   SupportedAffinityDomains(DI.SupportedAffinityDomains),

   Vendor(DI.Vendor),
   Name(DI.Name),
   Version(DI.Version),
   DriverVersion(DI.DriverVersion),
   OpenCLCVersion(DI.OpenCLCVersion),
   Profile(DI.Profile),
   Extensions(DI.Extensions),
   BuiltInKernels(DI.BuiltInKernels),

   SizeTypeMax(DI.SizeTypeMax),
   PrivateMemorySize(DI.PrivateMemorySize),
   PreferredWorkGroupSizeMultiple(DI.PreferredWorkGroupSizeMultiple)
{
}

DeviceInfo::
DevicePartition::DevicePartition(const cl_device_partition_property *P) {
  if (P) {
    Props.push_back(P[0]);
    switch (getType()) {
    case PartitionByAffinityDomain:
    case PartitionEqually: {
      Props.push_back(P[1]);
      Props.push_back(P[2]);
      break;
    }
    case PartitionByCounts: {
      int i = 1;
      for (; P[i] != PartitionByCountsListEnd; ++i)
        Props.push_back(P[i]);
      Props.push_back(P[i]);
      Props.push_back(P[i + 1]);
      break;
    }
    default: llvm_unreachable(0);
    }
  }
}

DeviceInfo::PartitionType DeviceInfo::DevicePartition::getType() const {
  switch (Props[0]) {
  case PartitionByAffinityDomain:
  case PartitionByCounts:
  case PartitionEqually:
    return static_cast<DeviceInfo::PartitionType>(Props[0]);
  default: llvm_unreachable(0);
  }
}

DeviceInfo::PartitionAffinity
DeviceInfo::DevicePartition::getAffinityDomain() const {
  assert(getType() == PartitionByAffinityDomain);

  switch (Props[1]) {
  case AffinityDomainNUMA:
  case AffinityDomainL1Cache:
  case AffinityDomainL2Cache:
  case AffinityDomainL3Cache:
  case AffinityDomainL4Cache:
  case AffinityDomainNext:
    return static_cast<DeviceInfo::PartitionAffinity>(Props[1]);
  default: llvm_unreachable(0);
  }
}

unsigned DeviceInfo::DevicePartition::getNumComputeUnits() const {
  assert(getType() == PartitionEqually);
  return static_cast<unsigned>(Props[1]);
}

unsigned DeviceInfo::DevicePartition::getNumCounts() const {
  assert(getType() == PartitionByCounts);
  assert(Props.size() > 3);
  return Props.size() - 3;
}

unsigned DeviceInfo::DevicePartition::getCount(unsigned i) const {
  assert(getType() == PartitionByCounts);
  assert(i < getNumCounts());
  return Props[i + 1];
}

unsigned DeviceInfo::DevicePartition::getTotalComputeUnits() const {
  assert(getType() == PartitionByCounts);
  unsigned Sum = 0;
  for (unsigned i = 0; i != Props.size() - 3; ++i)
    Sum += Props[i + 1];
  return Sum;
}

bool DeviceInfo::isImageFormatSupported(const cl_image_format *Fmt) const {
  for (auto &CurFmt : GetSupportedImageFormats())
    if (!memcmp(&CurFmt, Fmt, sizeof(cl_image_format)))
      return true;
  return false;
}

//
// Device implementation.
//

Device::Device(DeviceType Ty, llvm::StringRef Name)
 : DeviceInfo(Ty), Parent(nullptr) {
  this->Name = Name;

  // Reference Counter intiially set to 1.
  Retain();
}

Device::Device(Device &Parent, const DevicePartition &Part)
 : DeviceInfo(Parent), Parent(&Parent), Partition(Part) {
  this->Name = Parent.GetName();  
  
  // Reference Counter intiially set to 1.
  Retain();
 
  // If the parent is a sub-device then increment its reference counter.
  if (Parent.IsSubDevice())
    Parent.Retain();
}

Device::~Device() {
  if(IsSubDevice()) {
    Device *Parent = GetParent();
    if(Parent->IsSubDevice())
      Parent->Release();
  }
}
