
#include "opencrun/Core/MemoryObj.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/Device.h"

#include <algorithm>

using namespace opencrun;

//
// MemoryObj implementation.
//

MemoryObj::~MemoryObj() {
  Ctx->DestroyMemoryObj(*this);
}

bool MemoryObj::MappingInfo::CheckOverlap(const MappingInfo &MapInfo) {
  if(Origin && Region) {
    // Image mapping.
    const size_t MapInfo_Min[] = { MapInfo.Origin[0],
                                   MapInfo.Origin[1],
                                   MapInfo.Origin[2] };

    const size_t MapInfo_Max[] = { MapInfo.Origin[0] + MapInfo.Region[0],
                                   MapInfo.Origin[1] + MapInfo.Region[1],
                                   MapInfo.Origin[2] + MapInfo.Region[2] };

    const size_t Min[] = { Origin[0], Origin[1], Origin[2] };

    const size_t Max[] = { Origin[0] + Region[0],
                           Origin[1] + Region[1],
                           Origin[2] + Region[2] };

    bool Overlap = true;
    for(unsigned I = 0; I < 3 && !Overlap; ++I) {
      Overlap = Overlap && (MapInfo_Min[I] < Max[I])
        && (MapInfo_Max[I] > Min[I]);
    }

    if(Overlap)
      return true;

  } else {
    // Buffer or 1D Image buffer mapping.
    if((MapInfo.Offset <= (Offset + Size))
        && ((MapInfo.Offset + MapInfo.Size) >=  Offset))
      return true;
  }

  return false;
}

bool MemoryObj::AddNewMapping(void *MapBuf, const MemoryObj::MappingInfo &MapInfo) {
  if(!MapBuf)
    return false;
    
  if(MapInfo.Size == 0 && MapInfo.Origin == NULL && MapInfo.Region == NULL && 
      (MapInfo.Region[0] == 0 || MapInfo.Region[1] == 0 || MapInfo.Region[2] == 0))
    return false;
    
  if(MapInfo.MapFlags == 0)
    return false;
  
  llvm::sys::ScopedLock Lock(ThisLock);

  if(llvm::isa<Buffer>(this)) {

    if(MapInfo.MapFlags & CL_MAP_READ)
      Maps.insert(std::pair<void *, MappingInfo>(MapBuf, MapInfo));

    else if(MapInfo.MapFlags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION)) {
      if(Maps.count(MapBuf))
        return false;

      for(maps_iterator I = Maps.begin(), E = Maps.end(); I != E; ++I)
        if(I->second.CheckOverlap(MapInfo)) return false;

      Maps.insert(std::pair<void *, MappingInfo>(MapBuf, MapInfo));
    }

  } else if(Image *Img = llvm::dyn_cast<Image>(this)) {

    if(MapInfo.MapFlags & CL_MAP_READ) {
      if(Img->GetImageType() == Image::Image1D_Buffer) {
        Buffer *Buf = Img->GetBuffer();

        // Mapping is added to the associated buffer, checking
        // the image 1D buffer mapping doesn't overlap with
        // any other mapping involving the buffer.
        return Buf->AddNewMapping(MapBuf, MapInfo);
      }
      else 
        Maps.insert(std::pair<void *, MappingInfo>(MapBuf, MapInfo));
    } else if(MapInfo.MapFlags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION)) {
      if(Img->GetImageType() == Image::Image1D_Buffer) {
        Buffer *Buf = Img->GetBuffer();

        // Mapping is added to the associated buffer, checking
        // the image 1D buffer mapping doesn't overlap with
        // any other mapping involving the buffer.
        return Buf->AddNewMapping(MapBuf, MapInfo);
      }
      else {
        if(Maps.count(MapBuf))
          return false;

        for(maps_iterator I = Maps.begin(), E = Maps.end(); I != E; ++I)
          if(I->second.CheckOverlap(MapInfo)) return false;

        Maps.insert(std::pair<void *, MappingInfo>(MapBuf, MapInfo));
      }
    }

  }
  
  return true;
}

