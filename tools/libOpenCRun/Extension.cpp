
#include "CL/opencl.h"

CL_API_ENTRY void * CL_API_CALL 
clGetExtensionFunctionAddressForPlatform(cl_platform_id platform,
                                         const char *func_name) CL_API_SUFFIX__VERSION_1_2 {
  return 0;
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED void * CL_API_CALL
clGetExtensionFunctionAddress(const char *func_name) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED {
  return 0;
}
