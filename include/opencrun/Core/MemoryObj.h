
#ifndef OPENCRUN_CORE_MEMORYOBJ_H
#define OPENCRUN_CORE_MEMORYOBJ_H

#include "CL/opencl.h"

#include "opencrun/Util/MTRefCounted.h"

#include "llvm/Support/Mutex.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallPtrSet.h"

#include <map>

struct _cl_mem { };

namespace opencrun {

class Context;
class Device;
class Image;

class MemoryObj : public _cl_mem, public MTRefCountedBaseVPTR<MemoryObj> {
public:
  enum Type {
    HostBuffer,
    HostAccessibleBuffer,
    DeviceBuffer,
    VirtualBuffer,
    LastBuffer,
    HostImage,
    HostAccessibleImage,
    DeviceImage,
    LastImage
  };

  enum HostPtrUsageMode {
    UseHostPtr		= CL_MEM_USE_HOST_PTR,
    AllocHostPtr	= CL_MEM_ALLOC_HOST_PTR,
    CopyHostPtr		= CL_MEM_COPY_HOST_PTR,
    NoHostPtrUsage  = 0
  };
  
  enum AccessProtection {
    ReadWrite         = CL_MEM_READ_WRITE,
    WriteOnly         = CL_MEM_WRITE_ONLY,
    ReadOnly          = CL_MEM_READ_ONLY,
    InvalidProtection = 0
  };
  
  enum HostAccessProtection {
    HostWriteOnly		= CL_MEM_HOST_WRITE_ONLY,
    HostReadOnly		= CL_MEM_HOST_READ_ONLY,
    HostNoAccess		= CL_MEM_HOST_NO_ACCESS,
    HostNoProtection	= 0
  };
  
public:
  static bool classof(const _cl_mem *MemObj) { return true; }

public:
  struct MappingInfo {
    size_t Offset;
    size_t Size;
    const size_t *Origin;
    const size_t *Region;
    cl_map_flags MapFlags;

    bool CheckOverlap(const MappingInfo &MapInfo);
  };
  
  typedef std::multimap<void *, MappingInfo> MappingsContainer;
  
public:
  typedef MappingsContainer::iterator maps_iterator;
  
protected:
  MemoryObj(Type MemTy,
            Context &Ctx,
            size_t Size,
            HostPtrUsageMode HostPtrMode,
            AccessProtection AccessProt,
            HostAccessProtection HostAccessProt) : MemTy(MemTy),
                                                   Ctx(&Ctx),
                                                   Size(Size),
                                                   HostPtrMode(HostPtrMode),
                                                   AccessProt(AccessProt),
                                                   HostAccessProt(HostAccessProt)	{ }

public:
  virtual ~MemoryObj();

public:
  size_t GetSize() const { return Size; }
  Type GetType() const { return MemTy; }
  Context &GetContext() const { return *Ctx; }
  
  HostPtrUsageMode GetHostPtrUsageMode() const { return HostPtrMode; }
  AccessProtection GetAccessProtection() const { return AccessProt; }
  HostAccessProtection GetHostAccessProtection() const { return HostAccessProt; }
  
  unsigned long GetMemFlags() const { 
    return static_cast<unsigned long>(HostPtrMode | 
                                      AccessProt | 
                                      HostAccessProt); 
  }
  
public:
  bool AddNewMapping(void *MapBuf, const MappingInfo &MapInfo);
  
  bool RemoveMapping(void *MapBuf);
  
  bool IsValidMappingPtr(void *MapBuf);
  
  MappingInfo *GetMappingInfo(void *MapBuf);
  
  size_t GetMappedCount() const { return Maps.size(); }
  
private:
  Type MemTy;

  llvm::IntrusiveRefCntPtr<Context> Ctx;
  size_t Size;
  
  HostPtrUsageMode HostPtrMode;
  AccessProtection AccessProt;
  HostAccessProtection HostAccessProt;
  
  llvm::sys::Mutex ThisLock;
  MappingsContainer Maps;
};

class Buffer : public MemoryObj {
public:
  static bool classof(const MemoryObj *MemObj) {
    return MemObj->GetType() < MemoryObj::LastBuffer;
  }

public:
  typedef llvm::SmallPtrSet<Image *, 4> ImagesContainer;

  typedef ImagesContainer::iterator image_iterator;

public:
  image_iterator image_begin() { return AttachedImages.begin(); }
  image_iterator image_end() { return AttachedImages.end(); }

public:
  bool AddAttachedImage(Image *Img) {
    if(!Img) return false;
    return AttachedImages.insert(Img);
  }

