
#include "LibraryFixture.h"

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class AsyncCopyAndPrefetchFunctions_TestCase_1_INT : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                                  typename TyTy::Type> { };

template <typename TyTy>
class AsyncCopyAndPrefetchFunctions_TestCase_1_FLOAT : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                                    typename TyTy::Type> { };

#define LAST_SCALAR_TYPES_1_INT(D)  \
  DeviceTypePair<D, cl_char *>,     \
  DeviceTypePair<D, cl_uchar *>,    \
  DeviceTypePair<D, cl_short *>,    \
  DeviceTypePair<D, cl_ushort *>,   \
  DeviceTypePair<D, cl_int *>,      \
  DeviceTypePair<D, cl_uint *>,     \
  DeviceTypePair<D, cl_long *>,     \
  DeviceTypePair<D, cl_ulong *>

#define LAST_VECTOR_TYPES_1_INT(D, S)   \
  DeviceTypePair<D, cl_char ## S *>,    \
  DeviceTypePair<D, cl_uchar ## S *>,   \
  DeviceTypePair<D, cl_short ## S *>,   \
  DeviceTypePair<D, cl_ushort ## S *>,  \
  DeviceTypePair<D, cl_int ## S *>,     \
  DeviceTypePair<D, cl_uint ## S *>,    \
  DeviceTypePair<D, cl_long ## S *>,    \
  DeviceTypePair<D, cl_ulong ## S *>

#define LAST_SCALAR_TYPES_1_FLOAT(D)    \
  DeviceTypePair<D, cl_float *>,        \
  DeviceTypePair<D, cl_double *>

#define LAST_VECTOR_TYPES_1_FLOAT(D, S) \
  DeviceTypePair<D, cl_float ## S *>,   \
  DeviceTypePair<D, cl_double ## S *>

#define LAST_ALL_TYPES_1_INT(D)     \
  LAST_SCALAR_TYPES_1_INT(D),       \
  LAST_VECTOR_TYPES_1_INT(D, 2),    \
  LAST_VECTOR_TYPES_1_INT(D, 3),    \
  LAST_VECTOR_TYPES_1_INT(D, 4),    \
  LAST_VECTOR_TYPES_1_INT(D, 8),    \
  LAST_VECTOR_TYPES_1_INT(D, 16)

#define LAST_ALL_TYPES_1_FLOAT(D)   \
  LAST_SCALAR_TYPES_1_FLOAT(D),     \
  LAST_VECTOR_TYPES_1_FLOAT(D, 2),  \
  LAST_VECTOR_TYPES_1_FLOAT(D, 3),  \
  LAST_VECTOR_TYPES_1_FLOAT(D, 4),  \
  LAST_VECTOR_TYPES_1_FLOAT(D, 8),  \
  LAST_VECTOR_TYPES_1_FLOAT(D, 16)

#define ALL_DEVICE_TYPES_1_INT \
  LAST_ALL_TYPES_1_INT(CPUDev)

