#include "opencrun/Core/MemoryObject.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/Device.h"
#include "opencrun/Util/OptionalOutput.h"

#include "llvm/ADT/STLExtras.h"

using namespace opencrun;

MemoryObject::~MemoryObject() {
  if (Parent)
    closeRegion(0, getSize());
}

static inline
bool checkIntersection(size_t X1, size_t L1, size_t X2, size_t L2) {
  return (X2 < X1 + L1) && (X1 < X2 + L2);
}

bool MemoryObject::MemMappingInfo::
isOverlappedWith(const MemMappingInfo &I) const {
  bool Overlap = true;
  for (unsigned i = 0; i != 3; ++i)
    Overlap &= checkIntersection(Offset[i], Size[i], I.Offset[i], I.Size[i]);

  return Overlap;
}

bool MemoryObject::addMapping(void *Ptr, const MemMappingInfo &Info) {
  if (!Ptr || Info.Flags == 0)
    return false;

  if (Buffer *Buf = llvm::dyn_cast<Buffer>(this))
    if (Buf->isSubBuffer()) {
      MemMappingInfo NewInfo(Info);
      NewInfo.Offset[0] += Buf->getOrigin();
      return Buf->getParent()->addMapping(Ptr, NewInfo);
    }

  if (Image *Img = llvm::dyn_cast<Image>(this))
    if (Img->getType() == Image::Image1D_Buffer)
      return Img->getBuffer()->addMapping(Ptr, Info);

  llvm::sys::ScopedLock Lock(ThisLock);
  if (Info.Flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION)) {
    if (MemMappings.count(Ptr))
      return false;

    for (auto &Pair : MemMappings)
      if (Pair.second.isOverlappedWith(Info))
        return false;
  }

  MemMappings.emplace(Ptr, Info);
  return true;
}

bool MemoryObject::removeMapping(void *Ptr) {
  if (Buffer *Buf = llvm::dyn_cast<Buffer>(this))
    if (Buf->isSubBuffer())
      return Buf->getParent()->removeMapping(Ptr);

  if (Image *Img = llvm::dyn_cast<Image>(this))
    if (Img->getType() == Image::Image1D_Buffer)
      return Img->getBuffer()->removeMapping(Ptr);

  llvm::sys::ScopedLock Lock(ThisLock);

  // We remove the first element with the given key value. In case of multiple
  // read mappings, we remove the first mapping record in the container despite
  // of its MappingInfo values. On the contrary, we can have at most one write
  // mapping, thus we do not have choices.
  auto I = MemMappings.find(Ptr);
  if (I == MemMappings.end())
    return false;

  MemMappings.erase(I);
  return true;
}

bool MemoryObject::isValidMapping(void *Ptr) const {
  llvm::sys::ScopedLock Lock(ThisLock);
  return MemMappings.count(Ptr) > 0;
}

unsigned MemoryObject::getNumMappings() const {
  llvm::sys::ScopedLock Lock(ThisLock);
  return MemMappings.size();
}

void *MemoryObject::acquireMappingAddrFor(const Device &Dev) const {
  if (getFlags() & UseHostPtr)
    return reinterpret_cast<uint8_t*>(getHostPtr());

  return getDescriptorFor(Dev).map();
}

void MemoryObject::releaseMappingAddrFor(const Device &Dev) const {
  if (getFlags() & UseHostPtr)
    return;

  getDescriptorFor(Dev).unmap();
}

bool MemoryObject::initForDevices() {
  std::vector<std::unique_ptr<MemoryDescriptor>> Descs;
  Descs.reserve(Ctx->device_size());

  for (auto *Dev : Ctx->devices())
    Descs.push_back(Dev->createMemoryDescriptor(*this));

  for (const auto &Desc : Descs)
    if (Desc == nullptr)
      return false;

  Descriptors.swap(Descs);

  if (getFlags() & CopyHostPtr)
    for (auto &Desc : Descriptors) {
      if (void *Ptr = Desc->map()) {
        if (Desc->aliasWithHostPtr())
          memcpy(Ptr, getHostPtr(), getSize());
        Desc->unmap();
        continue;
      }
      return false;
    }

  if (MCT)
    MCT->initialize();

  return true;
}

MemoryDescriptor &MemoryObject::getDescriptorFor(const Device &Dev) const {
  for (const auto &Desc : Descriptors)
    if (&Desc->getDevice() == &Dev)
      return *Desc;

  llvm_unreachable("looking for a device not in the context");
}

void MemoryObject::initializeMCT() {
  // Only a main memory-object has a region-coherency tracker.
  if (!Parent)
    MCT = llvm::make_unique<MemoryCoherencyTracker>(*this);
  else
    openRegion(0, getSize());
}

MemoryCoherencyTracker &MemoryObject::getMCT() const {
  if (Parent)
    return Parent->getMCT();

  return *MCT.get();
}

