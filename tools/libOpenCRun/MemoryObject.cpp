
#include "CL/opencl.h"

#include "Utils.h"

#include "opencrun/Core/MemoryObject.h"

#include <algorithm>

#define CL_MEM_FIELD_ALL    \
  (CL_MEM_READ_WRITE |      \
   CL_MEM_WRITE_ONLY |      \
   CL_MEM_READ_ONLY |       \
   CL_MEM_USE_HOST_PTR |    \
   CL_MEM_ALLOC_HOST_PTR |  \
   CL_MEM_COPY_HOST_PTR |   \
   CL_MEM_HOST_WRITE_ONLY | \
   CL_MEM_HOST_READ_ONLY |  \
   CL_MEM_HOST_NO_ACCESS)

#define CL_IMG_TYPE_ALL             \
  (CL_MEM_OBJECT_IMAGE1D |          \
   CL_MEM_OBJECT_IMAGE1D_BUFFER |   \
   CL_MEM_OBJECT_IMAGE1D_ARRAY |    \
   CL_MEM_OBJECT_IMAGE2D |          \
   CL_MEM_OBJECT_IMAGE2D_ARRAY |    \
   CL_MEM_OBJECT_IMAGE3D)

static inline bool clValidMemField(cl_mem_flags flags) {
  return !(flags & ~CL_MEM_FIELD_ALL);
}

static inline bool clValidImgType(cl_mem_object_type image_type) {
  return !(image_type & ~CL_IMG_TYPE_ALL);
}

