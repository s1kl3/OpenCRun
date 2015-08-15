#ifndef OPENCRUN_CORE_MEMORYOBJECT_H
#define OPENCRUN_CORE_MEMORYOBJECT_H

#include "CL/opencl.h"

#include "opencrun/Util/MTRefCounted.h"

#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Mutex.h"

#include <unordered_map>

struct _cl_mem {};

namespace opencrun {
class Context;
class Device;
class Image;

class MemoryObject : public _cl_mem, public MTRefCountedBaseVPTR<MemoryObject> {
public:
  enum Kind {
    K_Buffer,
    K_Image
  };

  enum HostPtrMode : uint16_t {
    UseHostPtr = CL_MEM_USE_HOST_PTR,
    AllocHostPtr = CL_MEM_ALLOC_HOST_PTR,
    CopyHostPtr = CL_MEM_COPY_HOST_PTR,

    HostPtrModeMask = UseHostPtr | AllocHostPtr | CopyHostPtr
  };

  enum AccessProtection : uint16_t {
    ReadWrite = CL_MEM_READ_WRITE,
    WriteOnly = CL_MEM_WRITE_ONLY,
    ReadOnly = CL_MEM_READ_ONLY,

    AccessProtectionMask = ReadWrite | WriteOnly | ReadOnly,
    CanReadMask = ReadOnly | ReadWrite,
    CanWriteMask = WriteOnly | ReadWrite
  };

  enum HostAccessProtection : uint16_t {
    HostWriteOnly = CL_MEM_HOST_WRITE_ONLY,
    HostReadOnly = CL_MEM_HOST_READ_ONLY,
    HostNoAccess = CL_MEM_HOST_NO_ACCESS,

    HostAccessProtectionMask = HostNoAccess | HostWriteOnly | HostReadOnly,
    HostCanReadOrWriteMask = HostReadOnly | HostWriteOnly
  };

  struct MemMappingInfo {
    size_t Offset[3];
    size_t Size[3];
    cl_mem_flags Flags;

    bool isOverlappedWith(const MemMappingInfo &Info) const;
  };

public:
  static bool classof(const _cl_mem *Obj) { return true; }

public:
  virtual ~MemoryObject();

  Kind getKind() const { return MemObjKind; }

  Context &getContext() const { return *Ctx; }

  size_t getSize() const { return Size; }

  uint16_t getFlags() const { return Flags; }

  bool hasHostPtr() const { return HostPtr != nullptr; }
  void *getHostPtr() const { return HostPtr; }

  bool addMapping(void *Ptr, const MemMappingInfo &Info);
  bool removeMapping(void *Ptr);

  bool isValidMapping(void *Ptr) const;
  unsigned getNumMappings() const;

protected:
  MemoryObject(Context &Ctx, Kind K, size_t Size, void *HostPtr, unsigned Flags,
               MemoryObject *ParentObj, size_t ParentOffset)
   : MemObjKind(K), Size(Size), Ctx(&Ctx), HostPtr(HostPtr), Flags(Flags),
     Parent(ParentObj), ParentOffset(ParentOffset) {
  }

  size_t getParentOffset() const { return ParentOffset; }
  MemoryObject *getParentObject() const { return Parent.getPtr(); }

  template<typename Ty> Ty *getParentObject() const {
    return llvm::cast_or_null<Ty>(Parent.getPtr());
  }

private:
  mutable llvm::sys::Mutex ThisLock;

  Kind MemObjKind;
  size_t Size;
  llvm::IntrusiveRefCntPtr<Context> Ctx;

  void *HostPtr;
  unsigned Flags;

  llvm::IntrusiveRefCntPtr<MemoryObject> Parent;
  size_t ParentOffset;

private:
  using MemMappingsContainer =
    std::unordered_multimap<const void *, MemMappingInfo>;

  MemMappingsContainer MemMappings;
};

class Buffer : public MemoryObject {
public:
  static bool classof(const _cl_mem *Obj) {
    return classof(static_cast<const MemoryObject*>(Obj));
  }

  static bool classof(const MemoryObject *Obj) {
    return Obj->getKind() == K_Buffer;
  }

public:
  Buffer(Context &Ctx, size_t Size, void *HostPtr, unsigned Flags)
   : MemoryObject(Ctx, K_Buffer, Size, HostPtr, Flags, nullptr, 0) {}

  Buffer(Context &Ctx, Buffer &Parent, size_t Origin,
         size_t Size, unsigned Flags)
   : MemoryObject(Ctx, K_Buffer, Size, computeHostPtr(Parent, Origin),
                  Flags, &Parent, Origin) {}

  bool isSubBuffer() const { return getParent() != nullptr; }
  Buffer *getParent() const { return getParentObject<Buffer>(); }

  size_t getOrigin() const { return getParentOffset(); }

private:
  static void *computeHostPtr(Buffer &P, size_t O);
};

class Image : public MemoryObject {
public:
  enum Type : uint16_t {
    Image1D        = CL_MEM_OBJECT_IMAGE1D,
    Image1D_Array  = CL_MEM_OBJECT_IMAGE1D_ARRAY,
    Image1D_Buffer = CL_MEM_OBJECT_IMAGE1D_BUFFER,
    Image2D        = CL_MEM_OBJECT_IMAGE2D,
    Image2D_Array  = CL_MEM_OBJECT_IMAGE2D_ARRAY,
    Image3D        = CL_MEM_OBJECT_IMAGE3D
  };