void MemoryObject::openRegion(size_t RegionOffset, size_t RegionSize) const {
  if (auto *Buf = llvm::dyn_cast<Buffer>(this))
    if (Buf->isSubBuffer())
      RegionOffset += Buf->getOrigin();

  getMCT().addRegion(RegionOffset, RegionSize);
}

void MemoryObject::closeRegion(size_t RegionOffset, size_t RegionSize) const {
  // FIXME: regions elimination not supported yet.
}

void MemoryObject::synchronizeFor(const Device &D, unsigned SM) const {
  size_t Offset = 0;
  if (auto *Buf = llvm::dyn_cast<Buffer>(this))
    if (Buf->isSubBuffer())
      Offset = Buf->getOrigin();

  getMCT().updateRegion(Offset, getSize(), SM, D);
}

void MemoryObject::synchronizeRegionFor(const Device &D, unsigned SM,
                                        size_t Offset, size_t Size) const {
  if (auto *Buf = llvm::dyn_cast<Buffer>(this))
    if (Buf->isSubBuffer())
      Offset += Buf->getOrigin();

  getMCT().updateRegion(Offset, Size, SM, D);
}

void *Buffer::computeHostPtr(Buffer &P, size_t O) {
  if (P.getFlags() & UseHostPtr)
    return reinterpret_cast<uint8_t*>(P.getHostPtr()) + O;

  return nullptr;
}

static unsigned computeImageNumChannels(Image::ChannelOrder CO) {
  switch (CO) {
  case Image::Channel_R:
  case Image::Channel_A:
  case Image::Channel_Rx:
  case Image::Channel_Intensity:
  case Image::Channel_Luminance:
    return 1;
  case Image::Channel_RG:
  case Image::Channel_RA:
  case Image::Channel_RGx:
    return 2;
  case Image::Channel_RGB:
  case Image::Channel_RGBx:
    return 3;
  case Image::Channel_RGBA:
  case Image::Channel_BGRA:
  case Image::Channel_ARGB:
    return 4;
  default:
    llvm_unreachable(0);
  }
}

static unsigned computeImageChannelSizeInBytes(Image::ChannelDataType CT) {
  switch (CT) {
  case Image::DataType_Signed_Int8:
  case Image::DataType_Unsigned_Int8:
  case Image::DataType_SNorm_Int8:
  case Image::DataType_UNorm_Int8:
    return 1;
  case Image::DataType_Signed_Int16:
  case Image::DataType_Unsigned_Int16:
  case Image::DataType_SNorm_Int16:
  case Image::DataType_UNorm_Int16:
  case Image::DataType_UNorm_Short_565:
  case Image::DataType_UNorm_Short_555:
  case Image::DataType_Half_Float:
    return 2;
  case Image::DataType_Signed_Int32:
  case Image::DataType_Unsigned_Int32:
  case Image::DataType_UNorm_Int_101010:
  case Image::DataType_Float:
    return 4;
  default:
    llvm_unreachable(0);
  }
}

static bool isPackedChannelType(Image::ChannelDataType CT) {
  switch (CT) {
  default:
    return false;
  case Image::DataType_UNorm_Short_565:
  case Image::DataType_UNorm_Short_555:
  case Image::DataType_UNorm_Int_101010:
    return true;
  }
}

void Image::computePixelFeatures() {
  NumChannels = computeImageNumChannels(ChOrder);
  ElementSize = computeImageChannelSizeInBytes(ChDataType);
  if (!isPackedChannelType(ChDataType))
    ElementSize *= NumChannels;
}

MemoryDescriptor::~MemoryDescriptor() {}

MemoryCoherencyTracker::MemoryCoherencyTracker(const MemoryObject &Obj)
 : Obj(Obj) {}

void MemoryCoherencyTracker::initialize() {
  auto C = MemoryChunk {0, Obj.getSize(), getNumDevices()};

  if (Obj.getFlags() & MemoryObject::UseHostPtr)
    updateHostValidity(C);

  if (Obj.getFlags() & MemoryObject::CopyHostPtr) {
    auto &Dev = getDeviceFromIndex(0);
    auto &DevDesc = Obj.getDescriptorFor(Dev);

    void *DstPtr = DevDesc.map();
    memcpy(DstPtr, Obj.getHostPtr(), C.Size);
    DevDesc.unmap();

    C.Validity.set(0);
  }

  llvm::sys::ScopedLock Lock(ThisLock);
  Partition.push_back(C);
}

unsigned MemoryCoherencyTracker::getNumDevices() const {
  // We have one extra slot to track the coherency for the host.
  return Obj.getContext().device_size() + 1;
}

unsigned MemoryCoherencyTracker::getDeviceIndex(const Device &D) const {
  Context &Ctx = Obj.getContext();
  auto I = std::find(Ctx.device_begin(), Ctx.device_end(), &D);
  assert(I != Ctx.device_end() && "Device not in context");
  return std::distance(Ctx.device_begin(), I);
}

