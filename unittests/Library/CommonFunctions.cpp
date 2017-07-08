
#include "LibraryFixture.h"

/* INFO: cl_float3 is identical in size, alignment and behavior to cl_float4.
 * A cl_float3 vector type is actually defined by the OpenCL header files as
 * a cl_float4 type and, thus, they can't be distinguished by LibraryFixture.
 */

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class CommonFunctions_TestCase_1 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_ALL_TYPES_1(D)         \
  DeviceTypePair<D, cl_float>,      \
  DeviceTypePair<D, cl_float2>,     \
  DeviceTypePair<D, cl_float3>,     \
  DeviceTypePair<D, cl_float4>,     \
  DeviceTypePair<D, cl_float8>,     \
  DeviceTypePair<D, cl_float16>,    \
  DeviceTypePair<D, cl_double>,     \
  DeviceTypePair<D, cl_double2>,    \
  DeviceTypePair<D, cl_double3>,    \
  DeviceTypePair<D, cl_double4>,    \
  DeviceTypePair<D, cl_double8>,    \
  DeviceTypePair<D, cl_double16>
#else
#define LAST_ALL_TYPES_1(D)     \
  DeviceTypePair<D, cl_float>,  \
  DeviceTypePair<D, cl_float2>, \
  DeviceTypePair<D, cl_float3>, \
  DeviceTypePair<D, cl_float4>, \
  DeviceTypePair<D, cl_float8>, \
  DeviceTypePair<D, cl_float16>
#endif

