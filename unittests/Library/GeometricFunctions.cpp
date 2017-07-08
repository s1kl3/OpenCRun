
#include "LibraryFixture.h"

/* INFO: cl_float3 is identical in size, alignment and behavior to cl_float4.
 * A cl_float3 vector type is actually defined by the OpenCL header files as
 * a cl_float4 type and, thus, they can't be distinguished by LibraryFixture.
 */

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class GeometricFunctions_TestCase_1 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                   typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_ALL_TYPES_1(D)         \
  DeviceTypePair<D, cl_float3>,     \
  DeviceTypePair<D, cl_float4>,     \
  DeviceTypePair<D, cl_double3>,    \
  DeviceTypePair<D, cl_double4>
#else
#define LAST_ALL_TYPES_1(D)     \
  DeviceTypePair<D, cl_float3>, \
  DeviceTypePair<D, cl_float4>
#endif

#define ALL_DEVICE_TYPES_1 \
  LAST_ALL_TYPES_1(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_1> OCLDevicesTypes_1;

TYPED_TEST_CASE_P(GeometricFunctions_TestCase_1);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  cross       |   V3fV3fV3f, V4fV4fV4f, V3dV3dV3d, V4dV4dV4d
 */

TYPED_TEST_P(GeometricFunctions_TestCase_1, cross) {
  GENTYPE_DECLARE(Input_p0);
  GENTYPE_DECLARE(Input_p1);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_p0 = GENTYPE_CREATE(1);
  Input_p1 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("cross", Output, Input_p0, Input_p1);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(GeometricFunctions_TestCase_1, cross);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, GeometricFunctions_TestCase_1, OCLDevicesTypes_1);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class GeometricFunctions_TestCase_2 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                   typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_2(D)                                      \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float>>,      \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double>>

#define LAST_VECTOR_TYPES_2(D, S)                                           \
  DeviceTypePair<D, std::tuple<cl_float, cl_float ## S, cl_float ## S>>,    \
  DeviceTypePair<D, std::tuple<cl_double, cl_double ## S, cl_double ## S>>
#else
#define LAST_SCALAR_TYPES_2(D) \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float>>

#define LAST_VECTOR_TYPES_2(D, S) \
  DeviceTypePair<D, std::tuple<cl_float, cl_float ## S, cl_float ## S>>
#endif

#define LAST_ALL_TYPES_2(D)  \
  LAST_SCALAR_TYPES_2(D),    \
  LAST_VECTOR_TYPES_2(D, 2), \
  LAST_VECTOR_TYPES_2(D, 3), \
  LAST_VECTOR_TYPES_2(D, 4)

#define ALL_DEVICE_TYPES_2 \
  LAST_ALL_TYPES_2(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_2> OCLDevicesTypes_2;

TYPED_TEST_CASE_P(GeometricFunctions_TestCase_2);

/*  Function        |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  dot             |   Crrr
 *  distance        |   Crrr
 *  length          |   Crr
 */

TYPED_TEST_P(GeometricFunctions_TestCase_2, dot) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_p0);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_p1);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_p0 = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Input_p1 = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);
  if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float) ||
      GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float2) ||
           GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double2))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 2);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float3) ||
           GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double3))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 4);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float4) ||
           GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double4))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 4);
  this->Invoke("dot", Output, Input_p0, Input_p1);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(GeometricFunctions_TestCase_2, distance) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_p0);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_p1);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_p0 = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Input_p1 = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("distance", Output, Input_p0, Input_p1);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_p0 = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Input_p1 = GENTYPE_CREATE_TUPLE_ELEMENT(2, -1);
  if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float) ||
      GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 2);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float2) ||
           GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double2))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 2 * CL_M_SQRT2);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float3) ||
           GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double3))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 4);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float4) ||
           GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double4))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 4);
  this->Invoke("distance", Output, Input_p0, Input_p1);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(GeometricFunctions_TestCase_2, length) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_p = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float) ||
      GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float2) ||
           GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double2))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, CL_M_SQRT2);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float3) ||
           GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double3))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 2);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float4) ||
           GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_double4))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 2);
  this->Invoke("length", Output, Input_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(GeometricFunctions_TestCase_2, dot,
                                                          distance,
                                                          length);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, GeometricFunctions_TestCase_2, OCLDevicesTypes_2);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class GeometricFunctions_TestCase_3 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                   typename TyTy::Type> { };

#define LAST_SCALAR_TYPES_3(D) \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float>>

#define LAST_VECTOR_TYPES_3(D, S) \
  DeviceTypePair<D, std::tuple<cl_float, cl_float ## S, cl_float ## S>>

#define LAST_ALL_TYPES_3(D)  \
  LAST_SCALAR_TYPES_3(D),    \
  LAST_VECTOR_TYPES_3(D, 2), \
  LAST_VECTOR_TYPES_3(D, 3), \
  LAST_VECTOR_TYPES_3(D, 4)