bool MemoryObj::RemoveMapping(void *MapBuf) {
  llvm::sys::ScopedLock Lock(ThisLock);

  if(Image *Img = llvm::dyn_cast<Image>(this)) {
    if(Img->GetImageType() == Image::Image1D_Buffer) {
      Buffer *Buf = Img->GetBuffer();

      return Buf->RemoveMapping(MapBuf);
    }
  }
  
  // We remove the first found element with the given key value. In case of
  // read mappings with the same host address, this will remove the first
  // mapping record in the container (despite of its MappingInfo values). 
  // For write mappings this will remove the only existing record in the 
  // container.
  maps_iterator it = Maps.find(MapBuf);
  if(it == Maps.end())
    return false;
    
  Maps.erase(it);
  
  return true;
}													

bool MemoryObj::IsValidMappingPtr(void *MapBuf) {
  llvm::sys::ScopedLock Lock(ThisLock);
  
  return Maps.count(MapBuf) > 0 ? true : false;
}

MemoryObj::MappingInfo *MemoryObj::GetMappingInfo(void *MapBuf) {
  maps_iterator it = Maps.find(MapBuf);
  if(it == Maps.end())
    return NULL;
    
  return &(it->second);
}

//
// MemoryObjBuilder implementation.
//

#define RETURN_WITH_ERROR(VAR) \
  {                            \
  if(VAR)                      \
    *VAR = this->ErrCode;      \
  return NULL;                 \
  }

MemoryObjBuilder &MemoryObjBuilder::SetUseHostMemory(bool Enabled, void* HostPtr) {
  if(Enabled) {
    if(!HostPtr)
      return NotifyError(CL_INVALID_HOST_PTR, "missing host storage pointer");

    if((HostPtrMode == MemoryObj::AllocHostPtr) || (HostPtrMode == MemoryObj::CopyHostPtr))
      return NotifyError(CL_INVALID_VALUE,
                         "multiple memory object storage specifiers not allowed");

    HostPtrMode = MemoryObj::UseHostPtr;
    this->HostPtr = HostPtr;
  }

  return *this;
}

MemoryObjBuilder &MemoryObjBuilder::SetAllocHostMemory(bool Enabled) {
  if(Enabled) {
    if(HostPtrMode == MemoryObj::UseHostPtr)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple memory object storage specifiers not allowed");

    HostPtrMode = MemoryObj::AllocHostPtr;
  }

  return *this;
}

MemoryObjBuilder &MemoryObjBuilder::SetCopyHostMemory(bool Enabled, void* HostPtr) {
  if(Enabled) {
    if(!HostPtr)
      return NotifyError(CL_INVALID_HOST_PTR,
                         "missed pointer to initialization data");

    if(HostPtrMode == MemoryObj::UseHostPtr)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple memory object storage specifiers not allowed");

    HostPtrMode = MemoryObj::CopyHostPtr;
    this->HostPtr = HostPtr;
  }

  return *this;
}

MemoryObjBuilder &MemoryObjBuilder::SetReadWrite(bool Enabled) {
  if(Enabled) {
    if(AccessProt == MemoryObj::ReadOnly || AccessProt == MemoryObj::WriteOnly)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple access protection flags not allowed");

    AccessProt = MemoryObj::ReadWrite;
  }

  return *this;
}

MemoryObjBuilder &MemoryObjBuilder::SetWriteOnly(bool Enabled) {
  if(Enabled) {
    if(AccessProt == MemoryObj::ReadWrite || AccessProt == MemoryObj::ReadOnly)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple access protection flags not allowed");

    AccessProt = MemoryObj::WriteOnly;
  }

  return *this;
}

MemoryObjBuilder &MemoryObjBuilder::SetReadOnly(bool Enabled) {
  if(Enabled) {
    if(AccessProt == MemoryObj::ReadWrite || AccessProt == MemoryObj::WriteOnly)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple access protection flags not allowed");

    AccessProt = MemoryObj::ReadOnly;
  }

  return *this;
}