#define ALL_DEVICE_TYPES_1 \
  LAST_ALL_TYPES_1(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_1> OCLDevicesTypes_1;

TYPED_TEST_CASE_P(CommonFunctions_TestCase_1);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  clamp       |   rrrr
 *  degrees     |   ff, dd
 *  max         |   rrr
 *  min         |   rrrr
 *  mix         |   rrrr
 *  radians     |   ff, dd
 *  sign        |   rr
 *  smoothstep  |   ffff, dddd
 *  step        |   rrr
 */

TYPED_TEST_P(CommonFunctions_TestCase_1, clamp) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Input_3);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(0.5);
  Input_2 = GENTYPE_CREATE(0.6);
  Input_3 = GENTYPE_CREATE(1.5);
  Expected = GENTYPE_CREATE(0.6);
  this->Invoke("clamp", Output, Input_1, Input_2, Input_3);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(CommonFunctions_TestCase_1, degrees) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("degrees", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(CL_M_PI_2);
  Expected = GENTYPE_CREATE(90);
  this->Invoke("degrees", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(-CL_M_PI);
  Expected = GENTYPE_CREATE(-180);
  this->Invoke("degrees", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(CommonFunctions_TestCase_1, max) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(0);
  Input_2 = GENTYPE_CREATE(1.5);
  Expected = GENTYPE_CREATE(1.5);
  this->Invoke("max", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(-0);
  Input_2 = GENTYPE_CREATE(+0);
  Expected = GENTYPE_CREATE(-0);
  this->Invoke("max", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(CommonFunctions_TestCase_1, min) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(0);
  Input_2 = GENTYPE_CREATE(1.5);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("min", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(-0);
  Input_2 = GENTYPE_CREATE(+0);
  Expected = GENTYPE_CREATE(-0);
  this->Invoke("min", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(CommonFunctions_TestCase_1, mix) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Input_3);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(CL_M_E);
  Input_2 = GENTYPE_CREATE(0);
  Input_3 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("mix", Output, Input_1, Input_2, Input_3);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(0);
  Input_2 = GENTYPE_CREATE(1.5);
  Input_3 = GENTYPE_CREATE(1.5);
  Expected = GENTYPE_CREATE(2.25);
  this->Invoke("mix", Output, Input_1, Input_2, Input_3);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(CommonFunctions_TestCase_1, radians) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("radians", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(90);
  Expected = GENTYPE_CREATE(CL_M_PI_2);
  this->Invoke("radians", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(-180);
  Expected = GENTYPE_CREATE(-CL_M_PI);
  this->Invoke("radians", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(CommonFunctions_TestCase_1, sign) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(-0);
  Expected = GENTYPE_CREATE(-0);
  this->Invoke("sign", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("sign", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(-1.5);
  Expected = GENTYPE_CREATE(-1);
  this->Invoke("sign", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1.5);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("sign", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(CommonFunctions_TestCase_1, smoothstep) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Input_3);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(1.5);
  Input_2 = GENTYPE_CREATE(2.5);
  Input_3 = GENTYPE_CREATE(0.5);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("smoothstep", Output, Input_1, Input_2, Input_3);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(1.5);
  Input_2 = GENTYPE_CREATE(2.5);
  Input_3 = GENTYPE_CREATE(3);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("smoothstep", Output, Input_1, Input_2, Input_3);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(-2.5);
  Input_2 = GENTYPE_CREATE(-1.5);
  Input_3 = GENTYPE_CREATE(-3);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("smoothstep", Output, Input_1, Input_2, Input_3);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(-2.5);
  Input_2 = GENTYPE_CREATE(-1.5);
  Input_3 = GENTYPE_CREATE(-0.5);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("smoothstep", Output, Input_1, Input_2, Input_3);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(1.5);
  Input_2 = GENTYPE_CREATE(2.5);
  Input_3 = GENTYPE_CREATE(1.75);
  Expected = GENTYPE_CREATE(0.15625);
  this->Invoke("smoothstep", Output, Input_1, Input_2, Input_3);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(CommonFunctions_TestCase_1, step) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(1.5);
  Input_2 = GENTYPE_CREATE(1.0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("step", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(1.5);
  Input_2 = GENTYPE_CREATE(1.5);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("step", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(1.5);
  Input_2 = GENTYPE_CREATE(1.6);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("step", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(CommonFunctions_TestCase_1, clamp,
                                                       degrees,
                                                       max,
                                                       min,
                                                       mix,
                                                       radians,
                                                       sign,
                                                       smoothstep,
                                                       step);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, CommonFunctions_TestCase_1, OCLDevicesTypes_1);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class CommonFunctions_TestCase_2 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_1(D)                                              \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_float>>,    \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double, cl_double>>

#define LAST_VECTOR_TYPES_1(D, S)                                                       \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float, cl_float>>,      \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_double, cl_double>>
#else
#define LAST_SCALAR_TYPES_1(D)                                              \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_float>>

#define LAST_VECTOR_TYPES_1(D, S)                                                       \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float, cl_float>>
#endif

#define LAST_ALL_TYPES_2(D)     \
  LAST_SCALAR_TYPES_1(D),       \
  LAST_VECTOR_TYPES_1(D, 2),    \
  LAST_VECTOR_TYPES_1(D, 3),    \
  LAST_VECTOR_TYPES_1(D, 4),    \
  LAST_VECTOR_TYPES_1(D, 8),    \
  LAST_VECTOR_TYPES_1(D, 16)

#define ALL_DEVICE_TYPES_2 \
  LAST_ALL_TYPES_2(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_2> OCLDevicesTypes_2;

TYPED_TEST_CASE_P(CommonFunctions_TestCase_2);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  clamp       |   VrVrCrCr
 *  max         |   VrVrCr
 *  min         |   VrVrVrCr
 *  mix         |   VrVrVrCr
 */

TYPED_TEST_P(CommonFunctions_TestCase_2, clamp) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_minval);
  GENTYPE_DECLARE_TUPLE_ELEMENT(3, Input_maxval);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_minval = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Input_maxval = GENTYPE_CREATE_TUPLE_ELEMENT(3, 1.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0.6);
  this->Invoke("clamp", Output, Input_x, Input_minval, Input_maxval);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(CommonFunctions_TestCase_2, max) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1.5);
  this->Invoke("max", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, -0);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, +0);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, -0);
  this->Invoke("max", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(CommonFunctions_TestCase_2, min) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("min", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, -0);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, +0);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, -0);
  this->Invoke("min", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(CommonFunctions_TestCase_2, mix) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(3, Input_a);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_M_E);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0);
  Input_a = GENTYPE_CREATE_TUPLE_ELEMENT(3, 1);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("mix", Output, Input_x, Input_y, Input_a);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1.5);
  Input_a = GENTYPE_CREATE_TUPLE_ELEMENT(3, 1.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 2.25);
  this->Invoke("mix", Output, Input_x, Input_y, Input_a);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(CommonFunctions_TestCase_2, clamp,
                                                       max,
                                                       min,
                                                       mix);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, CommonFunctions_TestCase_2, OCLDevicesTypes_2);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class CommonFunctions_TestCase_3 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_3(D)                                              \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_float>>,    \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double, cl_double>>

#define LAST_VECTOR_TYPES_3(D, S)                                                       \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float, cl_float, cl_float ## S>>,      \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double, cl_double, cl_double ## S>>
#else
#define LAST_SCALAR_TYPES_3(D)                                              \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_float>>

#define LAST_VECTOR_TYPES_3(D, S)                                                       \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float, cl_float, cl_float ## S>>
#endif

#define LAST_ALL_TYPES_3(D)     \
  LAST_SCALAR_TYPES_3(D),       \
  LAST_VECTOR_TYPES_3(D, 2),    \
  LAST_VECTOR_TYPES_3(D, 3),    \
  LAST_VECTOR_TYPES_3(D, 4),    \
  LAST_VECTOR_TYPES_3(D, 8),    \
  LAST_VECTOR_TYPES_3(D, 16)

#define ALL_DEVICE_TYPES_3 \
  LAST_ALL_TYPES_3(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_3> OCLDevicesTypes_3;

TYPED_TEST_CASE_P(CommonFunctions_TestCase_3);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  smoothstep  |   VfCfCfVf, VdCdCdVd
 */

TYPED_TEST_P(CommonFunctions_TestCase_3, smoothstep) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_edge0);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_edge1);
  GENTYPE_DECLARE_TUPLE_ELEMENT(3, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_edge0 = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1.5);
  Input_edge1 = GENTYPE_CREATE_TUPLE_ELEMENT(2, 2.5);
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(3, 0.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("smoothstep", Output, Input_edge0, Input_edge1, Input_x);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_edge0 = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1.5);
  Input_edge1 = GENTYPE_CREATE_TUPLE_ELEMENT(2, 2.5);
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(3, 3);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("smoothstep", Output, Input_edge0, Input_edge1, Input_x);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_edge0 = GENTYPE_CREATE_TUPLE_ELEMENT(1, -2.5);
  Input_edge1 = GENTYPE_CREATE_TUPLE_ELEMENT(2, -1.5);
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(3, -3);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("smoothstep", Output, Input_edge0, Input_edge1, Input_x);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_edge0 = GENTYPE_CREATE_TUPLE_ELEMENT(1, -2.5);
  Input_edge1 = GENTYPE_CREATE_TUPLE_ELEMENT(2, -1.5);
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(3, -0.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("smoothstep", Output, Input_edge0, Input_edge1, Input_x);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_edge0 = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1.5);
  Input_edge1 = GENTYPE_CREATE_TUPLE_ELEMENT(2, 2.5);
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(3, 1.75);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0.15625);
  this->Invoke("smoothstep", Output, Input_edge0, Input_edge1, Input_x);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(CommonFunctions_TestCase_3, smoothstep);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, CommonFunctions_TestCase_3, OCLDevicesTypes_3);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class CommonFunctions_TestCase_4 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_4(D)                                              \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float>>,    \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double>>

#define LAST_VECTOR_TYPES_4(D, S)                                                       \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float, cl_float ## S>>,      \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double, cl_double ## S>>
#else
#define LAST_SCALAR_TYPES_4(D)                                              \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float>>

#define LAST_VECTOR_TYPES_4(D, S)                                                       \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float, cl_float ## S>>
#endif

#define LAST_ALL_TYPES_4(D)     \
  LAST_SCALAR_TYPES_4(D),       \
  LAST_VECTOR_TYPES_4(D, 2),    \
  LAST_VECTOR_TYPES_4(D, 3),    \
  LAST_VECTOR_TYPES_4(D, 4),    \
  LAST_VECTOR_TYPES_4(D, 8),    \
  LAST_VECTOR_TYPES_4(D, 16)

#define ALL_DEVICE_TYPES_4 \
  LAST_ALL_TYPES_4(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_4> OCLDevicesTypes_4;

TYPED_TEST_CASE_P(CommonFunctions_TestCase_4);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  step        |   VrCrVr
 */

TYPED_TEST_P(CommonFunctions_TestCase_4, step) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_edge);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_edge = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1.5);
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1.0);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("step", Output, Input_edge, Input_x);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_edge = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1.5);
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("step", Output, Input_edge, Input_x);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_edge = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1.5);
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1.6);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("step", Output, Input_edge, Input_x);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(CommonFunctions_TestCase_4, step);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, CommonFunctions_TestCase_4, OCLDevicesTypes_4);


