//===----------------------------------------------------------------------===//
// Round toward zero
//===----------------------------------------------------------------------===//

__opencrun_overload
float __internal_ocl_Si2f_rtz(long x) {
  return x;
}

__opencrun_overload
float __internal_ocl_Ui2f_rtz(ulong x) {
  return x;
}

#if defined(__opencrun_target_ext_cl_khr_fp64)
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

float __internal_ocl_d2f_rtz(double x) {
  return x;
}

__opencrun_overload
double __internal_ocl_Si2d_rtz(long x) {
  return x;
}

__opencrun_overload
double __internal_ocl_Ui2d_rtz(ulong x) {
  return x;
}

#pragma OPENCL EXTENSION cl_khr_fp64 : disable
#endif

//===----------------------------------------------------------------------===//
// Round toward plus infinity
//===----------------------------------------------------------------------===//

__opencrun_overload
float __internal_ocl_Si2f_rtp(long x) {
  return x;
}

__opencrun_overload
float __internal_ocl_Ui2f_rtp(ulong x) {
  return x;
}

#if defined(__opencrun_target_ext_cl_khr_fp64)
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

float __internal_ocl_d2f_rtp(double x) {
  return x;
}

__opencrun_overload
double __internal_ocl_Si2d_rtp(long x) {
  return x;
}

__opencrun_overload
double __internal_ocl_Ui2d_rtp(ulong x) {
  return x;
}

#pragma OPENCL EXTENSION cl_khr_fp64 : disable
#endif

//===----------------------------------------------------------------------===//
// Round toward minus infinity
//===----------------------------------------------------------------------===//

__opencrun_overload
float __internal_ocl_Si2f_rtn(long x) {
  return x;
}

__opencrun_overload
float __internal_ocl_Ui2f_rtn(ulong x) {
  return x;
}

#if defined(__opencrun_target_ext_cl_khr_fp64)
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

float __internal_ocl_d2f_rtn(double x) {
  return x;
}

__opencrun_overload
double __internal_ocl_Si2d_rtn(long x) {
  return x;
}

__opencrun_overload
double __internal_ocl_Ui2d_rtn(ulong x) {
  return x;
}

#pragma OPENCL EXTENSION cl_khr_fp64 : disable
#endif