  bool RemoveAttachedImage(Image *Img) {
    if(!Img) return false;
    return AttachedImages.erase(Img);
  }

protected:
  Buffer(Type MemTy,
         Context &Ctx,
         size_t Size,
         MemoryObj::HostPtrUsageMode HostPtrMode,
         MemoryObj::AccessProtection AccessProt,
         MemoryObj::HostAccessProtection HostAccessProt) :
  MemoryObj(MemTy, Ctx, Size, HostPtrMode, AccessProt, HostAccessProt) { }

private:
  // This container holds all 1D image buffers initialized using
  // the buffer object.
  ImagesContainer AttachedImages;
};

class Image : public MemoryObj {
public:
  enum ImgType {
    Image1D         = CL_MEM_OBJECT_IMAGE1D,
    Image1D_Array   = CL_MEM_OBJECT_IMAGE1D_ARRAY,
    Image1D_Buffer  = CL_MEM_OBJECT_IMAGE1D_BUFFER,
    Image2D         = CL_MEM_OBJECT_IMAGE2D,
    Image2D_Array   = CL_MEM_OBJECT_IMAGE2D_ARRAY,
    Image3D         = CL_MEM_OBJECT_IMAGE3D,
    ImageInvalid    = 0
  };

  enum ChannelOrder {
    ChOrder_R               = CL_R,
    ChOrder_A               = CL_A,
    ChOrder_RG              = CL_RG,
    ChOrder_RA              = CL_RA,
    ChOrder_RGB             = CL_RGB,
    ChOrder_RGBA            = CL_RGBA,
    ChOrder_BGRA            = CL_BGRA,
    ChOrder_ARGB            = CL_ARGB,
    ChOrder_Intensity       = CL_INTENSITY,
    ChOrder_Luminance       = CL_LUMINANCE,
    ChOrder_Rx              = CL_Rx,
    ChOrder_RGx             = CL_RGx,
    ChOrder_RGBx            = CL_RGBx,
    ChOrder_Depth           = CL_DEPTH,
    ChOrder_Depth_Stencil   = CL_DEPTH_STENCIL,
    ChOrder_Invalid         = 0
  };

  enum ChannelType {
    ChType_SNorm_Int8       = CL_SNORM_INT8,
    ChType_SNorm_Int16      = CL_SNORM_INT16,
    ChType_UNorm_Int8       = CL_UNORM_INT8,
    ChType_UNorm_In16       = CL_UNORM_INT16,
    ChType_UNorm_Short_565  = CL_UNORM_SHORT_565,
    ChType_UNorm_Short_555  = CL_UNORM_SHORT_555,
    ChType_UNorm_Int_101010 = CL_UNORM_INT_101010,
    ChType_Signed_Int8      = CL_SIGNED_INT8,
    ChType_Signed_Int16     = CL_SIGNED_INT16,
    ChType_Signed_Int32     = CL_SIGNED_INT32,
    ChType_Unsigned_Int8    = CL_UNSIGNED_INT8,
    ChType_Unsigned_Int16   = CL_UNSIGNED_INT16,
    ChType_Unsigned_Int32   = CL_UNSIGNED_INT32,
    ChType_Half_Float       = CL_HALF_FLOAT,
    ChType_Float            = CL_FLOAT,
    ChType_UNorm_Int24      = CL_UNORM_INT24,
    ChType_Invalid          = 0
  };


public:
  static bool classof(const MemoryObj *MemObj) {
    return (MemObj->GetType() > MemoryObj::LastBuffer) &&
           (MemObj->GetType() < MemoryObj::LastImage);
  }

public:
  // This container type is used to store pointers to devices
  // in context supporting the specified image format. 
  // Image will be allocated only on these devices.
  typedef llvm::SmallVector<Device *, 4> TargetDevices;