#define ALL_DEVICE_TYPES_1_FLOAT \
  LAST_ALL_TYPES_1_FLOAT(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_1_INT> OCLDevicesTypes_1_INT;
typedef testing::Types<ALL_DEVICE_TYPES_1_FLOAT> OCLDevicesTypes_1_FLOAT;

TYPED_TEST_CASE_P(AsyncCopyAndPrefetchFunctions_TestCase_1_INT);
TYPED_TEST_CASE_P(AsyncCopyAndPrefetchFunctions_TestCase_1_FLOAT);

/*  Function                        |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  async_work_group_copy           |   eP2gP1gze, eP1gP2gze
 *  async_work_group_strided_copy   |   eP2gP1gzze, eP1gP2gzze
 *  wait_group_events               |   vCnPe
 *  prefetch                        |   vP1gz
 *
 * The wait_group_events function is tested along with the asynchronous
 * copy functions.
 */

#define DEFINE_TYPED_TEST_P_1(T)                                                                        \
TYPED_TEST_P(AsyncCopyAndPrefetchFunctions_TestCase_1_ ## T, async_work_group_copy) {                   \
  cl::Device &Dev = this->Dev;                                                                          \
  cl::Context &Ctx = this->Ctx;                                                                         \
  cl::CommandQueue &Queue = this->Queue;                                                                \
                                                                                                        \
  cl_ulong LocalMemSz = Dev.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();                                        \
  size_t MaxWISz = Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0];                                     \
                                                                                                        \
  size_t ElementsPerWG = LocalMemSz / sizeof(cl_long16);                                                \
  size_t WGNum = MaxWISz / ElementsPerWG;                                                               \
  size_t Elements = ElementsPerWG * WGNum;                                                              \
  std::valarray<int> data(Elements);                                                                    \
  data = 1;                                                                                             \
                                                                                                        \
  GENTYPE_DECLARE_OUT_BUFFER(Elements, Output_data);                                                    \
  GENTYPE_DECLARE_BUFFER(Input_data);                                                                   \
  Input_data = GENTYPE_CREATE_BUFFER(data);                                                             \
                                                                                                        \
  cl::NDRange GlobalSpace = cl::NDRange(Elements);                                                      \
  cl::NDRange LocalSpace = cl::NDRange(ElementsPerWG);                                                  \
                                                                                                        \
  cl::Buffer DstBuf = cl::Buffer(Ctx, CL_MEM_WRITE_ONLY, Elements * sizeof(GENTYPE_GET_POINTEE_TYPE));  \
  cl::Buffer SrcBuf = cl::Buffer(Ctx, CL_MEM_READ_ONLY, Elements * sizeof(GENTYPE_GET_POINTEE_TYPE));   \
                                                                                                        \
  std::string KernName;                                                                                 \
  this->BuildKernelName("async_work_group_copy", KernName);                                             \
                                                                                                        \
  bool EnableFP64 = (OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.startswith("double") ||           \
                     OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.startswith("double"));            \
                                                                                                        \
  std::ostringstream SrcStream;                                                                         \
  SrcStream << (EnableFP64 ? "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n\n" : "")                  \
            << "kernel void " << KernName                                                               \
            << "(\n"                                                                                    \
            << "  global " << OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.str() << "out_data,\n"   \
            << "  global " << OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.str() << "in_data,\n"    \
            << "  local " << OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.str() << "local_data\n"   \
            << ")\n"                                                                                    \
            << "{\n"                                                                                    \
            << "  int wg_idx = get_group_id(0);\n"                                                      \
            << "  event_t evt_1, evt_2;\n\n"                                                            \
            << "  evt_1 = async_work_group_copy(local_data, in_data + wg_idx * " << ElementsPerWG       \
              << ", " << ElementsPerWG << ", 0);\n"                                                     \
            << "  wait_group_events(1, &evt_1);\n"                                                      \
            << "  evt_2 = async_work_group_copy(out_data + wg_idx * " << ElementsPerWG                  \
              << ", local_data, " << ElementsPerWG << ", 0);\n"                                         \
            << "  wait_group_events(1, &evt_2);\n"                                                      \
            << "}\n";                                                                                   \
  std::string Src = SrcStream.str();                                                                    \
                                                                                                        \
  cl::Kernel Kern = this->GetKernel(KernName, Src);                                                     \
                                                                                                        \
  Kern.setArg(0, DstBuf);                                                                               \
  Kern.setArg(1, SrcBuf);                                                                               \
  Kern.setArg(2, cl::Local(ElementsPerWG * sizeof(GENTYPE_GET_POINTEE_TYPE)));                          \
                                                                                                        \
  this->WriteBuffer(SrcBuf, Input_data);                                                                \
  Queue.enqueueNDRangeKernel(Kern, cl::NullRange, GlobalSpace, LocalSpace);                             \
  this->ReadBuffer(DstBuf, Output_data);                                                                \
                                                                                                        \
  ASSERT_GENTYPE_BUFFER_EQ(Input_data, Output_data);                                                    \
}                                                                                                       \
                                                                                                        \