MemoryObjBuilder &MemoryObjBuilder::SetHostWriteOnly(bool Enabled) {
  if(Enabled) {
    if(HostAccessProt == MemoryObj::HostReadOnly || 
        HostAccessProt == MemoryObj::HostNoAccess)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple host access protection flags not allowed");
    
    // TODO: Implement optimization strategy.
    HostAccessProt = MemoryObj::HostWriteOnly;
  }
  
  return *this;
}

MemoryObjBuilder &MemoryObjBuilder::SetHostReadOnly(bool Enabled) {
  if(Enabled) {
    if(HostAccessProt == MemoryObj::HostWriteOnly || 
        HostAccessProt == MemoryObj::HostNoAccess)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple host access protection flags not allowed");
    
    // TODO: Implement optimization strategy.
    HostAccessProt = MemoryObj::HostReadOnly;
  }
  
  return *this;
}

MemoryObjBuilder &MemoryObjBuilder::SetHostNoAccess(bool Enabled) {
  if(Enabled) {
    if(HostAccessProt == MemoryObj::HostWriteOnly || 
        HostAccessProt == MemoryObj::HostReadOnly)
      return NotifyError(CL_INVALID_VALUE,
                         "multiple host access protection flags not allowed");
    
    // TODO: Implement optimization strategy.
    HostAccessProt = MemoryObj::HostNoAccess;
  }
  
  return *this;
}

MemoryObjBuilder &MemoryObjBuilder::NotifyError(cl_int ErrCode, const char *Msg) {
  Ctx.ReportDiagnostic(Msg);
  this->ErrCode = ErrCode;

  return *this;
}

//
// BufferBuilder implementation.
//

BufferBuilder::BufferBuilder(Context &Ctx, size_t Size) :
  MemoryObjBuilder(MemoryObjBuilder::BufferBuilder, Ctx),
  Parent(NULL),
  Offset(0) {
  this->Size = Size;  
  if(!this->Size) {
    NotifyError(CL_INVALID_BUFFER_SIZE, "buffer size must be greater than zero");
    return;
  }

  for(Context::device_iterator I = Ctx.device_begin(), E = Ctx.device_end();
                               I != E;
                               ++I)
    if(this->Size > (*I)->GetMaxMemoryAllocSize()) {
      NotifyError(CL_INVALID_BUFFER_SIZE,
                  "buffer size exceed device capabilities");
      return;
    }
}

BufferBuilder::BufferBuilder(Buffer &Parent, size_t Offset, size_t Size) :
  MemoryObjBuilder(MemoryObjBuilder::BufferBuilder, Parent.GetContext()),
  Parent(&Parent),
  Offset(Offset) {
  if(Parent.GetParent()) {
    NotifyError(CL_INVALID_MEM_OBJECT, "parent buffer is a sub-buffer");
    return;
  }

  this->Size = Size;
  if(!this->Size) {
    NotifyError(CL_INVALID_BUFFER_SIZE, "sub-buffer size must be greater than zero");
    return;
  }

  if(this->Offset + this->Size > Parent.GetSize()) {
    NotifyError(CL_INVALID_VALUE, "specified region is out of bounds in the parent buffer");
    return;
  }

  // If Offset is not aligned with all devices in the context associated with the 
  // parent buffer, CL_MISALIGNED_SUB_BUFFER_OFFSET is returned as the error code.
  bool IsMisalignedOffset = true;
  Context &Ctx = Parent.GetContext();
  for(Context::device_iterator I = Ctx.device_begin(),
                               E = Ctx.device_end(); 
                               I != E;
                               ++I) {
    if(((*I)->GetMemoryBaseAddressAlignment() != 0) && 
       ((Offset % (*I)->GetMemoryBaseAddressAlignment()) == 0)) {
      IsMisalignedOffset = false;
      break;
    }
  }

  if(IsMisalignedOffset) {
    NotifyError(CL_MISALIGNED_SUB_BUFFER_OFFSET, "sub-buffer origin is misaligned");
    return;
  }
}