  typedef TargetDevices::iterator targetdev_iterator;

public:
  targetdev_iterator targetdev_begin() { return TargetDevs.begin(); }
  targetdev_iterator targetdev_end() { return TargetDevs.end(); }

protected:
  Image(Type MemTy,
        Context &Ctx,
        size_t Size,
        TargetDevices &TargetDevs,
        ChannelOrder ChOrder,
        ChannelType ChDataType,
        size_t ElementSize,
        ImgType ImgTy,
        size_t Width,
        size_t Height,
        size_t Depth,
        size_t ArraySize,
        size_t HostRowPitch,
        size_t HostSlicePitch,
        unsigned NumMipLevels,
        unsigned NumSamples,
        Buffer *Buf,
        MemoryObj::HostPtrUsageMode HostPtrMode,
        MemoryObj::AccessProtection AccessProt,
        MemoryObj::HostAccessProtection HostAccessProt) :
  MemoryObj(MemTy, Ctx, Size, HostPtrMode, AccessProt, HostAccessProt),
  TargetDevs(TargetDevs),
  ChOrder(ChOrder),
  ChDataType(ChDataType),
  ElementSize(ElementSize),
  ImgTy(ImgTy),
  Width(Width),
  Height(Height),
  Depth(Depth),
  ArraySize(ArraySize),
  NumMipLevels(NumMipLevels),
  NumSamples(NumSamples),
  Buf(Buf) { 
    if(GetType() == HostImage) {
      // In case of CL_MEM_USE_HOST_PTR the storage area for the
      // image object is inside host memory and its address is
      // given by Storage pointer (see HostImage class) and image
      // pitches coincide with storage pitches.
      HostPitches[0] = HostRowPitch;
      HostPitches[1] = HostSlicePitch;
      Pitches[0] = HostPitches[0];
      Pitches[1] = HostPitches[1];
    } else {
      HostPitches[0] = HostRowPitch;
      HostPitches[1] = HostSlicePitch;
      Pitches[0] = Width * ElementSize;
      Pitches[1] = Pitches[0] * Height;
    }
  }

  ~Image() {
    if(GetImageType() == Image1D_Buffer) {
      GetBuffer()->RemoveAttachedImage(this); 
    }
  }

public:
  ChannelOrder GetChannelOrder() const { return ChOrder; }
  ChannelType GetChannelType() const { return ChDataType; }

  cl_image_format GetImageFormat() const {
    cl_image_format img_fmt;
    img_fmt.image_channel_order = ChOrder;
    img_fmt.image_channel_data_type = ChDataType;

    return img_fmt;
  }

  ImgType GetImageType() const { return ImgTy; }
   
  size_t GetWidth() const { return Width; }
  size_t GetHeight() const { return Height; }
  size_t GetDepth() const { return Depth; }

  size_t GetArraySize() const { return ArraySize; }

  size_t GetRowPitch() const { return Pitches[0]; }
  size_t GetSlicePitch() const { return Pitches[1]; }
  size_t GetHostRowPitch() const { return HostPitches[0]; }
  size_t GetHostSlicePitch() const { return HostPitches[1]; }

  unsigned GetNumMipLevels() const { return NumMipLevels; }
  unsigned GetNumSamples() const { return NumSamples; }

  Buffer *GetBuffer() const { return Buf.getPtr(); }

  size_t GetElementSize() const { return ElementSize; }

private:
  TargetDevices TargetDevs;

  ChannelOrder ChOrder;
  ChannelType ChDataType;

  size_t ElementSize;

  ImgType ImgTy;

  size_t Width;
  size_t Height;
  size_t Depth;

  size_t ArraySize;

  size_t Pitches[2];
  size_t HostPitches[2];

  unsigned NumMipLevels;
  unsigned NumSamples;

  llvm::IntrusiveRefCntPtr<Buffer> Buf;
};

class HostBuffer : public Buffer {
public:
  static bool classof(const MemoryObj *MemObj) {
    return MemObj->GetType() == MemoryObj::HostBuffer;
  }
  
private:
  HostBuffer(Context &Ctx,
             size_t Size,
             void *Storage,
             MemoryObj::HostPtrUsageMode HostPtrMode,
             MemoryObj::AccessProtection AccessProt,
             MemoryObj::HostAccessProtection HostAccessProt)
    : Buffer(MemoryObj::HostBuffer, Ctx, Size, HostPtrMode, AccessProt, HostAccessProt),
      Storage(Storage) { }

  HostBuffer(const HostBuffer &That); // Do not implement.
  void operator=(const HostBuffer &That); // Do not implement.

public:
  void *GetStorageData() const { return Storage; }

private:
  void *Storage;
  
  friend class Context;
};

class HostAccessibleBuffer : public Buffer {
public:
  static bool classof(const MemoryObj *MemObj) {
    return MemObj->GetType() == MemoryObj::HostAccessibleBuffer;
  }
  
private:
  HostAccessibleBuffer(Context &Ctx,
                       size_t Size,
                       void *Src,
                       MemoryObj::HostPtrUsageMode HostPtrMode,
                       MemoryObj::AccessProtection AccessProt,
                       MemoryObj::HostAccessProtection HostAccessProt)
    : Buffer(MemoryObj::HostAccessibleBuffer, Ctx, Size, HostPtrMode, AccessProt, HostAccessProt),
      Src(Src) { }

