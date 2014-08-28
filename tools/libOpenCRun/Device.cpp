
#include "CL/opencl.h"

#include "Utils.h"

#include "opencrun/Core/Platform.h"

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(cl_platform_id platform,
               cl_device_type device_type,
               cl_uint num_entries,
               cl_device_id *devices,
               cl_uint *num_devices) CL_API_SUFFIX__VERSION_1_0 {
  if(!platform)
    return CL_INVALID_PLATFORM;

  if((num_entries == 0 && devices) || (!num_devices && !devices))
    return CL_INVALID_VALUE;

  opencrun::Platform &Plat = *llvm::cast<opencrun::Platform>(platform);
  llvm::SmallVector<opencrun::Device *, 4> Devs;

  switch(device_type) {
  case CL_DEVICE_TYPE_DEFAULT:
  case CL_DEVICE_TYPE_CPU:
    for(opencrun::Platform::cpu_iterator I = Plat.cpu_begin(),
                                         E = Plat.cpu_end();
                                         I != E;
                                         ++I)
      Devs.push_back(*I);
    break;

  case CL_DEVICE_TYPE_GPU:
    for(opencrun::Platform::gpu_iterator I = Plat.gpu_begin(),
                                         E = Plat.gpu_end();
                                         I != E;
                                         ++I)
      Devs.push_back(*I);
    break;

  case CL_DEVICE_TYPE_ACCELERATOR:
    for(opencrun::Platform::accelerator_iterator I = Plat.accelerator_begin(),
                                                 E = Plat.accelerator_end();
                                                 I != E;
                                                 ++I)
      Devs.push_back(*I);
    break;

  case CL_DEVICE_TYPE_ALL:
    for(opencrun::Platform::cpu_iterator I = Plat.cpu_begin(),
                                         E = Plat.cpu_end();
                                         I != E;
                                         ++I)
      Devs.push_back(*I);
    for(opencrun::Platform::gpu_iterator I = Plat.gpu_begin(),
                                         E = Plat.gpu_end();
                                         I != E;
                                         ++I)
      Devs.push_back(*I);
    for(opencrun::Platform::accelerator_iterator I = Plat.accelerator_begin(),
                                                 E = Plat.accelerator_end();
                                                 I != E;
                                                 ++I)
      Devs.push_back(*I);
    break;

  default:
    return CL_INVALID_DEVICE_TYPE;
  }

  if(Devs.empty())
    return CL_DEVICE_NOT_FOUND;

  if(num_devices)
    *num_devices = Devs.size();

  if(devices)
    for(unsigned I = 0; I < num_entries && I < Devs.size(); ++I)
      *devices++ = Devs[I];

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(cl_device_id device,
                cl_device_info param_name,
                size_t param_value_size,
                void *param_value,
                size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!device)
    return CL_INVALID_DEVICE;

  opencrun::Device &Dev = *llvm::cast<opencrun::Device>(device);
  switch(param_name) {
  #define PROPERTY(PARAM, FUN, PARAM_TY, FUN_TY)   \
  case PARAM: {                                    \
    return clFillValue<PARAM_TY, FUN_TY>(          \
             static_cast<PARAM_TY *>(param_value), \
             Dev.FUN(),                            \
             param_value_size,                     \
             param_value_size_ret);                \
  }
  #include "DeviceProperties.def"
  #undef PROPERTY

  case CL_DEVICE_TYPE: {
    cl_device_type DevTy;

    if(llvm::isa<opencrun::CPUDevice>(&Dev))
      DevTy = CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_DEFAULT;
    else if(llvm::isa<opencrun::GPUDevice>(&Dev))
      DevTy = CL_DEVICE_TYPE_GPU;
    else if(llvm::isa<opencrun::AcceleratorDevice>(&Dev))
      DevTy = CL_DEVICE_TYPE_ACCELERATOR;

    return clFillValue<cl_device_type, cl_device_type>(
             static_cast<cl_device_type *>(param_value),
             DevTy,
             param_value_size,
             param_value_size_ret);
  }

  case CL_DEVICE_PLATFORM:
    return clFillValue<cl_platform_id, opencrun::Platform &>(
             static_cast<cl_platform_id *>(param_value),
             opencrun::GetOpenCRunPlatform(),
             param_value_size,
             param_value_size_ret
             );

  default:
    return CL_INVALID_VALUE;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clCreateSubDevices(cl_device_id in_device,                        
                   const cl_device_partition_property *properties,
                   cl_uint num_devices,
                   cl_device_id *out_devices,               
                   cl_uint *num_devices_ret) CL_API_SUFFIX__VERSION_1_2 {
  if(!in_device)
    return CL_INVALID_DEVICE;

  if(!properties)
    return CL_INVALID_VALUE;

  if(out_devices && num_devices < 1)
    return CL_INVALID_VALUE;

  unsigned num_props = 0;
  switch(properties[0]) {
  case CL_DEVICE_PARTITION_EQUALLY:
    if(properties[1] <= 0 || properties[2] != 0)
      return CL_INVALID_VALUE;

    num_props = 3;
    break;
  case CL_DEVICE_PARTITION_BY_COUNTS: {
    unsigned i;
    for(i = 1; properties[i] != CL_DEVICE_PARTITION_BY_COUNTS_LIST_END; ++i) {
      if(properties[i] < 0)
        return CL_INVALID_DEVICE_PARTITION_COUNT;
    }

    if(properties[++i] != 0)
      return CL_INVALID_VALUE;

    num_props = i;
    break;
  }
  case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
    if(properties[1] <= 0 || properties[2] != 0)
      return CL_INVALID_VALUE;

    switch(properties[1]) {
    case CL_DEVICE_AFFINITY_DOMAIN_NUMA:
    case CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE:
    case CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE:
    case CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE:
    case CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE:
    case CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE:
      break;
    default:
      return CL_INVALID_VALUE;
    }

    num_props = 3;
    break;
  default:
    return CL_INVALID_VALUE;
  }

  opencrun::Device &ParentDev = *llvm::cast<opencrun::Device>(in_device);
  opencrun::DeviceInfo::PartitionPropertiesContainer Props(properties, properties + num_props);

  opencrun::SubDevicesBuilder *Bld;
  opencrun::SubDevicesBuilder::SubDevicesContainer *SubDevs = NULL;
  cl_uint NumSubDevs;
  cl_int ErrCode;
  opencrun::CPUDevice *ParentCPU;

  if((ParentCPU = llvm::dyn_cast<opencrun::CPUDevice>(&ParentDev)))
    Bld = new opencrun::CPUSubDevicesBuilder(*ParentCPU, Props);
  else
    return CL_INVALID_DEVICE;

  if(out_devices)
    SubDevs = new opencrun::SubDevicesBuilder::SubDevicesContainer();

  NumSubDevs = Bld->Create(SubDevs, ErrCode);

  if(num_devices_ret)
    *num_devices_ret = NumSubDevs;

  if(out_devices) {
    if(num_devices < NumSubDevs) {
      delete SubDevs;
      delete Bld;
      return CL_INVALID_VALUE;
    }

    // Reference count for sub-devices set initially to 1.
    for(opencrun::SubDevicesBuilder::SubDevicesContainer::iterator I = SubDevs->begin(),
                                                                   E = SubDevs->end();
                                                                   I != E;
                                                                   ++I)
      if(*I)
        (*I)->Retain();

    // Reference count for the parent device is incremented only if it is
    // a sub-device.
    if(ParentCPU->IsSubDevice())
      ParentCPU->Retain();

    std::copy(SubDevs->begin(), SubDevs->end(), out_devices);
    delete SubDevs;
  }

  delete Bld;
  return ErrCode;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2 {
  if(!device)
    return CL_INVALID_DEVICE;

  opencrun::Device &Dev = *llvm::cast<opencrun::Device>(device);

  // The reference count is incremented only if the device has been
  // created with a call to clCreateSubDevices().
  if(Dev.IsSubDevice())
    Dev.Retain();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2 {
  cl_int Err = CL_SUCCESS;

  if(!device) {
    Err = CL_INVALID_DEVICE;
    return Err;
  }

  opencrun::Device &Dev = *llvm::cast<opencrun::Device>(device);
  
  // The reference count is decremented only if the device has been
  // created with a call to clCreateSubDevices().
  if(Dev.IsSubDevice()) {
    // If the call to the Release method will lead to device destruction,
    // then parent's reference caount will be decremented too.
    bool ReleaseParent = false;
    if(Dev.GetReferenceCount() == 1)
      ReleaseParent = true;

    Dev.Release();

    // Parent's release happens here to force correct destruction order.
    if(ReleaseParent)
      Err =clReleaseDevice(static_cast<cl_device_id>(Dev.GetParent()));
  }

  return Err;
}