TYPED_TEST_P(AsyncCopyAndPrefetchFunctions_TestCase_1_ ## T, async_work_group_strided_copy) {           \
  cl::Device &Dev = this->Dev;                                                                          \
  cl::Context &Ctx = this->Ctx;                                                                         \
  cl::CommandQueue &Queue = this->Queue;                                                                \
                                                                                                        \
  cl_ulong LocalMemSz = Dev.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();                                        \
  size_t MaxWISz = Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0];                                     \
                                                                                                        \
  size_t ElementsPerWG = LocalMemSz / sizeof(cl_long16);                                                \
  size_t WGNum = MaxWISz / ElementsPerWG;                                                               \
  size_t Elements = ElementsPerWG * WGNum;                                                              \
  size_t SrcStride = 1;                                                                                 \
  size_t DstStride = 1;                                                                                 \
  std::valarray<int> data_1(Elements), data_2(Elements), expected(Elements);                            \
  for (unsigned I = 0; I < Elements; ++I) {                                                             \
    data_1[I] = (I % 2 ? 0 : 1);                                                                        \
    data_2[I] = (I % 2 ? 2 : 0);                                                                        \
    expected[I] = (I % 2 ? 2 : 1);                                                                      \
  }                                                                                                     \
                                                                                                        \
  GENTYPE_DECLARE_OUT_BUFFER(Elements, Output_data);                                                    \
  GENTYPE_DECLARE_BUFFER(Input_data_1);                                                                 \
  GENTYPE_DECLARE_BUFFER(Input_data_2);                                                                 \
  GENTYPE_DECLARE_BUFFER(Expected);                                                                     \
  Input_data_1 = GENTYPE_CREATE_BUFFER(data_1);                                                         \
  Input_data_2 = GENTYPE_CREATE_BUFFER(data_2);                                                         \
  Expected = GENTYPE_CREATE_BUFFER(expected);                                                           \
                                                                                                        \
  cl::NDRange GlobalSpace = cl::NDRange(Elements);                                                      \
  cl::NDRange LocalSpace = cl::NDRange(ElementsPerWG);                                                  \
                                                                                                        \
  cl::Buffer DstBuf = cl::Buffer(Ctx, CL_MEM_WRITE_ONLY, Elements * sizeof(GENTYPE_GET_POINTEE_TYPE));  \
  cl::Buffer SrcBuf_1 = cl::Buffer(Ctx, CL_MEM_READ_ONLY, Elements * sizeof(GENTYPE_GET_POINTEE_TYPE)); \
  cl::Buffer SrcBuf_2 = cl::Buffer(Ctx, CL_MEM_READ_ONLY, Elements * sizeof(GENTYPE_GET_POINTEE_TYPE)); \
                                                                                                        \
  std::string KernName;                                                                                 \
  this->BuildKernelName("async_work_group_strided_copy", KernName);                                     \
                                                                                                        \
  bool EnableFP64 = (OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.startswith("double") ||           \
                     OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.startswith("double"));            \
                                                                                                        \
  std::ostringstream SrcStream;                                                                         \
  SrcStream << (EnableFP64 ? "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n\n" : "")                  \
            << "kernel void " << KernName                                                               \
            << "(\n"                                                                                    \
            << "  global " << OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.str() << "out_data,\n"   \
            << "  global " << OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.str() << "in_data_1,\n"  \
            << "  global " << OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.str() << "in_data_2,\n"  \
            << "  local " << OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.str() << "local_data_1,\n"\
            << "  local " << OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.str() << "local_data_2\n" \
            << ")\n"                                                                                    \
            << "{\n"                                                                                    \
            << "  int wg_idx = get_group_id(0);\n"                                                      \
            << "  event_t evt_1, evt_2;\n\n"                                                            \
            << "  evt_1 = async_work_group_strided_copy(local_data_1, in_data_1 + wg_idx * "            \
              << ElementsPerWG << ", " << ElementsPerWG/2 << ", " << SrcStride << ", 0);\n"             \
            << "  async_work_group_strided_copy(local_data_2, in_data_2 + 1 + wg_idx * "                \
              << ElementsPerWG << ", " << ElementsPerWG/2 << ", " << SrcStride << ", evt_1);\n"         \
            << "  wait_group_events(1, &evt_1);\n"                                                      \
            << "  evt_2 = async_work_group_strided_copy(out_data + wg_idx * " << ElementsPerWG          \
              << ", local_data_1, " << ElementsPerWG/2 << ", " << DstStride << ", 0);\n"                \
            << "  async_work_group_strided_copy(out_data + 1 + wg_idx * " << ElementsPerWG              \
              << ", local_data_2, " << ElementsPerWG/2 << ", " << DstStride << ", evt_2);\n"            \
            << "  wait_group_events(1, &evt_2);\n"                                                      \
            << "}\n";                                                                                   \
  std::string Src = SrcStream.str();                                                                    \
                                                                                                        \
  cl::Kernel Kern = this->GetKernel(KernName, Src);                                                     \
                                                                                                        \
  Kern.setArg(0, DstBuf);                                                                               \
  Kern.setArg(1, SrcBuf_1);                                                                             \
  Kern.setArg(2, SrcBuf_2);                                                                             \
  Kern.setArg(3, cl::Local(ElementsPerWG/2 * sizeof(GENTYPE_GET_POINTEE_TYPE)));                        \
  Kern.setArg(4, cl::Local(ElementsPerWG/2 * sizeof(GENTYPE_GET_POINTEE_TYPE)));                        \
                                                                                                        \
  this->WriteBuffer(SrcBuf_1, Input_data_1);                                                            \
  this->WriteBuffer(SrcBuf_2, Input_data_2);                                                            \
  Queue.enqueueNDRangeKernel(Kern, cl::NullRange, GlobalSpace, LocalSpace);                             \
  this->ReadBuffer(DstBuf, Output_data);                                                                \
                                                                                                        \
  ASSERT_GENTYPE_BUFFER_EQ(Expected, Output_data);                                                      \
}                                                                                                       \
                                                                                                        \