  HostAccessibleBuffer(const HostAccessibleBuffer &That); // Do not implement.
  void operator=(const HostAccessibleBuffer &That); // Do not implement.

public:
  const void *GetInitializationData() const { return Src; }
  bool HasInitializationData() const { return Src; }
  
private:
  void *Src;
  
  friend class Context;
};

class DeviceBuffer : public Buffer {
public:
  static bool classof(const MemoryObj *MemObj) {
    return MemObj->GetType() == MemoryObj::DeviceBuffer;
  }
  
private:
  DeviceBuffer(Context &Ctx,
               size_t Size,
               void *Src,
               MemoryObj::HostPtrUsageMode HostPtrMode,
               MemoryObj::AccessProtection AccessProt,
               MemoryObj::HostAccessProtection HostAccessProt)
    : Buffer(MemoryObj::DeviceBuffer, Ctx, Size, HostPtrMode, AccessProt, HostAccessProt),
      Src(Src) { }

  DeviceBuffer(const DeviceBuffer &That); // Do not implement.
  void operator=(const DeviceBuffer &That); // Do not implement.

public:
  const void *GetInitializationData() const { return Src; }
  bool HasInitializationData() const { return Src; }

private:
  void *Src;

  friend class Context;
};

class VirtualBuffer : public Buffer {
public:
  static bool classof(const MemoryObj *MemObj) {
    return MemObj->GetType() == MemoryObj::VirtualBuffer;
  }
  
private:
  VirtualBuffer(Context &Ctx,
                size_t Size,
                MemoryObj::AccessProtection AccessProt,
                MemoryObj::HostAccessProtection HostAccessProt)
    : Buffer(MemoryObj::VirtualBuffer, Ctx, Size, MemoryObj::NoHostPtrUsage, AccessProt, HostAccessProt) { }

  VirtualBuffer(const VirtualBuffer &That); // Do not implement.
  void operator=(const VirtualBuffer &That); // Do not implement.

private:
  friend class Context;
};

class HostImage : public Image {
public:
  static bool classof(const MemoryObj *MemObj) {
    return MemObj->GetType() == MemoryObj::HostImage;
  }
  
private:
  HostImage(Context &Ctx,
            size_t Size,
            void *Storage,
            TargetDevices &TargetDevs,
            ChannelOrder ChOrder,
            ChannelType ChDataType,
            size_t ElementSize,
            ImgType ImgTy,
            size_t Width,
            size_t Height,
            size_t Depth,
            size_t ArraySize,
            size_t RowPitch,
            size_t SlicePitch,
            unsigned NumMipLevels,
            unsigned NumSamples,
            Buffer *Buf,
            MemoryObj::HostPtrUsageMode HostPtrMode,
            MemoryObj::AccessProtection AccessProt,
            MemoryObj::HostAccessProtection HostAccessProt)
    : Image(MemoryObj::HostImage,
            Ctx,
            Size,
            TargetDevs,
            ChOrder, ChDataType,
            ElementSize,
            ImgTy,
            Width, Height, Depth,
            ArraySize,
            RowPitch, SlicePitch,
            NumMipLevels,
            NumSamples,
            Buf,
            HostPtrMode, AccessProt, HostAccessProt),
      Storage(Storage) { }

  HostImage(const HostImage &That); // Do not implement.
  void operator=(const HostImage &That); // Do not implement.

public:
  void *GetStorageData() const { return Storage; }

private:
  void *Storage;
  
  friend class Context;
};

class HostAccessibleImage : public Image {
public:
  static bool classof(const MemoryObj *MemObj) {
    return MemObj->GetType() == MemoryObj::HostAccessibleImage;
  }
  
private:
  HostAccessibleImage(Context &Ctx,
                       size_t Size,
                       void *Src,
                       TargetDevices &TargetDevs,
                       ChannelOrder ChOrder,
                       ChannelType ChDataType,
                       size_t ElementSize,
                       ImgType ImgTy,
                       size_t Width,
                       size_t Height,
                       size_t Depth,
                       size_t ArraySize,
                       size_t RowPitch,
                       size_t SlicePitch,
                       unsigned NumMipLevels,
                       unsigned NumSamples,
                       Buffer *Buf,
                       MemoryObj::HostPtrUsageMode HostPtrMode,
                       MemoryObj::AccessProtection AccessProt,
                       MemoryObj::HostAccessProtection HostAccessProt)
    : Image(MemoryObj::HostAccessibleImage,
            Ctx,
            Size,
            TargetDevs,
            ChOrder, ChDataType,
            ElementSize,
            ImgTy,
            Width, Height, Depth,
            ArraySize,
            RowPitch, SlicePitch,
            NumMipLevels,
            NumSamples,
            Buf,
            HostPtrMode, AccessProt, HostAccessProt),
      Src(Src) { }