#define ALL_DEVICE_TYPES_3 \
  LAST_ALL_TYPES_3(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_3> OCLDevicesTypes_3;

TYPED_TEST_CASE_P(GeometricFunctions_TestCase_3);

/*  Function        |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  fast_distance   |   Cfff
 *  fast_length     |   Cff
 */

TYPED_TEST_P(GeometricFunctions_TestCase_3, fast_distance) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_p0);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_p1);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_p0 = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Input_p1 = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("fast_distance", Output, Input_p0, Input_p1);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_p0 = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Input_p1 = GENTYPE_CREATE_TUPLE_ELEMENT(2, -1);
  if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 2);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float2))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 2 * CL_M_SQRT2);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float3))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 4);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float4))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 4);
  this->Invoke("fast_distance", Output, Input_p0, Input_p1);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(GeometricFunctions_TestCase_3, fast_length) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_p = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float2))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, CL_M_SQRT2);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float3))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 2);
  else if (GENTYPE_CHECK_TUPLE_ELEMENT(1, cl_float4))
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 2);
  this->Invoke("fast_length", Output, Input_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(GeometricFunctions_TestCase_3, fast_distance,
                                                          fast_length);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, GeometricFunctions_TestCase_3, OCLDevicesTypes_3);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class GeometricFunctions_TestCase_4 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                   typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_ALL_TYPES_4(D)         \
  DeviceTypePair<D, cl_float>,      \
  DeviceTypePair<D, cl_float2>,     \
  DeviceTypePair<D, cl_float3>,     \
  DeviceTypePair<D, cl_float4>,     \
  DeviceTypePair<D, cl_double>,     \
  DeviceTypePair<D, cl_double2>,    \
  DeviceTypePair<D, cl_double3>,    \
  DeviceTypePair<D, cl_double4>
#else
#define LAST_ALL_TYPES_4(D)     \
  DeviceTypePair<D, cl_float>,  \
  DeviceTypePair<D, cl_float2>, \
  DeviceTypePair<D, cl_float3>, \
  DeviceTypePair<D, cl_float4>
#endif

#define ALL_DEVICE_TYPES_4 \
  LAST_ALL_TYPES_4(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_4> OCLDevicesTypes_4;

TYPED_TEST_CASE_P(GeometricFunctions_TestCase_4);

/*  Function        |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  normalize       |   rr
 */

TYPED_TEST_P(GeometricFunctions_TestCase_4, normalize) {
  GENTYPE_DECLARE(Input_p);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_p = GENTYPE_CREATE(2);
  if (GENTYPE_CHECK(cl_float) || GENTYPE_CHECK(cl_double))
    Expected = GENTYPE_CREATE(1);
  else if (GENTYPE_CHECK(cl_float2) || GENTYPE_CHECK(cl_double2))
    Expected = GENTYPE_CREATE(CL_M_SQRT1_2);
  else if (GENTYPE_CHECK(cl_float3) || GENTYPE_CHECK(cl_double3))
    Expected = GENTYPE_CREATE(0.5);
  else if (GENTYPE_CHECK(cl_float4) || GENTYPE_CHECK(cl_double4))
    Expected = GENTYPE_CREATE(0.5);
  this->Invoke("normalize", Output, Input_p);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(GeometricFunctions_TestCase_4, normalize);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, GeometricFunctions_TestCase_4, OCLDevicesTypes_4);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class GeometricFunctions_TestCase_5 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                   typename TyTy::Type> { };

#define LAST_ALL_TYPES_5(D)     \
  DeviceTypePair<D, cl_float>,  \
  DeviceTypePair<D, cl_float2>, \
  DeviceTypePair<D, cl_float3>, \
  DeviceTypePair<D, cl_float4>

#define ALL_DEVICE_TYPES_5 \
  LAST_ALL_TYPES_5(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_5> OCLDevicesTypes_5;

TYPED_TEST_CASE_P(GeometricFunctions_TestCase_5);

/*  Function        |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  fast_normalize  |   ff 
 */

TYPED_TEST_P(GeometricFunctions_TestCase_5, fast_normalize) {
  GENTYPE_DECLARE(Input_p);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_p = GENTYPE_CREATE(2);
  if (GENTYPE_CHECK(cl_float))
    Expected = GENTYPE_CREATE(1);
  else if (GENTYPE_CHECK(cl_float2))
    Expected = GENTYPE_CREATE(CL_M_SQRT1_2);
  else if (GENTYPE_CHECK(cl_float3))
    Expected = GENTYPE_CREATE(0.5);
  else if (GENTYPE_CHECK(cl_float4))
    Expected = GENTYPE_CREATE(0.5);
  this->Invoke("fast_normalize", Output, Input_p);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(GeometricFunctions_TestCase_5, fast_normalize);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, GeometricFunctions_TestCase_5, OCLDevicesTypes_5);