Buffer *BufferBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  // Sub-buffer object with non-zero Offset must account for it in HostPtr.
  if(Offset)
    HostPtr = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(HostPtr) + Offset);

  if(HostPtrMode == MemoryObj::UseHostPtr)
    return Ctx.CreateHostBuffer(Parent, Offset, Size, HostPtr, AccessProt, HostAccessProt, ErrCode);
  else if(HostPtrMode == MemoryObj::AllocHostPtr)
    return Ctx.CreateHostAccessibleBuffer(Parent, Offset, Size, HostPtr, AccessProt, HostAccessProt, ErrCode);
  else
    return Ctx.CreateDeviceBuffer(Parent, Offset, Size, HostPtr, AccessProt, HostAccessProt, ErrCode);
}

//
// ImageBuilder implementation.
//

ImageBuilder::ImageBuilder(Context &Ctx) :
  MemoryObjBuilder(MemoryObjBuilder::ImageBuilder, Ctx),
  ChOrder(Image::ChOrder_Invalid),
  ChDataType(Image::ChType_Invalid),
  ElementSize(0),
  Width(0),
  Height(0),
  Depth(0),
  ArraySize(0),
  HostRowPitch(0),
  HostSlicePitch(0),
  NumMipLevels(0),
  NumSamples(0),
  Buf(NULL) { }
  
ImageBuilder &ImageBuilder::SetFormat(const cl_image_format *ImgFmt) {
  if(!ImgFmt)
    return NotifyError(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
                       "image format descriptor is null");

  size_t NumChannels = 0;
  size_t DataSize = 0;
  
  // Check if values specified for the image format are valid and
  // determine the number of channels and their size.
  switch(ImgFmt->image_channel_order) {
  case CL_R:
  case CL_A:
  case CL_Rx:
    NumChannels = 1;
    break;
  
  case CL_INTENSITY:
  case CL_LUMINANCE:
    NumChannels = 1;
    switch(ImgFmt->image_channel_data_type) {
    case CL_UNORM_INT8:
    case CL_UNORM_INT16:
    case CL_SNORM_INT8:
    case CL_SNORM_INT16:
    case CL_HALF_FLOAT:
    case CL_FLOAT:
      break;
    default:
      return NotifyError(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
                         "invalid image channel order and data type combination");
    }
    break;
  
  case CL_RG:
  case CL_RA:
  case CL_RGx:
    NumChannels = 2;
    break;
  
    break;
    
  case CL_RGB:
  case CL_RGBx:
    NumChannels = 3;
    switch(ImgFmt->image_channel_data_type) {
    case CL_UNORM_SHORT_555:
	case CL_UNORM_SHORT_565:
	case CL_UNORM_INT_101010:
      break;
    default:
			return NotifyError(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
                         "invalid image channel order and data type combination");
    }
    break;
  
  case CL_RGBA:
    NumChannels = 4;
    break;
    
  case CL_ARGB:
  case CL_BGRA:
    NumChannels = 4;
    switch(ImgFmt->image_channel_data_type) {
    case CL_UNORM_INT8: 
    case CL_SNORM_INT8:
    case CL_SIGNED_INT8:
    case CL_UNSIGNED_INT8:
      break;
    default:
      return NotifyError(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
                         "invalid image channel order and data type combination");
    }
    break;
    
  default:
    return NotifyError(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
                       "invalid image channel order");
  }
  
  switch(ImgFmt->image_channel_data_type) {
  case CL_SNORM_INT8:
  case CL_UNORM_INT8:
  case CL_SIGNED_INT8:
  case CL_UNSIGNED_INT8:
    DataSize = 1;
    break;
    
  case CL_SNORM_INT16:
  case CL_UNORM_INT16:
  case CL_UNORM_SHORT_565:
  case CL_UNORM_SHORT_555:
  case CL_SIGNED_INT16:
  case CL_UNSIGNED_INT16:
  case CL_HALF_FLOAT:
    DataSize = 2;
    break;
  
  case CL_UNORM_INT_101010:
  case CL_SIGNED_INT32:
  case CL_UNSIGNED_INT32:
  case CL_FLOAT:
    DataSize = 4;
    break;
  
  default:
    return NotifyError(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
                       "invalid image channel data type");
  }
  
  // The element size is the size in bytes of each pixel element.
  ElementSize = (ImgFmt->image_channel_data_type == CL_UNORM_SHORT_565 ||
                 ImgFmt->image_channel_data_type == CL_UNORM_SHORT_555 ||
                 ImgFmt->image_channel_data_type == CL_UNORM_INT_101010) ?
                 DataSize : DataSize * NumChannels;
                 
  // Check if the specified image format is supported.
  bool FmtSupported = false;
  for(opencrun::Context::device_iterator I = Ctx.device_begin(),
                                         E = Ctx.device_end(); 
                                         I != E;
                                         ++I) {
    llvm::ArrayRef<cl_image_format> DevFmts = (*I)->GetSupportedImageFormats();
    if(DevFmts.size() == 0) continue;
    
    for(unsigned K = 0; K < DevFmts.size(); ++K)
      if((ImgFmt->image_channel_order == DevFmts[K].image_channel_order) &&
         (ImgFmt->image_channel_data_type == DevFmts[K].image_channel_data_type)) {
        FmtSupported = true;
        // Current device is added to the container for all
        // devices in context supporting the given image format.
        TargetDevs.push_back(*I);
        break;
      }
  }
  
  if(!FmtSupported)
    return NotifyError(CL_IMAGE_FORMAT_NOT_SUPPORTED,
                       "specified image format unsupported");

  // Convert the unsigned parameter types to enum values.
  ChOrder = Image::ChannelOrder(ImgFmt->image_channel_order);
  ChDataType = Image::ChannelType(ImgFmt->image_channel_data_type);

  return *this;
}