  enum ChannelOrder : uint16_t {
    Channel_R         = CL_R,
    Channel_A         = CL_A,
    Channel_RG        = CL_RG,
    Channel_RA        = CL_RA,
    Channel_RGB       = CL_RGB,
    Channel_RGBA      = CL_RGBA,
    Channel_BGRA      = CL_BGRA,
    Channel_ARGB      = CL_ARGB,
    Channel_Intensity = CL_INTENSITY,
    Channel_Luminance = CL_LUMINANCE,
    Channel_Rx        = CL_Rx,
    Channel_RGx       = CL_RGx,
    Channel_RGBx      = CL_RGBx
  };

  enum ChannelDataType : uint16_t {
    DataType_SNorm_Int8       = CL_SNORM_INT8,
    DataType_SNorm_Int16      = CL_SNORM_INT16,
    DataType_UNorm_Int8       = CL_UNORM_INT8,
    DataType_UNorm_Int16      = CL_UNORM_INT16,
    DataType_UNorm_Short_565  = CL_UNORM_SHORT_565,
    DataType_UNorm_Short_555  = CL_UNORM_SHORT_555,
    DataType_UNorm_Int_101010 = CL_UNORM_INT_101010,
    DataType_Signed_Int8      = CL_SIGNED_INT8,
    DataType_Signed_Int16     = CL_SIGNED_INT16,
    DataType_Signed_Int32     = CL_SIGNED_INT32,
    DataType_Unsigned_Int8    = CL_UNSIGNED_INT8,
    DataType_Unsigned_Int16   = CL_UNSIGNED_INT16,
    DataType_Unsigned_Int32   = CL_UNSIGNED_INT32,
    DataType_Half_Float       = CL_HALF_FLOAT,
    DataType_Float            = CL_FLOAT
  };

  struct Descriptor {
    size_t Width;
    size_t Height;
    size_t Depth;
    size_t ArraySize;
    size_t RowPitch;
    size_t SlicePitch;
  };

public:
  static bool classof(const _cl_mem *Obj) {
    return classof(static_cast<const MemoryObject*>(Obj));
  }

  static bool classof(const MemoryObject *Obj) {
    return Obj->getKind() == K_Image;
  }

public:
  Image(Context &Ctx, size_t Size, void *HostPtr, unsigned Flags,
        Type IT, ChannelOrder CO, ChannelDataType CT,
        const Descriptor &Desc, Buffer *Buf)
   : MemoryObject(Ctx, K_Image, Size, HostPtr, Flags, Buf, 0),
     ImgTy(IT), ChOrder(CO), ChDataType(CT), Desc(Desc) {
    computePixelFeatures();
  }

public:
  Type getType() const { return ImgTy; }
  ChannelOrder getChannelOrder() const { return ChOrder; }
  ChannelDataType getChannelDataType() const { return ChDataType; }

  cl_image_format getImageFormat() const {
    return { ChOrder, ChDataType };
  }

  size_t getWidth() const { return Desc.Width; }
  size_t getHeight() const { return Desc.Height; }
  size_t getDepth() const { return Desc.Depth; }
  size_t getArraySize() const { return Desc.ArraySize; }

  size_t getRowPitch() const { return Desc.RowPitch; }
  size_t getSlicePitch() const { return Desc.SlicePitch; }

  Buffer *getBuffer() { return getParentObject<Buffer>(); }
  const Buffer *getBuffer() const { return getParentObject<Buffer>(); }

  unsigned getElementSize() const { return ElementSize; }
  unsigned getNumChannels() const { return NumChannels; }

  unsigned getNumMipLevels() const { return 0; }
  unsigned getNumSamples() const { return 0; }

private:
  void computePixelFeatures();

private:
  Type ImgTy;
  ChannelOrder ChOrder;
  ChannelDataType ChDataType;
  uint8_t ElementSize;
  uint8_t NumChannels;
  Descriptor Desc;

};

class MemoryObjectBuilder {
public:
  enum Kind {
    K_Buffer,
    K_SubBuffer,
    K_Image
  };

  explicit MemoryObjectBuilder(Kind K);

  void setSubBufferParams(cl_mem Buf, cl_buffer_create_type BCT,
                          const void *BR);
  void setSubBufferFlags(cl_mem_flags Flags);

  void setContext(cl_context Ctx);
  void setBufferSize(size_t Size);
  void setImage(const cl_image_format *Fmt, const cl_image_desc *Desc);
  void setGenericFlags(cl_mem_flags Flags, void *HostPtr);

  std::unique_ptr<MemoryObject> create(cl_int *ErrCode);

private:
  void error(cl_int E, llvm::StringRef Msg);

private:
  Kind ObjKind;
  Context *Ctx;
  uint16_t Flags;
  void *HostPtr;
  union {
    struct {
      size_t Size;
    } Buf;

    struct {
      Buffer *Parent;
      size_t Origin;
      size_t Size;
    } SubBuf;

    struct {
      Image::Type Type;
      Image::ChannelOrder ChOrder;
      Image::ChannelDataType ChDataType;
      Image::Descriptor Desc;
      Buffer *Buf;
      size_t Size;
    } Img;
  };
  cl_int Err;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_MEMORYOBJECT_H