TYPED_TEST_P(AsyncCopyAndPrefetchFunctions_TestCase_1_ ## T, prefetch) {                                \
  cl::Device &Dev = this->Dev;                                                                          \
  cl::Context &Ctx = this->Ctx;                                                                         \
  cl::CommandQueue &Queue = this->Queue;                                                                \
                                                                                                        \
  cl_ulong GlobalCacheSz = Dev.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>();                              \
                                                                                                        \
  size_t Elements = GlobalCacheSz / sizeof(cl_long16);                                                  \
  std::valarray<int> data(Elements),                                                                    \
                     expected(Elements);                                                                \
  data = 1;                                                                                             \
  expected = data * 2;                                                                                  \
                                                                                                        \
  GENTYPE_DECLARE_BUFFER(Input_data);                                                                   \
  GENTYPE_DECLARE_BUFFER(Expected);                                                                     \
  GENTYPE_DECLARE_OUT_BUFFER(Elements, Output_data);                                                    \
                                                                                                        \
  Input_data = GENTYPE_CREATE_BUFFER(data);                                                             \
  Expected = GENTYPE_CREATE_BUFFER(expected);                                                           \
                                                                                                        \
  cl::NDRange Space = cl::NDRange(1);                                                                   \
                                                                                                        \
  cl::Buffer DstBuf = cl::Buffer(Ctx, CL_MEM_WRITE_ONLY, Elements * sizeof(GENTYPE_GET_POINTEE_TYPE));  \
  cl::Buffer SrcBuf = cl::Buffer(Ctx, CL_MEM_READ_ONLY, Elements * sizeof(GENTYPE_GET_POINTEE_TYPE));   \
                                                                                                        \
  std::string KernName;                                                                                 \
  this->BuildKernelName("prefetch", KernName);                                                          \
                                                                                                        \
  bool EnableFP64 = (OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.startswith("double") ||           \
                     OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.startswith("double"));            \
                                                                                                        \
  std::ostringstream SrcStream;                                                                         \
  SrcStream << (EnableFP64 ? "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n\n" : "")                  \
            << "kernel void " << KernName                                                               \
            << "(\n"                                                                                    \
            << "  global " << OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.str() << "out_data,\n"   \
            << "  global " << OCLTypeTraits<GENTYPE_GET_BUFFER_TYPE>::OCLCName.str() << "in_data\n"     \
            << ")\n"                                                                                    \
            << "{\n"                                                                                    \
            << "  prefetch(in_data, " << Elements << ");\n"                                             \
            << "  for (int i = 0; i < " << Elements << "; ++i)\n"                                       \
            << "    *(out_data + i) = *(in_data + i) * ("                                               \
              << OCLTypeTraits<GENTYPE_GET_POINTEE_TYPE>::OCLCName.str() << ")2;\n"                     \
            << "}\n";                                                                                   \
  std::string Src = SrcStream.str();                                                                    \
                                                                                                        \
  cl::Kernel Kern = this->GetKernel(KernName, Src);                                                     \
                                                                                                        \
  Kern.setArg(0, DstBuf);                                                                               \
  Kern.setArg(1, SrcBuf);                                                                               \
                                                                                                        \
  this->WriteBuffer(SrcBuf, Input_data);                                                                \
  Queue.enqueueNDRangeKernel(Kern, cl::NullRange, Space, Space);                                        \
  this->ReadBuffer(DstBuf, Output_data);                                                                \
                                                                                                        \
  ASSERT_GENTYPE_BUFFER_EQ(Expected, Output_data);                                                      \
}

DEFINE_TYPED_TEST_P_1(INT)
DEFINE_TYPED_TEST_P_1(FLOAT)

REGISTER_TYPED_TEST_CASE_P(AsyncCopyAndPrefetchFunctions_TestCase_1_INT, async_work_group_copy,
                                                                         async_work_group_strided_copy,
                                                                         prefetch);
REGISTER_TYPED_TEST_CASE_P(AsyncCopyAndPrefetchFunctions_TestCase_1_FLOAT, async_work_group_copy,
                                                                           async_work_group_strided_copy,
                                                                           prefetch);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, AsyncCopyAndPrefetchFunctions_TestCase_1_INT, OCLDevicesTypes_1_INT);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, AsyncCopyAndPrefetchFunctions_TestCase_1_FLOAT, OCLDevicesTypes_1_FLOAT);

