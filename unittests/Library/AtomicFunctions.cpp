
#include "LibraryFixture.h"

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class AtomicFunctions_TestCase_1 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                typename TyTy::Type> { };

#define LAST_ALL_TYPES_1(D) \
  DeviceTypePair<D, std::tuple<cl_int, cl_int *, cl_int>>,     \
  DeviceTypePair<D, std::tuple<cl_uint, cl_uint *, cl_uint>>

#define ALL_DEVICE_TYPES_1 \
  LAST_ALL_TYPES_1(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_1> OCLDevicesTypes_1;

TYPED_TEST_CASE_P(AtomicFunctions_TestCase_1);

/*  Function        |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------------------------
 *  atomic_add      |   nP1nn, UnP1UnUn, nP2nn, UnP2UnUn
 *  atomic_sub      |   nP1nn, UnP1UnUn, nP2nn, UnP2UnUn
 *  atomic_inc      |   nP1n, UnP1Un, nP2n, UnP2Un
 *  atomic_dec      |   nP1n, UnP1Un, nP2n, UnP2Un
 *  atomic_cmpxchg  |   nP1nnn, UnP1UnUnUn, nP2nnn, UnP2UnUnUn
 *  atomic_min      |   nP1nn, UnP1UnUn, nP2nn, UnP2UnUn
 *  atomic_max      |   nP1nn, UnP1UnUn, nP2nn, UnP2UnUn
 *  atomic_and      |   nP1nn, UnP1UnUn, nP2nn, UnP2UnUn
 *  atomic_or       |   nP1nn, UnP1UnUn, nP2nn, UnP2UnUn
 *  atomic_xor      |   nP1nn, UnP1UnUn, nP2nn, UnP2UnUn
 */  

TYPED_TEST_P(AtomicFunctions_TestCase_1, atomic_add) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  int MaxWISz = static_cast<int>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_val);

  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, MaxWISz - 1);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ MaxWISz })));
  Input_val = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  this->Invoke("atomic_add", Output, InputOutput_p, Input_val, GlobalSpace, LocalSpace);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(AtomicFunctions_TestCase_1, atomic_sub) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  int MaxWISz = static_cast<int>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_val);

  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ MaxWISz }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 })));
  Input_val = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  this->Invoke("atomic_sub", Output, InputOutput_p, Input_val, GlobalSpace, LocalSpace);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(AtomicFunctions_TestCase_1, atomic_inc) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  int MaxWISz = static_cast<int>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, MaxWISz - 1);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ MaxWISz })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  this->Invoke("atomic_inc", Output, InputOutput_p, GlobalSpace, LocalSpace);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(AtomicFunctions_TestCase_1, atomic_dec) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  int MaxWISz = static_cast<int>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ MaxWISz }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  this->Invoke("atomic_dec", Output, InputOutput_p, GlobalSpace, LocalSpace);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(AtomicFunctions_TestCase_1, atomic_cmpxchg) {
  cl::Device Dev = this->GetDevice();
  int MaxWISz = static_cast<int>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  std::valarray<int> expected(MaxWISz);
  for (unsigned I = 1; I < expected.size(); ++I) {
    expected[I] = I;
  }

  GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(0, MaxWISz, Output_old);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(0, Expected_old);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  Expected_old = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(0, expected);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ MaxWISz })));

  // The barrier function synchronizes only work-items of the same work-group.
  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz);

  cl::Buffer RetBuf = this->AllocReturnBuffer(Output_old);
  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atomic_cmpxchg_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_TYPE(0)>::OCLCName.str() << " *old,\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  uint id = get_global_id(0);\n"
            << "  uint i;\n\n"
            << "  for (i = 0; i < get_global_size(0); ++i) {\n"
            << "    if (i == id)\n"
            << "      old[i] = atomic_cmpxchg(p, ("
              << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_TYPE(2)>::OCLCName.str()
              << ")id, ("
              << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_TYPE(2)>::OCLCName.str()
              << ")id + 1);\n\n"
            << "    barrier(CLK_GLOBAL_MEM_FENCE);\n"
            << "  }\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atomic_cmpxchg_test", SrcStream.str());

  Kern.setArg(0, RetBuf);
  Kern.setArg(1, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(RetBuf, Output_old);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(0, Expected_old, Output_old);
}

TYPED_TEST_P(AtomicFunctions_TestCase_1, atomic_min) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  int MaxWISz = static_cast<int>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ MaxWISz }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atomic_min_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(2)>::OCLCName.str()
              << " id = get_global_id(0);\n\n"
            << "  atomic_min(p, id);\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atomic_min_test", SrcStream.str());

  Kern.setArg(0, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
}

