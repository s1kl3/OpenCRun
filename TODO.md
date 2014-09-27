
OpenCRun -- Todo list
======================================

* OpenCRun hardware detection should handle **cluster** system and make an
  *opencrun::sys::HardwareSystem* class instance the root of the hardware
  topology built by the hardware parser. So far the root is represented
  by an *opencrun::sys::HardwareMachine* instance which, in a cluster
  environment, would represent a single node.

* Implement **synchronization** between an *Image1D_buffer* instance and the
  corresponding *Buffer* instance and viceversa.

* Replace *llvm::JIT* with *llvm::MCJIT*.

* Implement the following API for complete adherence to the OpenCL 1.2 specs:

  - *clCreateContext* (handling of the lacking *properties*)
  - *clCreateContextFromType* (handling of the lacking *properties*)
  - *clEnqueueMigrateMemObjects*
  - *clSetMemObjectDestructorCallback*
  - *clBuildProgram* (handling of all build options)
  - *clCreateProgramWithBuiltInKernels*
  - *clCompileProgram*
  - *clLinkProgram*
  - *clUnloadCompiler*
  - *clUnloadPlatformCompiler* (replacement for *clUnloadCompiler*)
  - *clGetProgramBuildInfo* (handling of all lacking *param_name* values)
  - *clGetKernelInfo* (handling of the CL_KERNEL_ATTRIBUTES value for *param_name*)
  - *clGetKernelWorkGroupInfo* (handling of the CL_KERNEL_GLOBAL_WORK_SIZE value for *param_name*)
  - *clGetExtensionFunctionAddressForPlatform*
  - *clGetExtensionFunctionAddress*
