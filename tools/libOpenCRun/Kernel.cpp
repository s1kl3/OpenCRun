
#include "CL/opencl.h"

#include "opencrun/Core/Kernel.h"

#include "Utils.h"

inline bool clValidKernelDevicePair(opencrun::Kernel *Kern,
                                    opencrun::Device *Dev) {
  const opencrun::KernelDescriptor &Desc = Kern->getDescriptor();
  return (!Dev && Desc.isBuiltForSingleDevice()) ||
         (Dev && Desc.isBuiltForDevice(Dev));
}

CL_API_ENTRY cl_kernel CL_API_CALL
clCreateKernel(cl_program program,
               const char *kernel_name,
               cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!program)
    RETURN_WITH_ERROR(errcode_ret, CL_INVALID_PROGRAM);

  opencrun::Program &Prog = *llvm::cast<opencrun::Program>(program);

  if(!kernel_name)
    RETURN_WITH_ERROR(errcode_ret, CL_INVALID_VALUE);

  llvm::StringRef KernName(kernel_name);

  opencrun::Kernel *Kern = Prog.CreateKernel(KernName, errcode_ret);
  
  if(Kern)
    Kern->Retain();

  return Kern;
}

CL_API_ENTRY cl_int CL_API_CALL
clCreateKernelsInProgram(cl_program program,
                         cl_uint num_kernels,
                         cl_kernel *kernels,
                         cl_uint *num_kernels_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!program)
    return CL_INVALID_PROGRAM;

  opencrun::Program &Prog = *llvm::cast<opencrun::Program>(program);

  return Prog.CreateKernelsInProgram(num_kernels, kernels, num_kernels_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainKernel(cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0 {
  if(!kernel)
    return CL_INVALID_KERNEL;

  opencrun::Kernel &Kern = *llvm::cast<opencrun::Kernel>(kernel);
  Kern.Retain();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0 {
  if(!kernel)
    return CL_INVALID_KERNEL;

  opencrun::Kernel &Kern = *llvm::cast<opencrun::Kernel>(kernel);
  Kern.Release();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(cl_kernel kernel,
               cl_uint arg_index,
               size_t arg_size,
               const void *arg_value) CL_API_SUFFIX__VERSION_1_0 {
  if(!kernel)
    return CL_INVALID_KERNEL;

  opencrun::Kernel &Kern = *llvm::cast<opencrun::Kernel>(kernel);

  return Kern.SetArg(arg_index, arg_size, arg_value);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelInfo(cl_kernel kernel,
                cl_kernel_info param_name,
                size_t param_value_size,
                void *param_value,
                size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!kernel)
    return CL_INVALID_KERNEL;

  opencrun::Kernel &Kern = *llvm::cast<opencrun::Kernel>(kernel);
  switch(param_name) {
  #define PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)   \
  case PARAM:                                      \
    return clFillValue<PARAM_TY, FUN_TY>(          \
             static_cast<PARAM_TY *>(param_value), \
             Kern.FUN(),                           \
             param_value_size,                     \
             param_value_size_ret);
  #define DS_PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)
  #define KA_PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)
  #include "KernelProperties.def"
  #undef PROPERTY
  #undef DS_PROPERTY
  #undef KA_PROPERTY

  default:
    return CL_INVALID_VALUE;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelArgInfo(cl_kernel kernel,
                   cl_uint arg_indx,
                   cl_kernel_arg_info param_name,
                   size_t param_value_size,
                   void *param_value,
                   size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_2 {
  if(!kernel)
    return CL_INVALID_KERNEL;

  opencrun::Kernel &Kern = *llvm::cast<opencrun::Kernel>(kernel);
  switch(param_name) {
  #define KA_PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY) \
  case PARAM:                                       \
    return clFillValue<PARAM_TY, FUN_TY>(           \
             static_cast<PARAM_TY *>(param_value),  \
             Kern.FUN(arg_indx),                    \
             param_value_size,                      \
             param_value_size_ret);
  #define PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)
  #define DS_PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)
  #include "KernelProperties.def"
  #undef KA_PROPERTY
  #undef PROPERTY
  #undef DS_PROPERTY

  default:
    return CL_INVALID_VALUE;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelWorkGroupInfo(cl_kernel kernel,
                         cl_device_id device,
                         cl_kernel_work_group_info param_name,
                         size_t param_value_size,
                         void *param_value,
                         size_t *param_value_size_ret)
CL_API_SUFFIX__VERSION_1_0 {
  opencrun::Device *Dev = llvm::cast<opencrun::Device>(device);

  if(!kernel)
    return CL_INVALID_KERNEL;

  opencrun::Kernel &Kern = *llvm::cast<opencrun::Kernel>(kernel);
  switch(param_name) {
  #define DS_PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)  \
  case PARAM: {                                      \
    FUN_TY FunRet;                                   \
    if(!Kern.getDescriptor().FUN(FunRet, Dev))       \
      return CL_INVALID_DEVICE;                      \
    else                                             \
      return clFillValue<PARAM_TY, FUN_TY>(          \
               static_cast<PARAM_TY *>(param_value), \
               FunRet,                               \
               param_value_size,                     \
               param_value_size_ret);                \
  }
  #define PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)
  #define KA_PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)
  #include "KernelProperties.def"
  #undef DS_PROPERTY
  #undef PROPERTY
  #undef KA_PROPERTY

  // This property does not depended on the device, however we must ensure that
  // the device is associated with the kernel.
  case CL_KERNEL_COMPILE_WORK_GROUP_SIZE: {
    if(!clValidKernelDevicePair(&Kern, Dev))
      return CL_INVALID_DEVICE;

    llvm::SmallVector<size_t, 4> Sizes;
    Kern.getDescriptor().getRequiredWorkGroupSizes(Sizes, Dev); 

    return clFillValue<size_t, llvm::SmallVector<size_t, 4> &>(
             static_cast<size_t *>(param_value),
             Sizes,
             param_value_size,
             param_value_size_ret);
  }
  // This property depends on the device, but not on the kernel, however we must
  // ensure that the device is associated with the kernel.
  case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
    if(!clValidKernelDevicePair(&Kern, Dev))
      return CL_INVALID_DEVICE;

    return clFillValue<size_t, size_t>(
             static_cast<size_t *>(param_value),
             Dev->GetPreferredWorkGroupSizeMultiple(),
             param_value_size,
             param_value_size_ret);

  default:
    return CL_INVALID_VALUE;
  }
}
