
#include "CL/opencl.h"

#include "opencrun/Core/Context.h"
#include "opencrun/Core/Program.h"

#include "Utils.h"

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithSource(cl_context context,
                          cl_uint count,
                          const char **strings,
                          const size_t *lengths,
                          cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!context)
    RETURN_WITH_ERROR(errcode_ret, CL_INVALID_CONTEXT);

  opencrun::Context &Ctx = *llvm::cast<opencrun::Context>(context);

  opencrun::ProgramBuilder Bld(Ctx);
  opencrun::Program *Prog = Bld.SetSources(count, strings, lengths)
                               .Create(errcode_ret);

  if(Prog)
    Prog->Retain();

  return Prog;
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBinary(cl_context context,
                          cl_uint num_devices,
                          const cl_device_id *device_list,
                          const size_t *lengths,
                          const unsigned char **binaries,
                          cl_int *binary_status,
                          cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  llvm_unreachable("Not yet implemented");
  return 0;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0 {
  if(!program)
    return CL_INVALID_PROGRAM;

  opencrun::Program &Prog = *llvm::cast<opencrun::Program>(program);
  Prog.Retain();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0 {
  if(!program)
    return CL_INVALID_PROGRAM;

  opencrun::Program &Prog = *llvm::cast<opencrun::Program>(program);
  Prog.Release();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(cl_program program,
               cl_uint num_devices,
               const cl_device_id *device_list,
               const char *options,
               void (CL_CALLBACK *pfn_notify)(cl_program program,
                                              void *user_data),
               void *user_data) CL_API_SUFFIX__VERSION_1_0 {
  if(!program)
    return CL_INVALID_PROGRAM;

  opencrun::Program &Prog = *llvm::cast<opencrun::Program>(program);

  if((!device_list && num_devices) || (device_list && !num_devices))
    return CL_INVALID_VALUE;

  opencrun::Program::DevicesContainer Devs;
  for(unsigned I = 0; I < num_devices; ++I) {
    if(!device_list[I])
      return CL_INVALID_DEVICE;

    Devs.push_back(llvm::cast<opencrun::Device>(device_list[I]));
  }

  if(!pfn_notify && user_data)
    return CL_INVALID_VALUE;

  llvm::StringRef Opts = options ? llvm::StringRef(options) : llvm::StringRef();
  opencrun::CompilerCallbackClojure Callback(pfn_notify, user_data);

  return Prog.Build(Devs, Opts, Callback);
}

CL_API_ENTRY cl_int CL_API_CALL
clUnloadCompiler(void) CL_API_SUFFIX__VERSION_1_0 {
  llvm_unreachable("Not yet implemented");
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramInfo(cl_program program,
                 cl_program_info param_name,
                 size_t param_value_size,
                 void *param_value,
                 size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  llvm_unreachable("Not yet implemented");
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramInfo(cl_program program,
                 cl_program_info param_name,
                 size_t param_value_size,
                 void *param_value,
                 size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!program)
    return CL_INVALID_PROGRAM;

  opencrun::Program &Prog = *llvm::cast<opencrun::Program>(program);

  switch(param_name) {
  #define PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)   \
  case PARAM: {                                    \
    return clFillValue<PARAM_TY, FUN_TY>(          \
             static_cast<PARAM_TY *>(param_value), \
             Prog.FUN(),                           \
             param_value_size,                     \
             param_value_size_ret);                \
  }
  #define DS_PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)
  #include "ProgramProperties.def"
  #undef PROPERTY
  #undef DS_PROPERTY

  case CL_PROGRAM_NUM_DEVICES: {
    opencrun::Context &Ctx = Prog.GetContext();

    return clFillValue<cl_uint, opencrun::Context::DevicesContainer::size_type>(
            static_cast<cl_uint *>(param_value),
            Ctx.devices_size(),
            param_value_size,
            param_value_size_ret);
  }

  case CL_PROGRAM_DEVICES: {
    opencrun::Context &Ctx = Prog.GetContext();
    
    return clFillValue<cl_device_id, opencrun::Context::device_iterator>(
            static_cast<cl_device_id *>(param_value),
            Ctx.device_begin(),
            Ctx.device_end(),
            param_value_size,
            param_value_size_ret);
  }

  case CL_PROGRAM_NUM_KERNELS: {
    if(!Prog.HasAttachedKernels())
      return CL_INVALID_PROGRAM_EXECUTABLE;
    
    opencrun::Program::AttachedKernelsContainer &AttachedKernels =
      Prog.GetAttachedKernels();

    return clFillValue<size_t, unsigned>(
            static_cast<size_t *>(param_value),
            AttachedKernels.size(),
            param_value_size,
            param_value_size_ret);
  }

  case CL_PROGRAM_KERNEL_NAMES: {
    if(!Prog.HasAttachedKernels())
      return CL_INVALID_PROGRAM_EXECUTABLE;

    opencrun::Program::AttachedKernelsContainer &AttachedKernels =
      Prog.GetAttachedKernels();
    
    std::string kernel_names; 
    for(opencrun::Program::AttachedKernelsContainer::iterator I = AttachedKernels.begin(),
                                                              E = AttachedKernels.end();
                                                              I != E;
                                                              I++) {
      kernel_names += (*I)->GetName().str();
      kernel_names += ';';
    }
    
    llvm::StringRef kernel_names_ref(kernel_names);
    return clFillValue<char, llvm::StringRef>(
        static_cast<char *>(param_value),
        kernel_names_ref,
        param_value_size,
        param_value_size_ret);
  }

  default:
    return CL_INVALID_VALUE;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramBuildInfo(cl_program program,
                      cl_device_id device,
                      cl_program_build_info param_name,
                      size_t param_value_size,
                      void *param_value,
                      size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!program)
    return CL_INVALID_PROGRAM;

  if(!device)
    return CL_INVALID_DEVICE;

  opencrun::Program &Prog = *llvm::cast<opencrun::Program>(program);
  opencrun::Device &Dev = *llvm::cast<opencrun::Device>(device);

  opencrun::Context &Ctx = Prog.GetContext();
  
  if(!Ctx.IsAssociatedWith(Dev))
    return CL_INVALID_DEVICE;

  opencrun::BuildInformation &BuildInfo = Prog.GetBuildInformation(Dev);

  switch(param_name) {
  #define DS_PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)   \
  case PARAM: {                                    \
    return clFillValue<PARAM_TY, FUN_TY>(          \
             static_cast<PARAM_TY *>(param_value), \
             BuildInfo.FUN(),                      \
             param_value_size,                     \
             param_value_size_ret);                \
  }
  #define PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)
  #include "ProgramProperties.def"
  #undef DS_PROPERTY
  #undef PROPERTY

  default:
    return CL_INVALID_VALUE;
  }
}