TYPED_TEST_P(AtomicFunctions_TestCase_1, atomic_max) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  int MaxWISz = static_cast<int>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ MaxWISz - 1 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atomic_max_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(2)>::OCLCName.str()
              << " id = get_global_id(0);\n\n"
            << "  atomic_max(p, id);\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atomic_max_test", SrcStream.str());

  Kern.setArg(0, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
}

TYPED_TEST_P(AtomicFunctions_TestCase_1, atomic_and) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  int MaxWISz = static_cast<int>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ MaxWISz }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atomic_and_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(2)>::OCLCName.str()
              << " id = get_group_id(0);\n\n"
            << "  atomic_and(p, id);\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atomic_and_test", SrcStream.str());

  Kern.setArg(0, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
}

TYPED_TEST_P(AtomicFunctions_TestCase_1, atomic_or) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  int MaxWISz = static_cast<int>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 7 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atomic_or_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(2)>::OCLCName.str()
              << " id = get_group_id(0);\n\n"
            << "  atomic_or(p, id);\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atomic_or_test", SrcStream.str());

  Kern.setArg(0, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
}

TYPED_TEST_P(AtomicFunctions_TestCase_1, atomic_xor) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  int MaxWISz = static_cast<int>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atomic_xor_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(2)>::OCLCName.str()
              << " id = get_group_id(0);\n\n"
            << "  atomic_xor(p, id);\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atomic_xor_test", SrcStream.str());

  Kern.setArg(0, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
}

REGISTER_TYPED_TEST_CASE_P(AtomicFunctions_TestCase_1, atomic_add,
                                                       atomic_sub,
                                                       atomic_inc,
                                                       atomic_dec,
                                                       atomic_cmpxchg,
                                                       atomic_min,
                                                       atomic_max,
                                                       atomic_and,
                                                       atomic_or,
                                                       atomic_xor);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, AtomicFunctions_TestCase_1, OCLDevicesTypes_1);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class AtomicFunctions_TestCase_2 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                typename TyTy::Type> { };

#define LAST_ALL_TYPES_2(D)                                     \
  DeviceTypePair<D, std::tuple<cl_int, cl_int *, cl_int>>,      \
  DeviceTypePair<D, std::tuple<cl_uint, cl_uint *, cl_uint>>,   \
  DeviceTypePair<D, std::tuple<cl_float, cl_float *, cl_float>>

#define ALL_DEVICE_TYPES_2 \
  LAST_ALL_TYPES_2(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_2> OCLDevicesTypes_2;

TYPED_TEST_CASE_P(AtomicFunctions_TestCase_2);

/*  Function        |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------------------------
 *  atomic_xchg     |   nP1nn, UnP1UnUn, fP1ff, nP2nn, UnP2UnUn, fP2ff
 */  

TYPED_TEST_P(AtomicFunctions_TestCase_2, atomic_xchg) {
  cl::Device Dev = this->GetDevice();
  int MaxWISz = static_cast<int>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  std::valarray<int> expected_old(MaxWISz);
  expected_old[0] = 0;
  for (unsigned I = 1; I < expected_old.size(); ++I) {
    expected_old[I] = I - 1;
  }

  GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(0, MaxWISz, Output_old);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(0, Expected_old);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  Expected_old = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(0, expected_old);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(int, ({ MaxWISz - 1 })));

  // The barrier function synchronizes only work-items of the same work-group.
  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz);

  cl::Buffer RetBuf = this->AllocReturnBuffer(Output_old);
  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atomic_xchg_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_TYPE(0)>::OCLCName.str() << " *old,\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  uint id = get_global_id(0);\n"
            << "  uint i;\n\n"
            << "  for (i = 0; i < get_global_size(0); ++i) {\n"
            << "    if (i == id)\n"
            << "      old[i] = atomic_xchg(p, ("
              << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_TYPE(2)>::OCLCName.str()
              << ")id);\n\n"
            << "    barrier(CLK_GLOBAL_MEM_FENCE);\n"
            << "  }\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atomic_xchg_test", SrcStream.str());

  Kern.setArg(0, RetBuf);
  Kern.setArg(1, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(RetBuf, Output_old);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(0, Expected_old, Output_old);
}

REGISTER_TYPED_TEST_CASE_P(AtomicFunctions_TestCase_2, atomic_xchg);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, AtomicFunctions_TestCase_2, OCLDevicesTypes_2);

// ---------------------------------------------------------------------------------------- //

#ifdef cl_khr_int64_base_atomics

template <typename TyTy>
class AtomicFunctions_TestCase_3 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                typename TyTy::Type> { };

#define LAST_ALL_TYPES_3(D) \
  DeviceTypePair<D, std::tuple<cl_long, cl_long *, cl_long>>,     \
  DeviceTypePair<D, std::tuple<cl_ulong, cl_ulong *, cl_long>>