unsigned MemoryCoherencyTracker::getHostIndex() const {
  return getNumDevices() - 1;
}

const Device &MemoryCoherencyTracker::getDeviceFromIndex(unsigned Idx) const {
  assert(Idx < Obj.getContext().device_size());
  return **(Obj.getContext().device_begin() + Idx);
}

MemoryCoherencyTracker::iterator
MemoryCoherencyTracker::findChunk(iterator I, iterator E, size_t Offset) {
  I = std::lower_bound(I, E, Offset);
  if (I != E && I->Offset == Offset)
    return I;
  return std::prev(I);
}

MemoryCoherencyTracker::iterator_range
MemoryCoherencyTracker::findRangeFor(size_t Offset, size_t Size) {
  auto I = findChunk(Partition.begin(), Partition.end(), Offset);
  auto E = findChunk(I, Partition.end(), Offset + Size);
  return { I, std::next(E) };
}

MemoryCoherencyTracker::iterator
MemoryCoherencyTracker::splitChunk(iterator I, size_t Offset) {
  if (I->Offset == Offset)
    return I;

  assert(I->Offset < Offset && I->Offset + I->Size > Offset);

  size_t NewSize = I->Offset + I->Size - Offset;
  I->Size -= NewSize;
  I = Partition.insert(std::next(I), *I);
  I->Offset = Offset;
  I->Size = NewSize;

  return I;
}

void MemoryCoherencyTracker::addRegion(size_t Offset, size_t Size) {
  llvm::sys::ScopedLock Lock(ThisLock);

  auto C = findChunk(Partition.begin(), Partition.end(), Offset);
  C = splitChunk(C, Offset);
  C = findChunk(C, Partition.end(), Offset + Size);
  C = splitChunk(C, Offset + Size);
}

void MemoryCoherencyTracker::transferToDevice(const Device &D,
                                              const MemoryChunk &C) {
  if (C.Validity.none())
    return;

  // Check whether the region is already valid for this device.
  if (C.Validity.test(getDeviceIndex(D)))
    return;

  auto &DevDesc = Obj.getDescriptorFor(D);
  void *DstPtr = DevDesc.map();
  if (C.Validity.test(getHostIndex())) {
    // Prefer sync from host.
    assert(Obj.getFlags() & MemoryObject::UseHostPtr);
    void *SrcPtr = Obj.getHostPtr();

    if (!DevDesc.aliasWithHostPtr())
      memcpy(reinterpret_cast<uint8_t*>(DstPtr) + C.Offset,
             reinterpret_cast<uint8_t*>(SrcPtr) + C.Offset,
             C.Size);
  } else {
    // Use first device with a valid state.
    const Device &SrcDev = getDeviceFromIndex(C.Validity.find_first());
    auto &SrcDesc = Obj.getDescriptorFor(SrcDev);

    void *SrcPtr = SrcDesc.map();
    memcpy(reinterpret_cast<uint8_t*>(DstPtr) + C.Offset,
           reinterpret_cast<uint8_t*>(SrcPtr) + C.Offset,
           C.Size);
    SrcDesc.unmap();
  }
  DevDesc.unmap();
}

void MemoryCoherencyTracker::transferToHost(const Device &D,
                                            const MemoryChunk &C) {
  if (C.Validity.none())
    return;

  // Check whether the region is already valid for the host.
  if (C.Validity.test(getHostIndex()))
    return;

  // Use first device with a valid state.
  const Device &SrcDev = getDeviceFromIndex(C.Validity.find_first());
  auto &SrcDesc = Obj.getDescriptorFor(SrcDev);

  void *DstPtr = Obj.getHostPtr();
  void *SrcPtr = SrcDesc.map();
  assert(DstPtr != SrcPtr);
  memcpy(reinterpret_cast<uint8_t*>(DstPtr) + C.Offset,
         reinterpret_cast<uint8_t*>(SrcPtr) + C.Offset,
         C.Size);
  SrcDesc.unmap();
}

void MemoryCoherencyTracker::updateHostValidity(MemoryChunk &C) {
  C.Validity.set(getHostIndex());

  // If we set the host to valide we need to update the validity for all
  // the devices aliasing with the host.
  for (unsigned Idx = 0; Idx != getHostIndex(); ++Idx) {
    auto &Desc = Obj.getDescriptorFor(getDeviceFromIndex(Idx));
    if (Desc.aliasWithHostPtr())
      C.Validity.set(Idx);
  }
}