  HostAccessibleImage(const HostAccessibleImage &That); // Do not implement.
  void operator=(const HostAccessibleImage &That); // Do not implement.

public:
  const void *GetInitializationData() const { return Src; }
  bool HasInitializationData() const { return Src; }
  
private:
  void *Src;
  
  friend class Context;
};

class DeviceImage : public Image {
public:
  static bool classof(const MemoryObj *MemObj) {
    return MemObj->GetType() == MemoryObj::DeviceImage;
  }
  
private:
  DeviceImage(Context &Ctx,
              size_t Size,
              void *Src,
              TargetDevices &TargetDevs,
              ChannelOrder ChOrder,
              ChannelType ChDataType,
              size_t ElementSize,
              ImgType ImgTy,
              size_t Width,
              size_t Height,
              size_t Depth,
              size_t ArraySize,
              size_t RowPitch,
              size_t SlicePitch,
              unsigned NumMipLevels,
              unsigned NumSamples,
              Buffer *Buf,
              MemoryObj::HostPtrUsageMode HostPtrMode,
              MemoryObj::AccessProtection AccessProt,
              MemoryObj::HostAccessProtection HostAccessProt)
    : Image(MemoryObj::DeviceImage,
            Ctx,
            Size,
            TargetDevs,
            ChOrder, ChDataType,
            ElementSize,
            ImgTy,
            Width, Height, Depth,
            ArraySize,
            RowPitch, SlicePitch,
            NumMipLevels,
            NumSamples,
            Buf,
            HostPtrMode, AccessProt, HostAccessProt),
      Src(Src) { }

  DeviceImage(const DeviceImage &That); // Do not implement.
  void operator=(const DeviceImage &That); // Do not implement.

public:
  const void *GetInitializationData() const { return Src; }
  bool HasInitializationData() const { return Src; }

private:
  void *Src;

  friend class Context;
};

class MemoryObjBuilder {
public:
  enum Type {
    BufferBuilder,
    ImageBuilder
  };

protected:
  MemoryObjBuilder(Type BldTy, Context &Ctx) 
    : BldTy(BldTy),
      Ctx(Ctx),
      ErrCode(CL_SUCCESS),
      Size(0),
      HostPtr(NULL),
      HostPtrMode(MemoryObj::NoHostPtrUsage),
      AccessProt(MemoryObj::InvalidProtection),
      HostAccessProt(MemoryObj::HostNoProtection) { }

public:
  Type GetType() const { return BldTy; }

public:
  MemoryObjBuilder &SetUseHostMemory(bool Enabled, void *Storage);
  MemoryObjBuilder &SetAllocHostMemory(bool Enabled);
  MemoryObjBuilder &SetCopyHostMemory(bool Enabled, void *Src);
  MemoryObjBuilder &SetReadWrite(bool Enabled);
  MemoryObjBuilder &SetWriteOnly(bool Enabled);
  MemoryObjBuilder &SetReadOnly(bool Enabled);
  MemoryObjBuilder &SetHostWriteOnly(bool Enabled);
  MemoryObjBuilder &SetHostReadOnly(bool Enabled);
  MemoryObjBuilder &SetHostNoAccess(bool Enabled);

public:
  MemoryObjBuilder &NotifyError(cl_int ErrCode, const char *Msg = "");

private:
  Type BldTy;

protected:
  Context &Ctx;
  cl_int ErrCode;
  size_t Size;
  void *HostPtr;

  MemoryObj::HostPtrUsageMode HostPtrMode;
  MemoryObj::AccessProtection AccessProt;
  MemoryObj::HostAccessProtection HostAccessProt;
};

class BufferBuilder : public MemoryObjBuilder {
public:
  static bool classof(const MemoryObjBuilder *Bld) {
    return Bld->GetType() == MemoryObjBuilder::BufferBuilder;
  }

public:
  BufferBuilder(Context &Ctx, size_t Size);

public:
  BufferBuilder &SetUseHostMemory(bool Enabled, void *Storage) {
    MemoryObjBuilder::SetUseHostMemory(Enabled, Storage);
    return *this;
  }