#define ALL_DEVICE_TYPES_3 \
  LAST_ALL_TYPES_3(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_3> OCLDevicesTypes_3;

TYPED_TEST_CASE_P(AtomicFunctions_TestCase_3);

/*  Function        |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------------------------
 *  atom_add        |   lP1ll, UlP1UlUl, lP2ll, UlP2UlUl
 *  atom_sub        |   lP1ll, UlP1UlUl, lP2ll, UlP2UlUl
 *  atom_xchg       |   lP1ll, UlP1UlUl, lP2ll, UlP2UlUl
 *  atom_inc        |   lP1l, UlP1Ul, lP2l, UlP2Ul
 *  atom_dec        |   lP1l, UlP1Ul, lP2l, UlP2Ul
 *  atom_cmpxchg    |   lP1lll, UlP1UlUlUl, lP2lll, UlP2UlUlUl
 */  

TYPED_TEST_P(AtomicFunctions_TestCase_3, atom_add) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  long MaxWISz = static_cast<long>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_val);

  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, MaxWISz - 1);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ MaxWISz })));
  Input_val = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  this->Invoke("atom_add", Output, InputOutput_p, Input_val, GlobalSpace, LocalSpace);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(AtomicFunctions_TestCase_3, atom_sub) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  long MaxWISz = static_cast<long>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_val);

  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ MaxWISz }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 })));
  Input_val = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  this->Invoke("atom_sub", Output, InputOutput_p, Input_val, GlobalSpace, LocalSpace);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(AtomicFunctions_TestCase_3, atom_xchg) {
  cl::Device Dev = this->GetDevice();
  long MaxWISz = static_cast<long>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  std::valarray<long> expected_old(MaxWISz);
  expected_old[0] = 0;
  for (unsigned I = 1; I < expected_old.size(); ++I) {
    expected_old[I] = I - 1;
  }

  GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(0, MaxWISz, Output_old);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(0, Expected_old);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  Expected_old = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(0, expected_old);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ MaxWISz - 1 })));

  // The barrier function synchronizes only work-items of the same work-group.
  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz);

  cl::Buffer RetBuf = this->AllocReturnBuffer(Output_old);
  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atom_xchg_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_TYPE(0)>::OCLCName.str() << " *old,\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  uint id = get_global_id(0);\n"
            << "  uint i;\n\n"
            << "  for (i = 0; i < get_global_size(0); ++i) {\n"
            << "    if (i == id)\n"
            << "      old[i] = atom_xchg(p, ("
              << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_TYPE(2)>::OCLCName.str()
              << ")id);\n\n"
            << "    barrier(CLK_GLOBAL_MEM_FENCE);\n"
            << "  }\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atom_xchg_test", SrcStream.str());

  Kern.setArg(0, RetBuf);
  Kern.setArg(1, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(RetBuf, Output_old);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(0, Expected_old, Output_old);
}

TYPED_TEST_P(AtomicFunctions_TestCase_3, atom_inc) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  long MaxWISz = static_cast<long>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, MaxWISz - 1);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ MaxWISz })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  this->Invoke("atom_inc", Output, InputOutput_p, GlobalSpace, LocalSpace);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(AtomicFunctions_TestCase_3, atom_dec) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  long MaxWISz = static_cast<long>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ MaxWISz }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  this->Invoke("atom_dec", Output, InputOutput_p, GlobalSpace, LocalSpace);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(AtomicFunctions_TestCase_3, atom_cmpxchg) {
  cl::Device Dev = this->GetDevice();
  long MaxWISz = static_cast<long>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  std::valarray<long> expected_old(MaxWISz);
  for (unsigned I = 1; I < expected_old.size(); ++I) {
    expected_old[I] = I;
  }

  GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(0, MaxWISz, Output_old);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(0, Expected_old);
  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  Expected_old = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(0, expected_old);
  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ MaxWISz })));

  // The barrier function synchronizes only work-items of the same work-group.
  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz);

  cl::Buffer RetBuf = this->AllocReturnBuffer(Output_old);
  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atom_cmpxchg_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_TYPE(0)>::OCLCName.str() << " *old,\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  uint id = get_global_id(0);\n"
            << "  uint i;\n\n"
            << "  for (i = 0; i < get_global_size(0); ++i) {\n"
            << "    if (i == id)\n"
            << "      old[i] = atom_cmpxchg(p, ("
              << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_TYPE(2)>::OCLCName.str()
              << ")id, ("
              << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_TYPE(2)>::OCLCName.str()
              << ")id + 1);\n\n"
            << "    barrier(CLK_GLOBAL_MEM_FENCE);\n"
            << "  }\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atom_cmpxchg_test", SrcStream.str());

  Kern.setArg(0, RetBuf);
  Kern.setArg(1, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(RetBuf, Output_old);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(0, Expected_old, Output_old);
}