ImageBuilder &ImageBuilder::SetDesc(const cl_image_desc *ImgDesc) {
  if(!ImgDesc)
    return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR,
                       "image descriptor is null");

  if(ImgDesc->num_mip_levels)
    return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR,
        "non zero mip levels");

  if(ImgDesc->num_samples)
    return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR,
        "non zero samples");

  if(TargetDevs.size() == 0)
    return NotifyError(CL_IMAGE_FORMAT_NOT_SUPPORTED,
                       "specified image format unsupported");

  // Determine the minimum maximum image dimensions.
  size_t Min_Image2DMaxWidth = TargetDevs[0]->GetImage2DMaxWidth(),
         Min_Image2DMaxHeight = TargetDevs[0]->GetImage2DMaxHeight(),
         Min_Image3DMaxWidth = TargetDevs[0]->GetImage3DMaxWidth(),
         Min_Image3DMaxHeight = TargetDevs[0]->GetImage3DMaxHeight(),
         Min_Image3DMaxDepth = TargetDevs[0]->GetImage3DMaxDepth(),
         Min_ImageMaxBufferSize = TargetDevs[0]->GetImageMaxBufferSize(), 
         Min_ImageMaxArraySize = TargetDevs[0]->GetImageMaxArraySize();

  for(unsigned I = 1; I < TargetDevs.size(); ++I) {
    Min_Image2DMaxWidth = std::min(Min_Image2DMaxWidth,
                                   TargetDevs[I]->GetImage2DMaxWidth());
    Min_Image2DMaxHeight = std::min(Min_Image2DMaxHeight,
                                    TargetDevs[I]->GetImage2DMaxHeight());
    Min_Image3DMaxWidth = std::min(Min_Image3DMaxWidth,
                                   TargetDevs[I]->GetImage3DMaxWidth());
    Min_Image3DMaxHeight = std::min(Min_Image3DMaxHeight,
                                    TargetDevs[I]->GetImage3DMaxHeight());
    Min_Image3DMaxDepth = std::min(Min_Image3DMaxDepth,
                                   TargetDevs[I]->GetImage3DMaxDepth());
    Min_ImageMaxBufferSize = std::min(Min_ImageMaxBufferSize,
                                      TargetDevs[I]->GetImageMaxBufferSize()); 
    Min_ImageMaxArraySize = std::min(Min_ImageMaxArraySize,
                                     TargetDevs[I]->GetImageMaxArraySize());
  }

  switch(ImgDesc->image_type) {
    case CL_MEM_OBJECT_IMAGE1D:
      if(ImgDesc->image_width == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "image width is zero");

      if(ImgDesc->image_width > Min_Image2DMaxWidth)
        return NotifyError(CL_INVALID_IMAGE_SIZE, 
                           "image width bigger than 2D image maximum width");

      if(ImgDesc->buffer != NULL)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "buffer object is not null");

      ImgTy = Image::Image1D;
      Width = ImgDesc->image_width;
      Height = 1;
      Depth = 1;
      ArraySize = 1;
      break;
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
      if(ImgDesc->image_width == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "image width is zero");

      if(ImgDesc->image_width > Min_ImageMaxBufferSize)
        return NotifyError(CL_INVALID_IMAGE_SIZE, 
                           "image width bigger than maximum buffer size");

      if(ImgDesc->buffer == NULL)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "buffer object is null");

      Buf = llvm::cast<Buffer>(llvm::cast<MemoryObj>(ImgDesc->buffer));

      if((ImgDesc->image_width * ElementSize) <= Buf->GetSize())
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, 
                           "incompatible image width and given buffer size");

      ImgTy = Image::Image1D_Buffer;
      Width = ImgDesc->image_width;
      Height = 1;
      Depth = 1;
      ArraySize = 1;
      break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
      if(ImgDesc->image_width == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "image width is zero");

      if(ImgDesc->image_array_size == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "array size is zero");

      if(ImgDesc->image_width > Min_Image2DMaxWidth)
        return NotifyError(CL_INVALID_IMAGE_SIZE, 
                           "image width bigger than 2D image maximum width");

      if(ImgDesc->image_array_size < 1 || 
         ImgDesc->image_array_size > Min_ImageMaxArraySize)
        return NotifyError(CL_INVALID_IMAGE_SIZE, 
                           "invalid image array size");

      if(ImgDesc->buffer != NULL)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "buffer object is not null");

      ImgTy = Image::Image1D_Array;
      Width = ImgDesc->image_width;
      Height = 1;
      Depth = 1;
      ArraySize = ImgDesc->image_array_size;
      break;
    case CL_MEM_OBJECT_IMAGE2D:
      if(ImgDesc->image_width == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "image width is zero");

      if(ImgDesc->image_height == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "image height is zero");

      if(ImgDesc->image_width > Min_Image2DMaxWidth)
        return NotifyError(CL_INVALID_IMAGE_SIZE, 
                           "image width bigger than 2D image maximum width");

      if(ImgDesc->image_height > Min_Image2DMaxHeight)
        return NotifyError(CL_INVALID_IMAGE_SIZE, 
                           "image height bigger than 2D image maximum height");

      if(ImgDesc->buffer != NULL)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "buffer object is not null");

      ImgTy = Image::Image2D;
      Width = ImgDesc->image_width;
      Height = ImgDesc->image_height;
      Depth = 1;
      ArraySize = 1;
      break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
      if(ImgDesc->image_width == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "image width is zero");

      if(ImgDesc->image_height == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "image height is zero");

      if(ImgDesc->image_array_size == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "array size is zero");
      
      if(ImgDesc->image_width > Min_Image2DMaxWidth)
        return NotifyError(CL_INVALID_IMAGE_SIZE, 
                           "image width bigger than 2D image maximum width");

      if(ImgDesc->image_height > Min_Image2DMaxHeight)
        return NotifyError(CL_INVALID_IMAGE_SIZE, 
                           "image height bigger than 2D image maximum height");

      if(ImgDesc->image_array_size < 1 || 
         ImgDesc->image_array_size > Min_ImageMaxArraySize)
        return NotifyError(CL_INVALID_IMAGE_SIZE, 
                           "invalid image array size");
     
      if(ImgDesc->buffer != NULL)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "buffer object is not null");

      ImgTy = Image::Image2D_Array;
      Width = ImgDesc->image_width;
      Height = ImgDesc->image_height;
      Depth = 1;
      ArraySize = ImgDesc->image_array_size;
      break;
    case CL_MEM_OBJECT_IMAGE3D:
      if(ImgDesc->image_width == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "image width is zero");

      if(ImgDesc->image_height == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "image height is zero");

      if(ImgDesc->image_depth == 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "image depth is zero");

      if(ImgDesc->image_width > Min_Image3DMaxWidth)
        return NotifyError(CL_INVALID_IMAGE_SIZE,
                           "image width bigger than 3D image maximum width");

      if(ImgDesc->image_height > Min_Image3DMaxHeight)
        return NotifyError(CL_INVALID_IMAGE_SIZE,
                           "image height bigger than 3D image maximum height");

      if(ImgDesc->image_depth > Min_Image3DMaxDepth)
        return NotifyError(CL_INVALID_IMAGE_SIZE,
                           "image depth bigger than 3D image maximum depth");

      if(ImgDesc->buffer != NULL)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR, "buffer object is not null");

      ImgTy = Image::Image3D;
      Width = ImgDesc->image_width;
      Height = ImgDesc->image_height;
      Depth = ImgDesc->image_depth;
      ArraySize = 1;
      break;
    default:
      return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR,
                         "invalid image type");
  }

  // Total size in bytes required by the image.
  Size = Width * Height * Depth * ElementSize * ArraySize;

  if(HostPtr == NULL) {
    if(ImgDesc->image_row_pitch != 0)
      return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR,
                         "host-ptr is null but row pitch is not zero");

    // The image_slice_pitch correctness is checked only for those image
    // types for which it makes sense. In this way we avoid errors caused
    // by calling the C++ APIs (cl::Image derived classes from cl.hpp).
    switch(ImgTy) {
    case Image::Image1D_Array:
    case Image::Image2D_Array:
    case Image::Image3D:
      if(ImgDesc->image_slice_pitch != 0)
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR,
                           "host-ptr is null but slice pitch is not zero");
      break;
    default:
      break;
    }
  } else {
    if(ImgDesc->image_row_pitch == 0)
      HostRowPitch = Width * ElementSize;
    else if((ImgDesc->image_row_pitch < Width * ElementSize) ||
            (ImgDesc->image_row_pitch % ElementSize != 0))
      return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR,
                         "invalid row pitch for non null host-ptr"); 
    else
      HostRowPitch = ImgDesc->image_row_pitch;
    
    // Slice-pitch is used only for 1D image arrays, 2D image arrays and
    // 3D images.
    switch(ImgTy) {
    case Image::Image1D_Array:
      if(ImgDesc->image_slice_pitch == 0)
        HostSlicePitch = HostRowPitch;
      else if((ImgDesc->image_slice_pitch < HostRowPitch) ||
              (ImgDesc->image_slice_pitch % HostRowPitch != 0))
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR,
                           "invalid slice pitch for non null host-ptr");
      else
        HostSlicePitch = ImgDesc->image_slice_pitch;
      break;
    case Image::Image2D_Array:
    case Image::Image3D:
      if(ImgDesc->image_slice_pitch == 0)
        HostSlicePitch = HostRowPitch * Height;
      else if((ImgDesc->image_slice_pitch < HostRowPitch * Height) ||
              (ImgDesc->image_slice_pitch % HostRowPitch != 0))
        return NotifyError(CL_INVALID_IMAGE_DESCRIPTOR,
                           "invalid slice pitch for non null host-ptr");
      else
        HostSlicePitch = ImgDesc->image_slice_pitch;
      break;
    default:
      break;
    }
  }

  switch(ImgTy) {
  case Image::Image1D_Buffer:
    if((Buf->GetAccessProtection() == MemoryObj::WriteOnly &&
          (AccessProt == MemoryObj::ReadWrite ||
           AccessProt == MemoryObj::ReadOnly)) ||
       (Buf->GetAccessProtection() == MemoryObj::ReadOnly &&
          (AccessProt == MemoryObj::ReadWrite ||
           AccessProt == MemoryObj::WriteOnly)))
      return NotifyError(CL_INVALID_VALUE,
          "access mode incompatible with data buffer");

    if((Buf->GetHostAccessProtection() == MemoryObj::HostWriteOnly && 
        HostAccessProt == MemoryObj::HostReadOnly) ||
       (Buf->GetHostAccessProtection() == MemoryObj::HostReadOnly &&
        HostAccessProt == MemoryObj::HostWriteOnly) ||
       (Buf->GetHostAccessProtection() == MemoryObj::HostNoAccess &&
        (HostAccessProt == MemoryObj::HostReadOnly ||
         HostAccessProt == MemoryObj::HostWriteOnly)))
      return NotifyError(CL_INVALID_VALUE,
          "host access mode incompatible with data buffer");

    // 1D image buffers cannot specify host-ptr usage mode as they
    // inherit it from associated data buffer.
    if(HostPtrMode)
      return NotifyError(CL_INVALID_VALUE,
          "host-ptr usage mode specified");

    HostPtrMode = Buf->GetHostPtrUsageMode();

    // If access protections are not specified withing flags they
    // are inherited from the corresponding memory access qualifiers
    // associated with buffer.
    if(!AccessProt)
      AccessProt = Buf->GetAccessProtection();

    // The same happens for host access protections.
    if(!HostAccessProt)
      HostAccessProt = Buf->GetHostAccessProtection();
    break;
  default:
    // For all image types except CL_MEM_OBJECT_IMAGE1D_BUFFER,
    // if value specified for flags is 0, the default is used 
    // which is CL_MEM_READ_WRITE.
    if(!(HostPtrMode | AccessProt | HostAccessProt))
      AccessProt = MemoryObj::ReadWrite;
  }

  return *this;
}

