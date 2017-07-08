
OpenCRun -- Todo list
=====================

* Align OpenCRun with the latest LLVM/Clang release.

* Implement a better **scheduling policy** for enqueued task. For example, task
  assignment to active worker threads could take advantage of dynamic load
  balancing algorithms.

* OpenCRun hardware detection cannot rely on libhwloc in case of single core
  x86 CPUs since cache memory sizes are not collected within sysfs.

* OpenCRun hardware detection should handle **cluster** system and make an
  *opencrun::sys::HardwareSystem* class instance the root of the hardware
  topology built by the hardware parser. So far the root is represented
  by an *opencrun::sys::HardwareMachine* instance which, in a cluster
  environment, would represent a single node.

* Implement detection of *CPUDevice* properties at run-time instead of setting
  them statically (see *CPUDevice::InitDeviceInfo*). This approach is suggested
  in particular for:
 
    - Device geometry (maximum sizes for work-item iteration space)
    - CPU extensions (MMX, SSE, SSE2, ..., AVX)
    - CPU address size (32 or 64 bit)
 
  The OpenCL extenstions will be set accordingly to the detected CPU features.

* Implement optimization strategies when creating memory objects marked as
  read-only/write-only/no-access for the host side.

* Replace POSIX threads with C++11 threads. LLVM now offers thread-pool support.

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
    - *clIcdGetPlatformIDsKHR*