REGISTER_TYPED_TEST_CASE_P(AtomicFunctions_TestCase_3, atom_add,
                                                       atom_sub,
                                                       atom_xchg,
                                                       atom_inc,
                                                       atom_dec,
                                                       atom_cmpxchg);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, AtomicFunctions_TestCase_3, OCLDevicesTypes_3);

#endif // cl_khr_int64_base_atomics

// ---------------------------------------------------------------------------------------- //

#ifdef cl_khr_int64_extended_atomics

template <typename TyTy>
class AtomicFunctions_TestCase_4 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                typename TyTy::Type> { };

#define LAST_ALL_TYPES_4(D) \
  DeviceTypePair<D, std::tuple<cl_long, cl_long *, cl_long>>,     \
  DeviceTypePair<D, std::tuple<cl_ulong, cl_ulong *, cl_long>>

#define ALL_DEVICE_TYPES_4 \
  LAST_ALL_TYPES_4(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_4> OCLDevicesTypes_4;

TYPED_TEST_CASE_P(AtomicFunctions_TestCase_4);

/*  Function        |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------------------------
 *  atom_min        |   lP1ll, UlP1UlUl, lP2ll, UlP2UlUl
 *  atom_max        |   lP1ll, UlP1UlUl, lP2ll, UlP2UlUl
 *  atom_and        |   lP1ll, UlP1UlUl, lP2ll, UlP2UlUl
 *  atom_or         |   lP1ll, UlP1UlUl, lP2ll, UlP2UlUl
 *  atom_xor        |   lP1ll, UlP1UlUl, lP2ll, UlP2UlUl
 */  

TYPED_TEST_P(AtomicFunctions_TestCase_4, atom_min) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  long MaxWISz = static_cast<long>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ MaxWISz }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atom_min_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(2)>::OCLCName.str()
              << " id = get_global_id(0);\n\n"
            << "  atom_min(p, id);\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atom_min_test", SrcStream.str());

  Kern.setArg(0, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
}

TYPED_TEST_P(AtomicFunctions_TestCase_4, atom_max) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  long MaxWISz = static_cast<long>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ MaxWISz - 1 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atom_max_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(2)>::OCLCName.str()
              << " id = get_global_id(0);\n\n"
            << "  atom_max(p, id);\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atom_max_test", SrcStream.str());

  Kern.setArg(0, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
}

TYPED_TEST_P(AtomicFunctions_TestCase_4, atom_and) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  long MaxWISz = static_cast<long>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ MaxWISz }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atom_and_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(2)>::OCLCName.str()
              << " id = get_group_id(0);\n\n"
            << "  atom_and(p, id);\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atom_and_test", SrcStream.str());

  Kern.setArg(0, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
}

TYPED_TEST_P(AtomicFunctions_TestCase_4, atom_or) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  long MaxWISz = static_cast<long>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 7 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atom_or_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(2)>::OCLCName.str()
              << " id = get_group_id(0);\n\n"
            << "  atom_or(p, id);\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atom_or_test", SrcStream.str());

  Kern.setArg(0, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
}

TYPED_TEST_P(AtomicFunctions_TestCase_4, atom_xor) {
  cl::Device Dev = this->GetDevice();
  cl_uint MaxCUs = Dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
  long MaxWISz = static_cast<long>(Dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]);

  GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(1, InputOutput_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(1, Expected_p);

  InputOutput_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 }))); 
  Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(1, INIT_LIST(long, ({ 0 })));

  cl::NDRange GlobalSpace = cl::NDRange(MaxWISz);
  cl::NDRange LocalSpace = cl::NDRange(MaxWISz / MaxCUs);

  cl::Buffer A1Buf = this->AllocArgBuffer(InputOutput_p, KernelArgType::InputOutputBuffer);

  std::ostringstream SrcStream;
  SrcStream << "kernel void atom_xor_test(\n"
            << "  global " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(1)>::OCLCName.str() << " *p\n"
            << ")\n"
            << "{\n"
            << "  " << OCLTypeTraits<GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(2)>::OCLCName.str()
              << " id = get_group_id(0);\n\n"
            << "  atom_xor(p, id);\n"
            << "}\n";
  cl::Kernel Kern = this->GetKernel("atom_xor_test", SrcStream.str());

  Kern.setArg(0, A1Buf);

  cl::CommandQueue Queue = this->GetQueue();
  this->WriteBuffer(A1Buf, InputOutput_p);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             GlobalSpace,
                             LocalSpace);
  this->ReadBuffer(A1Buf, InputOutput_p);

  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(1, Expected_p, InputOutput_p);
}

REGISTER_TYPED_TEST_CASE_P(AtomicFunctions_TestCase_4, atom_min,
                                                       atom_max,
                                                       atom_and,
                                                       atom_or,
                                                       atom_xor);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, AtomicFunctions_TestCase_4, OCLDevicesTypes_4);

#endif // cl_khr_int64_extended_atomics