std::vector<MemoryCoherencyTracker::MemoryChunk>
MemoryCoherencyTracker::computeChunksToTransfer(size_t Offset, size_t Size,
                                                unsigned Mode) {
  using SM = MemoryObject::SynchronizeMode;

  std::vector<MemoryChunk> Chunks;

  auto Range = findRangeFor(Offset, Size);

  if (Mode & SM::SyncRead) {
    // Read access implies to transfer all the chunks in the range.
    Chunks.assign(Range.begin(), Range.end());
    return Chunks;
  }

  assert(Mode & SM::SyncWrite);

  // Write-only access to a given region still requires to transfer data
  // outside the queried region but still in the range of chunks we have.
  // This is mandatory for correctness as we keep the partition generated by
  // sub-objects only.

  const auto &FirstChunk = *Range.begin();
  if (FirstChunk.Offset < Offset) {
    Chunks.push_back(FirstChunk);
    Chunks.back().Size = Offset - FirstChunk.Offset;
  }

  const auto &LastChunk = *std::prev(Range.end());
  if (LastChunk.Offset + LastChunk.Size > Offset + Size) {
    Chunks.push_back(LastChunk);
    Chunks.back().Offset = Offset + Size;
    Chunks.back().Size = (LastChunk.Offset + LastChunk.Size) - (Offset + Size);
  }

  return Chunks;
}

void MemoryCoherencyTracker::updateRegion(size_t Offset, size_t Size,
                                          unsigned Mode, const Device &D) {
  // The update of a memory object region is split in three steps:
  // 1. compute the chunks of memory we need to transfer.
  // 2. for each chunk copy the content from a valid device/host memory.
  // 3. update the validity bits for all the memory chunks in the partition
  // that contain the queried region.
  //
  // It is correct and safe to perform step 1 and 3 in two different atomic
  // transactions since a reader-writer or writer-writer on the same region may
  // leads to undefined behavior by standard. Thus, a writer that invalidates
  // all the caches is assumed to be exclusive.
  // This allow to perform the actual memory transfer *outside* the critical
  // section.
  //
  // NOTE: the initial implementation uses standard locks. We need to check if
  // we can improve the scalability employing a reader-writer lock instead.

  using SM = MemoryObject::SynchronizeMode;

  assert(Mode && (Mode & ~(SM::SyncMaskAll)) == 0);

  bool DeviceUpdate = !((Mode & SM::SyncMap) &&
                        (Obj.getFlags() & MemoryObject::UseHostPtr));

  std::vector<MemoryChunk> Chunks;

  // Step 1: compute the chunks of memory we need to transfer.
  {
    llvm::sys::ScopedLock Lock(ThisLock);

    Chunks = computeChunksToTransfer(Offset, Size, Mode);
  }

  // Step 2: do actual memory transfers considering the validity bits inherited
  // by the partition.
  for (const auto &C : Chunks)
    if (DeviceUpdate)
      transferToDevice(D, C);
    else
      transferToHost(D, C);

  // Step 3: update validity bits for the memory chunks in the partition that
  // contain the queried region.
  {
    llvm::sys::ScopedLock Lock(ThisLock);

    auto Range = findRangeFor(Offset, Size);
    assert(Range.begin() != Range.end());

    auto &DevDesc = Obj.getDescriptorFor(D);
    unsigned DevIndex = getDeviceIndex(D);

    for (auto &C : Range) {
      if (Mode & SM::SyncWrite)
        C.Validity.reset();

      if (DeviceUpdate) {
        C.Validity.set(DevIndex);
        if (DevDesc.aliasWithHostPtr())
          updateHostValidity(C);
      } else {
        updateHostValidity(C);
      }
    }
  }
}

//===----------------------------------------------------------------------===//
// MemoryObjectBuilder implementation
//===----------------------------------------------------------------------===//

MemoryObjectBuilder::MemoryObjectBuilder(Kind K)
 : ObjKind(K), Ctx(nullptr), Flags(0), HostPtr(0), Err(CL_SUCCESS) {
  switch (ObjKind) {
  case K_Buffer:
    memset(&Buf, 0, sizeof(Buf));
    break;
  case K_SubBuffer:
    memset(&SubBuf, 0, sizeof(SubBuf));
    break;
  case K_Image:
    memset(&Img, 0, sizeof(Img));
    break;
  }
}

void MemoryObjectBuilder::error(cl_int Err, llvm::StringRef Msg) {
  Ctx->ReportDiagnostic(Msg.str().c_str());
  this->Err = Err;
}