  BufferBuilder &SetAllocHostMemory(bool Enabled) {
    MemoryObjBuilder::SetAllocHostMemory(Enabled);
    return *this;
  }

  BufferBuilder &SetCopyHostMemory(bool Enabled, void *Src) {
    MemoryObjBuilder::SetCopyHostMemory(Enabled, Src);
    return *this;
  }

  BufferBuilder &SetReadWrite(bool Enabled) {
    MemoryObjBuilder::SetReadWrite(Enabled);
    return *this;

  }

  BufferBuilder &SetWriteOnly(bool Enabled) {
    MemoryObjBuilder::SetWriteOnly(Enabled);
    return *this;

  }

  BufferBuilder &SetReadOnly(bool Enabled) {
    MemoryObjBuilder::SetReadOnly(Enabled);
    return *this;
  }

  BufferBuilder &SetHostWriteOnly(bool Enabled) {
    MemoryObjBuilder::SetHostWriteOnly(Enabled);
    return *this;
  }

  BufferBuilder &SetHostReadOnly(bool Enabled) {
    MemoryObjBuilder::SetHostReadOnly(Enabled);
    return *this;
  }

  BufferBuilder &SetHostNoAccess(bool Enabled) {
    MemoryObjBuilder::SetHostNoAccess(Enabled);
    return *this;
  }

  Buffer *Create(cl_int *ErrCode = NULL);

private:
  BufferBuilder &NotifyError(cl_int ErrCode, const char *Msg = "") {
    MemoryObjBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }
};

class ImageBuilder : public MemoryObjBuilder {
public:
  static bool classof(const MemoryObjBuilder *Bld) {
    return Bld->GetType() == MemoryObjBuilder::ImageBuilder;
  }

public:
  ImageBuilder(Context &Ctx);
  
public:
  ImageBuilder &SetUseHostMemory(bool Enabled, void *Storage) {
    MemoryObjBuilder::SetUseHostMemory(Enabled, Storage);
    return *this;
  }

  ImageBuilder &SetAllocHostMemory(bool Enabled) {
    MemoryObjBuilder::SetAllocHostMemory(Enabled);
    return *this;
  }

  ImageBuilder &SetCopyHostMemory(bool Enabled, void *Src) {
    MemoryObjBuilder::SetCopyHostMemory(Enabled, Src);
    return *this;
  }

  ImageBuilder &SetReadWrite(bool Enabled) {
    MemoryObjBuilder::SetReadWrite(Enabled);
    return *this;

  }

  ImageBuilder &SetWriteOnly(bool Enabled) {
    MemoryObjBuilder::SetWriteOnly(Enabled);
    return *this;

  }

  ImageBuilder &SetReadOnly(bool Enabled) {
    MemoryObjBuilder::SetReadOnly(Enabled);
    return *this;
  }

  ImageBuilder &SetHostWriteOnly(bool Enabled) {
    MemoryObjBuilder::SetHostWriteOnly(Enabled);
    return *this;
  }

  ImageBuilder &SetHostReadOnly(bool Enabled) {
    MemoryObjBuilder::SetHostReadOnly(Enabled);
    return *this;
  }

  ImageBuilder &SetHostNoAccess(bool Enabled) {
    MemoryObjBuilder::SetHostNoAccess(Enabled);
    return *this;
  }

  ImageBuilder &SetFormat(const cl_image_format *ImgFmt);
  
  ImageBuilder &SetDesc(const cl_image_desc *ImgDesc);

  Image *Create(cl_int *ErrCode = NULL);

private:
  ImageBuilder &NotifyError(cl_int ErrCode, const char *Msg = "") {
    MemoryObjBuilder::NotifyError(ErrCode, Msg);
    return *this;
  }

private:
  Image::TargetDevices TargetDevs;

  Image::ChannelOrder ChOrder;
  Image::ChannelType ChDataType;

  size_t ElementSize;

  Image::ImgType ImgTy;

  size_t Width;
  size_t Height;
  size_t Depth;

  size_t ArraySize;

  size_t HostRowPitch;
  size_t HostSlicePitch;

  unsigned NumMipLevels;
  unsigned NumSamples;

  Buffer *Buf;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_MEMORYOBJ_H
