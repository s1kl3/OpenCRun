
#include "CL/opencl.h"

#include "Utils.h"

#include "opencrun/Core/Event.h"

CL_API_ENTRY cl_int CL_API_CALL
clGetEventProfilingInfo(cl_event event,
                        cl_profiling_info param_name,
                        size_t param_value_size,
                        void *param_value,
                        size_t *param_value_size_ret)
CL_API_SUFFIX__VERSION_1_0 {
  if(!event)
    return CL_INVALID_EVENT;

  opencrun::Event &Ev = *llvm::cast<opencrun::Event>(event);
  opencrun::InternalEvent *InternalEv = llvm::dyn_cast<opencrun::InternalEvent>(&Ev);

  if(!InternalEv ||
     InternalEv->GetStatus() != CL_COMPLETE ||
     !InternalEv->GetCommandQueue().ProfilingEnabled())
    return CL_PROFILING_INFO_NOT_AVAILABLE;

  switch(param_name) {
  #define PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)   \
  case PARAM: {                                    \
    return clFillValue<PARAM_TY, FUN_TY>(          \
             static_cast<PARAM_TY *>(param_value), \
             InternalEv->FUN(),                    \
             param_value_size,                     \
             param_value_size_ret);                \
  }
  #include "ProfilingProperties.def"
  #undef PROPERTY

  default:
    return CL_INVALID_VALUE;
  }
}