void MemoryObjectBuilder::setSubBufferFlags(cl_mem_flags Flags) {
  assert(ObjKind == K_SubBuffer);

  const cl_mem_flags AllFlags = MemoryObject::HostPtrModeMask |
                                MemoryObject::AccessProtectionMask |
                                MemoryObject::HostAccessProtectionMask;

  if (Flags & ~AllFlags)
    return error(CL_INVALID_VALUE, "illegal flags");

  if (Flags == 0) {
    Flags = SubBuf.Parent->getFlags();
  } else {
    bool RO = Flags & CL_MEM_READ_ONLY;
    bool WO = Flags & CL_MEM_WRITE_ONLY;
    bool RW = Flags & CL_MEM_READ_WRITE;

    if (RO + WO + RW > 1)
      return error(CL_INVALID_VALUE,
                   "multiple access protection flags not allowed");

    if (RO + WO + RW == 0)
      Flags = SubBuf.Parent->getFlags() & (CL_MEM_READ_ONLY |
                                           CL_MEM_WRITE_ONLY |
                                           CL_MEM_READ_WRITE);

    bool HRO = Flags & CL_MEM_HOST_READ_ONLY;
    bool HWO = Flags & CL_MEM_HOST_WRITE_ONLY;
    bool HNA = Flags & CL_MEM_HOST_NO_ACCESS;

    if (HRO + HWO + HNA > 0)
      return error(CL_INVALID_VALUE,
                   "multiple host access protection flags not allowed");

    bool UHP = Flags & CL_MEM_USE_HOST_PTR;
    bool AHP = Flags & CL_MEM_ALLOC_HOST_PTR;
    bool CHP = Flags & CL_MEM_COPY_HOST_PTR;

    if (UHP + AHP + CHP != 0)
      return error(CL_INVALID_VALUE,
                   "memory object storage specifiers not allowed");

    Flags = SubBuf.Parent->getFlags() & (CL_MEM_USE_HOST_PTR |
                                         CL_MEM_ALLOC_HOST_PTR |
                                         CL_MEM_COPY_HOST_PTR);
  }
  this->Flags = Flags;
}

void MemoryObjectBuilder::setGenericFlags(cl_mem_flags Flags, void *HostPtr) {
  assert(ObjKind != K_SubBuffer);
  const cl_mem_flags AllFlags = MemoryObject::HostPtrModeMask |
                                MemoryObject::AccessProtectionMask |
                                MemoryObject::HostAccessProtectionMask;

  if (Flags & ~AllFlags)
    return error(CL_INVALID_VALUE, "illegal flags");

  if (Flags == 0) {
    Flags = CL_MEM_READ_WRITE;
  } else {
    bool RO = Flags & CL_MEM_READ_ONLY;
    bool WO = Flags & CL_MEM_WRITE_ONLY;
    bool RW = Flags & CL_MEM_READ_WRITE;

    if (RO + WO + RW > 1)
      return error(CL_INVALID_VALUE,
                   "multiple access protection flags not allowed");

    bool HRO = Flags & CL_MEM_HOST_READ_ONLY;
    bool HWO = Flags & CL_MEM_HOST_WRITE_ONLY;
    bool HNA = Flags & CL_MEM_HOST_NO_ACCESS;

    if (HRO + HWO + HNA > 1)
      return error(CL_INVALID_VALUE,
                   "multiple host access protection flags not allowed");

    bool UHP = Flags & CL_MEM_USE_HOST_PTR;
    bool AHP = Flags & CL_MEM_ALLOC_HOST_PTR;
    bool CHP = Flags & CL_MEM_COPY_HOST_PTR;

    if ((UHP | CHP) && !HostPtr)
      return error(CL_INVALID_HOST_PTR,
                   "missed pointer to initialization data");

    if ((UHP + AHP > 1) || (UHP + CHP > 1))
      return error(CL_INVALID_VALUE,
                   "multiple memory object storage specifiers not allowed");
  }

  // Image1D_Buffer additional checks.
  if (ObjKind == K_Image && Img.Type == Image::Image1D_Buffer) {
    uint16_t ImgRORW = Flags & Image::CanReadMask;
    uint16_t ImgWORW = Flags & Image::CanWriteMask;
    uint16_t BufWO = Img.Buf->getFlags() & Image::WriteOnly;
    uint16_t BufRO = Img.Buf->getFlags() & Image::ReadOnly;

    if ((ImgRORW && BufWO) || (ImgWORW && BufRO))
      return error(CL_INVALID_VALUE, "access mode incompatible with buffer");

    uint16_t ImgHostRO = Flags & Image::HostReadOnly;
    uint16_t ImgHostWO = Flags & Image::HostWriteOnly;
    uint16_t ImgHostNA = Flags & Image::HostNoAccess;
    uint16_t BufHostWO = Img.Buf->getFlags() & Image::HostWriteOnly;
    uint16_t BufHostRO = Img.Buf->getFlags() & Image::HostReadOnly;
    uint16_t BufHostWORO = BufHostWO | BufHostRO;

    if ((ImgHostRO && BufHostWO) || (ImgHostWO && BufHostRO) ||
        (ImgHostNA && BufHostWORO))
      return error(CL_INVALID_VALUE,
                   "host access mode incompatible with buffer");

    if (Flags & Image::HostPtrModeMask)
      return error(CL_INVALID_VALUE, "host-ptr mode shall not be specified");

    // Inherit host-ptr mode from buffer.
    Flags |= (Img.Buf->getFlags() & Image::HostPtrModeMask);

    // Inherit access protection.
    if (!(Flags & Image::AccessProtectionMask))
      Flags |= (Img.Buf->getFlags() & Image::AccessProtectionMask);

    // Inherit host access protection.
    if (!(Flags & Image::HostAccessProtectionMask))
      Flags |= (Img.Buf->getFlags() & Image::HostAccessProtectionMask);
  }

  this->Flags = Flags;
  this->HostPtr = HostPtr;
}