Image *ImageBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(HostPtrMode == MemoryObj::UseHostPtr)
    return Ctx.CreateHostImage(
        Size,
        HostPtr,
        TargetDevs,
        ChOrder, ChDataType,
        ElementSize,
        ImgTy,
        Width, Height, Depth,
        ArraySize,
        HostRowPitch, HostSlicePitch,
        NumMipLevels,
        NumSamples,
        Buf,
        AccessProt, HostAccessProt,
        ErrCode);
  else if(HostPtrMode == MemoryObj::AllocHostPtr)
    return Ctx.CreateHostAccessibleImage(
        Size,
        HostPtr,
        TargetDevs,
        ChOrder, ChDataType,
        ElementSize,
        ImgTy,
        Width, Height, Depth,
        ArraySize,
        HostRowPitch, HostSlicePitch,
        NumMipLevels,
        NumSamples,
        Buf,
        AccessProt, HostAccessProt,
        ErrCode);
  else
    return Ctx.CreateDeviceImage(
        Size,
        HostPtr,
        TargetDevs,
        ChOrder, ChDataType,
        ElementSize,
        ImgTy,
        Width, Height, Depth,
        ArraySize,
        HostRowPitch, HostSlicePitch,
        NumMipLevels,
        NumSamples,
        Buf,
        AccessProt, HostAccessProt,
        ErrCode);
}

