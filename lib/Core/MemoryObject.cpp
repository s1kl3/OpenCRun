#include "opencrun/Core/MemoryObject.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/Device.h"
#include "opencrun/Util/OptionalOutput.h"

using namespace opencrun;

MemoryObject::~MemoryObject() {
  Ctx->destroyMemoryObject(*this);
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
    Obj = Ctx->createBuffer(Buf.Size, HostPtr, Flags);
    break;
  case K_SubBuffer:
    Obj = Ctx->createSubBuffer(*SubBuf.Parent, SubBuf.Origin, SubBuf.Size, Flags);
    break;
  case K_Image:
    Obj = Ctx->createImage(Img.Size, HostPtr, Flags, Img.Type,
                           Img.ChOrder, Img.ChDataType, Img.Desc, Img.Buf);
    break;
  }

  if (!Obj)
    error(CL_OUT_OF_HOST_MEMORY, "out of host memory");

  return Obj;
}