void MemoryObjectBuilder::setContext(cl_context Ctx) {
  if (!Ctx)
    return error(CL_INVALID_CONTEXT, "invalid context");

  this->Ctx = llvm::cast<Context>(Ctx);
}

void MemoryObjectBuilder::setSubBufferParams(cl_mem Buf,
                                             cl_buffer_create_type BCT,
                                             const void *BCI) {
  if (!Buf || !llvm::isa<Buffer>(Buf))
    return error(CL_INVALID_MEM_OBJECT, "invalid memory object");

  if (llvm::cast<Buffer>(Buf)->isSubBuffer())
    return error(CL_INVALID_MEM_OBJECT,
                 "cannot create a sub-buffer of sub-buffer");

  SubBuf.Parent = llvm::cast<Buffer>(Buf);

  Ctx = &SubBuf.Parent->getContext();

  if (BCT != CL_BUFFER_CREATE_TYPE_REGION)
    return error(CL_INVALID_VALUE, "invalid buffer_create_type value");

  if (!BCI)
    return error(CL_INVALID_VALUE, "null buffer_create_info value");

  const cl_buffer_region *BR = reinterpret_cast<const cl_buffer_region*>(BCI);
  if (!BR->size)
    return error(CL_INVALID_BUFFER_SIZE, "empty sub-buffer");

  if (BR->origin + BR->size > SubBuf.Parent->getSize())
    return error(CL_INVALID_VALUE, "out of bounds sub-buffer");
}

void MemoryObjectBuilder::setBufferSize(size_t Size) {
  if (!Size)
    return error(CL_INVALID_BUFFER_SIZE, "empty buffer");

  Buf.Size = Size;
}