static bool ImgFmtCmp(cl_image_format fmt_1, cl_image_format fmt_2) {
  if(fmt_1.image_channel_order < fmt_2.image_channel_order)
    return true;
  else if(fmt_1.image_channel_order > fmt_2.image_channel_order)
    return false;
  else {
    if(fmt_1.image_channel_data_type < fmt_2.image_channel_data_type)
      return true;
    else
      return false;
  }
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context context,
               cl_mem_flags flags,
               size_t size,
               void *host_ptr,
               cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  using MemObjBuilder = opencrun::MemoryObjectBuilder;

  MemObjBuilder Bld(MemObjBuilder::K_Buffer);
  Bld.setContext(context);
  Bld.setBufferSize(size);
  Bld.setGenericFlags(flags, host_ptr);
  auto Buf = Bld.create(errcode_ret);

  if (Buf)
    Buf->Retain();

  return Buf.release();
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateSubBuffer(cl_mem buffer,
                  cl_mem_flags flags,
                  cl_buffer_create_type buffer_create_type,
                  const void *buffer_create_info,
                  cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_1 {
  using MemObjBuilder = opencrun::MemoryObjectBuilder;

  MemObjBuilder Bld(MemObjBuilder::K_Buffer);
  Bld.setSubBufferParams(buffer, buffer_create_type, buffer_create_info);
  Bld.setSubBufferFlags(flags);
  auto SubBuf = Bld.create(errcode_ret);

  if (SubBuf)
    SubBuf->Retain();

  return SubBuf.release();
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage(cl_context context,
              cl_mem_flags flags,
              const cl_image_format *image_format,
              const cl_image_desc *image_desc, 
              void *host_ptr,
              cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_2 {
  using MemObjBuilder = opencrun::MemoryObjectBuilder;

  MemObjBuilder Bld(MemObjBuilder::K_Image);
  Bld.setContext(context);
  Bld.setImage(image_format, image_desc);
  Bld.setGenericFlags(flags, host_ptr);
  auto Img = Bld.create(errcode_ret);

  if (Img)
    Img->Retain();

  return Img.release();
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
clCreateImage2D(cl_context context,
                cl_mem_flags flags,
                const cl_image_format *image_format,
                size_t image_width,
                size_t image_height,
                size_t image_row_pitch,
                void *host_ptr,
                cl_int *errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED {
  cl_image_desc image_desc;

  image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  image_desc.image_width = image_width;
  image_desc.image_height = image_height;
  image_desc.image_depth = 0;
  image_desc.image_array_size = 1;
  image_desc.image_row_pitch = image_row_pitch;
  image_desc.image_slice_pitch = 0;
  image_desc.num_mip_levels = 0;
  image_desc.num_samples = 0;
  image_desc.buffer = NULL;

  return clCreateImage(context,
                       flags,
                       image_format,
                       &image_desc,
                       host_ptr,
                       errcode_ret);
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_API_CALL
clCreateImage3D(cl_context context,
                cl_mem_flags flags,
                const cl_image_format *image_format,
                size_t image_width,
                size_t image_height,
                size_t image_depth,
                size_t image_row_pitch,
                size_t image_slice_pitch,
                void *host_ptr,
                cl_int *errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED {
  cl_image_desc image_desc;

  image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
  image_desc.image_width = image_width;
  image_desc.image_height = image_height;
  image_desc.image_depth = image_depth;
  image_desc.image_array_size = 1;
  image_desc.image_row_pitch = image_row_pitch;
  image_desc.image_slice_pitch = image_slice_pitch;
  image_desc.num_mip_levels = 0;
  image_desc.num_samples = 0;
  image_desc.buffer = NULL;

  return clCreateImage(context,
                       flags,
                       image_format,
                       &image_desc,
                       host_ptr,
                       errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {
  if(!memobj)
    return CL_INVALID_MEM_OBJECT;

  opencrun::MemoryObject &MemObj = *llvm::cast<opencrun::MemoryObject>(memobj);
  MemObj.Retain();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {
  if(!memobj)
    return CL_INVALID_MEM_OBJECT;

  opencrun::MemoryObject &MemObj = *llvm::cast<opencrun::MemoryObject>(memobj);
  MemObj.Release();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSupportedImageFormats(cl_context context,
                           cl_mem_flags flags,
                           cl_mem_object_type image_type,
                           cl_uint num_entries,
                           cl_image_format *image_formats,
                           cl_uint *num_image_formats)
CL_API_SUFFIX__VERSION_1_0 {
  if(!context)
    return CL_INVALID_CONTEXT;

  opencrun::Context &Ctx = *llvm::cast<opencrun::Context>(context);

  if((num_entries == 0 && image_formats))
    return CL_INVALID_VALUE;

  if(!clValidMemField(flags) || !clValidImgType(image_type))
    return CL_INVALID_VALUE;

  llvm::SmallVector<llvm::ArrayRef<cl_image_format>, 4> DevsImgFmts;
  std::vector<cl_image_format> ImgFmts;

  for(opencrun::Context::device_iterator I = Ctx.device_begin(),
                                         E = Ctx.device_end(); 
                                         I != E;
                                         ++I) {
    llvm::ArrayRef<cl_image_format> CurDevImgFmts = (*I)->GetSupportedImageFormats();

    if(CurDevImgFmts.size())
      DevsImgFmts.push_back(CurDevImgFmts);
  }

  for(unsigned int I = 0; I < DevsImgFmts.size(); ++I) {
    for(unsigned int J = 0; J < DevsImgFmts[I].size(); ++J) { 
      const cl_image_format &CurFmt = DevsImgFmts[I][J];

      bool FmtFound = false;
      for(unsigned int K = 0; !FmtFound && K < ImgFmts.size(); ++K)
        if((CurFmt.image_channel_order == ImgFmts[K].image_channel_order) &&
           (CurFmt.image_channel_data_type == ImgFmts[K].image_channel_data_type))
          FmtFound = true;

      if(!FmtFound)
        ImgFmts.push_back(CurFmt);
    }
  }

  // Collected image formats reordered.
  std::sort(ImgFmts.begin(), ImgFmts.end(), ImgFmtCmp);

  if(num_image_formats)
    *num_image_formats = static_cast<cl_uint>(ImgFmts.size());

  if(image_formats && (ImgFmts.size() <= num_entries))
    for(unsigned I = 0; I < num_entries && I < ImgFmts.size(); ++I)
      *image_formats++ = ImgFmts[I];

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetMemObjectInfo(cl_mem memobj,
                   cl_mem_info param_name,
                   size_t param_value_size,
                   void *param_value,
                   size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!memobj)
    return CL_INVALID_MEM_OBJECT;

  opencrun::MemoryObject &MemObj = *llvm::cast<opencrun::MemoryObject>(memobj);
  switch(param_name) {
  #define PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)   \
  case PARAM:                                      \
    return clFillValue<PARAM_TY, FUN_TY>(          \
             static_cast<PARAM_TY *>(param_value), \
             MemObj.FUN(),                         \
             param_value_size,                     \
             param_value_size_ret);
  #define IMG_PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)
  #include "MemoryObjectProperties.def"
  #undef PROPERTY
  #undef IMG_PROPERTY

  case CL_MEM_TYPE: {
    if(llvm::isa<opencrun::Buffer>(MemObj))
      return clFillValue<cl_mem_object_type, cl_mem_object_type>(
               static_cast<cl_mem_object_type *>(param_value),
               CL_MEM_OBJECT_BUFFER,
               param_value_size,
               param_value_size_ret);

    if(opencrun::Image *Img = llvm::dyn_cast<opencrun::Image>(&MemObj))
      return clFillValue<cl_mem_object_type, opencrun::Image::Type>(
               static_cast<cl_mem_object_type *>(param_value),
               Img->getType(),
               param_value_size,
               param_value_size_ret);

    return CL_INVALID_MEM_OBJECT;
  }

  case CL_MEM_HOST_PTR:
    return clFillValue<void *, void *>(
             static_cast<void **>(param_value),
             MemObj.getHostPtr(),
             param_value_size,
             param_value_size_ret);

  case CL_MEM_ASSOCIATED_MEMOBJECT: {
    if(opencrun::Buffer *Buf = llvm::dyn_cast<opencrun::Buffer>(&MemObj))
      return clFillValue<cl_mem, opencrun::MemoryObject *>(
               static_cast<cl_mem *>(param_value),
               Buf->getParent(),
               param_value_size,
               param_value_size_ret);

    return clFillValue<cl_mem, opencrun::MemoryObject *>(
             static_cast<cl_mem *>(param_value),
             NULL,
             param_value_size,
             param_value_size_ret);
  }

  case CL_MEM_OFFSET: {
    if(opencrun::Buffer *Buf = llvm::dyn_cast<opencrun::Buffer>(&MemObj))
      return clFillValue<size_t, size_t>(
               static_cast<size_t *>(param_value),
               Buf->getOrigin(),
               param_value_size,
               param_value_size_ret);

    return clFillValue<size_t, size_t>(
             static_cast<size_t *>(param_value),
             0,
             param_value_size,
             param_value_size_ret);
  }
 
  default:
    return CL_INVALID_VALUE;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clGetImageInfo(cl_mem image,
               cl_image_info param_name,
               size_t param_value_size,
               void *param_value,
               size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!image)
    return CL_INVALID_MEM_OBJECT;

  opencrun::Image &Img = *llvm::cast<opencrun::Image>(image);
  switch(param_name) {
  #define IMG_PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)    \
  case PARAM:                                           \
    return clFillValue<PARAM_TY, FUN_TY>(               \
             static_cast<PARAM_TY *>(param_value),      \
             Img.FUN(),                                 \
             param_value_size,                          \
             param_value_size_ret);
  #define PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)
  #include "MemoryObjectProperties.def"
  #undef IMG_PROPERTY
  #undef PROPERTY

  default:
    return CL_INVALID_VALUE;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clSetMemObjectDestructorCallback(cl_mem memobj,
                                 void (CL_CALLBACK *pfn_notify)
                                        (cl_mem memobj, void* user_data),
                                 void *user_data) CL_API_SUFFIX__VERSION_1_1 {
  return CL_SUCCESS;
}
