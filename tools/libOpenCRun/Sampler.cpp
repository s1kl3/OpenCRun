
#include "CL/opencl.h"

#include "Utils.h"

#include "opencrun/Core/Sampler.h"

#define CL_ADDRESSING_MODE_ALL  \
  (CL_ADDRESS_NONE |            \
   CL_ADDRESS_CLAMP_TO_EDGE |   \
   CL_ADDRESS_CLAMP |           \
   CL_ADDRESS_REPEAT |          \
   CL_ADDRESS_MIRRORED_REPEAT)

#define CL_FILTER_MODE_ALL  \
  (CL_FILTER_NEAREST |      \
   CL_FILTER_LINEAR)

static inline bool clValidAddressingMode(cl_addressing_mode addressing_mode) {
  return !(addressing_mode & ~CL_ADDRESSING_MODE_ALL);
}

static inline bool clValidFilterMode(cl_filter_mode filter_mode) {
  return !(filter_mode & ~CL_FILTER_MODE_ALL);
}

CL_API_ENTRY cl_sampler CL_API_CALL
clCreateSampler(cl_context context,
                cl_bool normalized_coords,
                cl_addressing_mode addressing_mode,
                cl_filter_mode filter_mode,
                cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!context)
    RETURN_WITH_ERROR(errcode_ret, CL_INVALID_CONTEXT);

  opencrun::Context &Ctx = *llvm::cast<opencrun::Context>(context);

  bool IsInvalidOp = true;
  for(opencrun::Context::device_iterator I = Ctx.device_begin(),
                                         E = Ctx.device_end(); 
                                         I != E;
                                         ++I) {
    if((*I)->HasImageSupport()) {
      IsInvalidOp = false;
      break;
    }
  }
  
  if(IsInvalidOp)
    RETURN_WITH_ERROR(errcode_ret, CL_INVALID_OPERATION);

  

  opencrun::SamplerBuilder Bld(Ctx);
  opencrun::Sampler *Smplr;

  Smplr = Bld.SetNormalizedCoords(normalized_coords)
             .SetAddressNone(addressing_mode & CL_ADDRESS_NONE)
             .SetAddressClampToEdge(addressing_mode & CL_ADDRESS_CLAMP_TO_EDGE)
             .SetAddressClamp(addressing_mode & CL_ADDRESS_CLAMP)
             .SetAddressRepeat(addressing_mode & CL_ADDRESS_REPEAT)
             .SetAddressMirroredRepeat(addressing_mode & CL_ADDRESS_MIRRORED_REPEAT)
             .SetFilterNearest(filter_mode & CL_FILTER_NEAREST)
             .SetFilterLinear(filter_mode & CL_FILTER_LINEAR)
             .Create(errcode_ret);

  if(Smplr)
    Smplr->Retain();

  return Smplr;  
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainSampler(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0 {
  if(!sampler)
    return CL_INVALID_SAMPLER;

  opencrun::Sampler &Smplr = *llvm::cast<opencrun::Sampler>(sampler);
  Smplr.Retain();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseSampler(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0 {
  if(!sampler)
    return CL_INVALID_SAMPLER;

  opencrun::Sampler &Smplr = *llvm::cast<opencrun::Sampler>(sampler);
  Smplr.Release();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSamplerInfo(cl_sampler sampler,
                 cl_sampler_info param_name,
                 size_t param_value_size,
                 void *param_value,
                 size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!sampler)
    return CL_INVALID_SAMPLER;

  opencrun::Sampler &Smplr = *llvm::cast<opencrun::Sampler>(sampler);
  switch(param_name) {
  #define PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)   \
  case PARAM:                                      \
    return clFillValue<PARAM_TY, FUN_TY>(          \
             static_cast<PARAM_TY *>(param_value), \
             Smplr.FUN(),                          \
             param_value_size,                     \
             param_value_size_ret);
  #include "SamplerProperties.def"
  #undef PROPERTY

  default:
    return CL_INVALID_VALUE;
  }
}