void MemoryObjectBuilder::setImage(const cl_image_format *Fmt,
                                   const cl_image_desc *Desc) {
  if (!Fmt)
    return error(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR, "image format is null");

  switch (Fmt->image_channel_order) {
  case CL_R:
  case CL_A:
  case CL_Rx:
  case CL_RG:
  case CL_RA:
  case CL_RGx:
  case CL_RGB:
  case CL_RGBx:
  case CL_RGBA:
  case CL_ARGB:
  case CL_BGRA:
  case CL_INTENSITY:
  case CL_LUMINANCE:
    break;
  default:
    return error(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
                 "invalid image channel order");
  }

  switch (Fmt->image_channel_data_type) {
  case CL_SNORM_INT8:
  case CL_UNORM_INT8:
  case CL_SIGNED_INT8:
  case CL_UNSIGNED_INT8:
  case CL_SNORM_INT16:
  case CL_UNORM_INT16:
  case CL_UNORM_SHORT_565:
  case CL_UNORM_SHORT_555:
  case CL_SIGNED_INT16:
  case CL_UNSIGNED_INT16:
  case CL_HALF_FLOAT:
  case CL_UNORM_INT_101010:
  case CL_SIGNED_INT32:
  case CL_UNSIGNED_INT32:
  case CL_FLOAT:
    break;
  default:
    return error(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
                 "invalid image channel data type");
  }

  switch (Fmt->image_channel_order) {
  case CL_RGB:
  case CL_RGBx:
    switch (Fmt->image_channel_data_type) {
    case CL_UNORM_SHORT_565:
    case CL_UNORM_SHORT_555:
    case CL_UNORM_INT_101010:
      break;
    default:
      return error(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
                   "invalid image channel order-data type pair");
    }
    break;
  case CL_ARGB:
  case CL_BGRA:
    switch (Fmt->image_channel_data_type) {
    case CL_UNORM_INT8:
    case CL_SNORM_INT8:
    case CL_SIGNED_INT8:
    case CL_UNSIGNED_INT8:
      break;
    default:
      return error(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
                   "invalid image channel order-data type pair");
    }
    break;
  case CL_INTENSITY:
  case CL_LUMINANCE:
    switch (Fmt->image_channel_data_type) {
    case CL_UNORM_INT8:
    case CL_UNORM_INT16:
    case CL_SNORM_INT8:
    case CL_SNORM_INT16:
    case CL_HALF_FLOAT:
    case CL_FLOAT:
      break;
    default:
      return error(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
                   "invalid image channel order-data type pair");
    }
    break;
  default:
    break;
  }

  Img.ChOrder = (Image::ChannelOrder)(Fmt->image_channel_order);
  Img.ChDataType = (Image::ChannelDataType)(Fmt->image_channel_data_type);

  if (!Desc)
    return error(CL_INVALID_IMAGE_DESCRIPTOR, "image descriptor is null");

  if (Desc->num_mip_levels)
    return error(CL_INVALID_IMAGE_DESCRIPTOR, "non zero mip levels");

  if (Desc->num_samples)
    return error(CL_INVALID_IMAGE_DESCRIPTOR, "non zero samples");

  size_t MMImg2DWidth = ~0;
  size_t MMImg2DHeight = ~0;
  size_t MMImg3DWidth = ~0;
  size_t MMImg3DHeight = ~0;
  size_t MMImg3DDepth = ~0;
  size_t MMImgBufSize = ~0;
  size_t MMImgArraySize = ~0;
  size_t Supported = 0;
  for (auto I = Ctx->device_begin(), E = Ctx->device_end(); I != E; ++I) {
    auto *D = *I;
    if (D->isImageFormatSupported(Fmt)) {
      MMImg2DWidth = std::min(D->GetImage2DMaxWidth(), MMImg2DWidth);
      MMImg2DHeight = std::min(D->GetImage2DMaxHeight(), MMImg2DHeight);
      MMImg3DWidth = std::min(D->GetImage3DMaxWidth(), MMImg3DWidth);
      MMImg3DHeight = std::min(D->GetImage3DMaxHeight(), MMImg3DHeight);
      MMImg3DDepth = std::min(D->GetImage3DMaxHeight(), MMImg3DDepth);
      MMImgBufSize = std::min(D->GetImageMaxBufferSize(), MMImgBufSize);
      MMImgArraySize = std::min(D->GetImageMaxArraySize(), MMImgArraySize);

      ++Supported;
    }
  }

  if (!Supported)
    return error(CL_IMAGE_FORMAT_NOT_SUPPORTED,
                 "specified image format unsupported");

  unsigned ElementSize = computeImageChannelSizeInBytes(Img.ChDataType);
  if (!isPackedChannelType(Img.ChDataType))
    ElementSize *= computeImageNumChannels(Img.ChOrder);

  switch (Desc->image_type) {
  case CL_MEM_OBJECT_IMAGE1D:
  case CL_MEM_OBJECT_IMAGE1D_BUFFER:
  case CL_MEM_OBJECT_IMAGE1D_ARRAY: {
    if (Desc->image_width == 0)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "image width is zero");

    if (Desc->image_width >= MMImgBufSize / ElementSize)
      return error(CL_INVALID_IMAGE_SIZE,
                   "image width bigger than image buffer size");

    bool BufferImage = Desc->image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER;

    if (BufferImage && Desc->buffer == nullptr)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "buffer object is null");

    if (!BufferImage && Desc->buffer != nullptr)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "buffer object is not null");

    Img.Buf = llvm::cast_or_null<Buffer>(Desc->buffer);

    bool ArrayImage = Desc->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY;

    if (ArrayImage && Desc->image_array_size == 0)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "image array size is zero");

    if (ArrayImage && Desc->image_array_size >= MMImgArraySize)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "invalid image array size");

    Img.Desc.Width = Desc->image_width;
    // FIXME: these values should be zero!
    Img.Desc.Height = 1;
    Img.Desc.Depth = 1;
    Img.Desc.ArraySize = ArrayImage ? Desc->image_array_size : 1;
    break;
  }
  case CL_MEM_OBJECT_IMAGE2D:
  case CL_MEM_OBJECT_IMAGE2D_ARRAY: {
    if (Desc->image_width == 0)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "image width is zero");

    if (Desc->image_width >= MMImg2DWidth)
      return error(CL_INVALID_IMAGE_SIZE,
                   "image width bigger than image buffer size");

    if (Desc->image_height == 0)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "image height is zero");

    if (Desc->image_height >= MMImg2DHeight)
      return error(CL_INVALID_IMAGE_SIZE,
                   "image height bigger than image buffer size");

    if (Desc->buffer != nullptr)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "buffer object is not null");

    bool ArrayImage = Desc->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY;

    if (ArrayImage && Desc->image_array_size == 0)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "image array size is zero");

    if (ArrayImage && Desc->image_array_size >= MMImgArraySize)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "invalid image array size");

    Img.Desc.Width = Desc->image_width;
    Img.Desc.Height = Desc->image_height;
    // FIXME: these values should be zero!
    Img.Desc.Depth = 1;
    Img.Desc.ArraySize = ArrayImage ? Desc->image_array_size : 1;
    break;
  }
  case CL_MEM_OBJECT_IMAGE3D: {
    if (Desc->image_width == 0)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "image width is zero");

    if (Desc->image_width >= MMImg3DWidth)
      return error(CL_INVALID_IMAGE_SIZE, "invalid image width");

    if (Desc->image_height == 0)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "image height is zero");

    if (Desc->image_height >= MMImg3DHeight)
      return error(CL_INVALID_IMAGE_SIZE, "invalid image height");

    if (Desc->image_depth == 0)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "image depth is zero");

    if (Desc->image_depth >= MMImg3DDepth)
      return error(CL_INVALID_IMAGE_SIZE, "invalid image depth");

    if (Desc->buffer != nullptr)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "buffer object is not null");

    Img.Desc.Width = Desc->image_width;
    Img.Desc.Height = Desc->image_height;
    Img.Desc.Depth = Desc->image_depth;
    // FIXME: these values should be zero!
    Img.Desc.ArraySize = 1;
    break;
  }
  default:
    return error(CL_INVALID_IMAGE_DESCRIPTOR, "invalid image type");
  }

  Img.Type = static_cast<Image::Type>(Desc->image_type);

  bool RequiresSlicePitch = Img.Type == Image::Image1D_Array ||
                            Img.Type == Image::Image2D_Array ||
                            Img.Type == Image::Image3D;

  if (!HostPtr) {
    if (Desc->image_row_pitch != 0)
      return error(CL_INVALID_IMAGE_DESCRIPTOR,
                   "host-ptr is null but row pitch is not zero");

    if (RequiresSlicePitch && Desc->image_slice_pitch != 0)
      return error(CL_INVALID_IMAGE_DESCRIPTOR,
                   "host-ptr is null but slice pitch is not zero");

    Img.Desc.RowPitch = Img.Desc.Width * ElementSize;

    if (RequiresSlicePitch)
      Img.Desc.SlicePitch = Img.Desc.RowPitch * Img.Desc.Height;

  } else {
    size_t MinRowPitch = Img.Desc.Width * ElementSize;
    Img.Desc.RowPitch = Desc->image_row_pitch ? Desc->image_row_pitch
                                              : MinRowPitch;

    if (Img.Desc.RowPitch < MinRowPitch || Img.Desc.RowPitch % ElementSize != 0)
      return error(CL_INVALID_IMAGE_DESCRIPTOR, "invalid row pitch value");

    if (RequiresSlicePitch) {
      size_t MinSlicePitch = Img.Desc.RowPitch * Img.Desc.Height;
      Img.Desc.SlicePitch = Desc->image_slice_pitch ? Desc->image_slice_pitch
                                                    : MinSlicePitch;

      if (Img.Desc.SlicePitch < MinSlicePitch ||
          Img.Desc.SlicePitch % Img.Desc.RowPitch != 0)
        return error(CL_INVALID_IMAGE_DESCRIPTOR, "invalid row pitch value");
    }
  }

  switch (Desc->image_type) {
  case CL_MEM_OBJECT_IMAGE1D:
  case CL_MEM_OBJECT_IMAGE1D_BUFFER:
    Img.Size = Img.Desc.RowPitch;
    break;
  case CL_MEM_OBJECT_IMAGE1D_ARRAY:
    Img.Size = Img.Desc.RowPitch * Img.Desc.ArraySize;
    break;
  case CL_MEM_OBJECT_IMAGE2D:
    Img.Size = Img.Desc.RowPitch * Img.Desc.Height;
    break;
  case CL_MEM_OBJECT_IMAGE2D_ARRAY:
    Img.Size = Img.Desc.RowPitch * Img.Desc.SlicePitch * Img.Desc.ArraySize;
    break;
  case CL_MEM_OBJECT_IMAGE3D:
    Img.Size = Img.Desc.RowPitch * Img.Desc.SlicePitch * Img.Desc.Depth;
    break;
  default:
    llvm_unreachable(0);
  }
}

std::unique_ptr<MemoryObject> MemoryObjectBuilder::create(cl_int *ErrCode) {
  OptionalOutput<cl_int> RetErrCode(Err, ErrCode);

  if (Err)
    return nullptr;

  std::unique_ptr<MemoryObject> Obj;

  switch (ObjKind) {
  case K_Buffer:
    Obj.reset(new Buffer(*Ctx, Buf.Size, HostPtr, Flags));
    break;
  case K_SubBuffer:
    Obj.reset(new Buffer(*Ctx, *SubBuf.Parent, SubBuf.Origin, SubBuf.Size,
                         Flags));
    break;
  case K_Image:
    Obj.reset(new Image(*Ctx, Img.Size, HostPtr, Flags, Img.Type, Img.ChOrder,
                        Img.ChDataType, Img.Desc, Img.Buf));
    break;
  }

  if (!Obj || !Obj->initForDevices())
    error(CL_OUT_OF_HOST_MEMORY, "out of host memory");

  return Obj;
}
