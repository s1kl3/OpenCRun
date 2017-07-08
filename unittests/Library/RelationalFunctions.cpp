
#include "LibraryFixture.h"

/* INFO: cl_float3 is identical in size, alignment and behavior to cl_float4.
 * A cl_float3 vector type is actually defined by the OpenCL header files as
 * a cl_float4 type and, thus, they can't be distinguished by LibraryFixture.
 */

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class RelationalFunctions_TestCase_1 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                    typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_1(D)                                \
  DeviceTypePair<D, std::tuple<cl_int, cl_float, cl_float>>,  \
  DeviceTypePair<D, std::tuple<cl_int, cl_double, cl_double>>

#define LAST_VECTOR_TYPES_1(D, S)                                             \
  DeviceTypePair<D, std::tuple<cl_int ## S, cl_float ## S, cl_float ## S>>,   \
  DeviceTypePair<D, std::tuple<cl_long ## S, cl_double ## S, cl_double ## S>>
#else
#define LAST_SCALAR_TYPES_1(D) \
  DeviceTypePair<D, std::tuple<cl_int, cl_float, cl_float>>

#define LAST_VECTOR_TYPES_1(D, S) \
  DeviceTypePair<D, std::tuple<cl_int ## S, cl_float ## S, cl_float ## S>>
#endif

#define LAST_ALL_TYPES_1(D)     \
  LAST_SCALAR_TYPES_1(D),       \
  LAST_VECTOR_TYPES_1(D, 2),    \
  LAST_VECTOR_TYPES_1(D, 3),    \
  LAST_VECTOR_TYPES_1(D, 4),    \
  LAST_VECTOR_TYPES_1(D, 8),    \
  LAST_VECTOR_TYPES_1(D, 16)

#define ALL_DEVICE_TYPES_1 \
  LAST_ALL_TYPES_1(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_1> OCLDevicesTypes_1;

TYPED_TEST_CASE_P(RelationalFunctions_TestCase_1);

/*  Function        |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  isequal         |   Snff, Sidd
 *  isnotequal      |   Snff, Sidd
 *  isgreater       |   Snff, Sidd
 *  isgreaterequal  |   Snff, Sidd
 *  isless          |   Snff, Sidd
 *  islessequal     |   Snff, Sidd
 *  islessgreater   |   Snff, Sidd
 *  isordered       |   Snff, Sidd
 *  isunordered     |   Snff, Sidd
 */

TYPED_TEST_P(RelationalFunctions_TestCase_1, isequal) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isequal", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.5);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isequal", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_1, isnotequal) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.5);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isnotequal", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isnotequal", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_1, isgreater) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isgreater", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.6);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.5);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isgreater", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_1, isgreaterequal) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isgreaterequal", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.5);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isgreaterequal", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.6);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.5);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isgreaterequal", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_1, isless) {
 GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
 GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
 GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
 GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.6);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isless", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
   GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
   GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isless", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_1, islessequal) {
 GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
 GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
 GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
 GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.6);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("islessequal", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.5);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
   GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
   GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("islessequal", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
   GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
   GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("islessequal", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_1, islessgreater) {
 GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
 GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
 GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
 GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("islessgreater", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
   GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
   GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("islessgreater", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.6);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.5);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
   GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
   GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("islessgreater", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_1, isordered) {
 GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
 GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
 GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
 GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_NAN);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isordered", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
   GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
   GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isordered", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_1, isunordered) {
 GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
 GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
 GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
 GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isunordered", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_NAN);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0.6);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
   GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
   GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isunordered", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_1, isequal,
                                                           isnotequal,
                                                           isgreater,
                                                           isgreaterequal,
                                                           isless,
                                                           islessequal,
                                                           islessgreater,
                                                           isordered,
                                                           isunordered);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_1, OCLDevicesTypes_1);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class RelationalFunctions_TestCase_2 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                    typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_2(D)                      \
  DeviceTypePair<D, std::tuple<cl_int, cl_float>>,  \
  DeviceTypePair<D, std::tuple<cl_int, cl_double>>

#define LAST_VECTOR_TYPES_2(D, S)                               \
  DeviceTypePair<D, std::tuple<cl_int ## S, cl_float ## S>>,    \
  DeviceTypePair<D, std::tuple<cl_long ## S, cl_double ## S>>
#else
#define LAST_SCALAR_TYPES_2(D) \
  DeviceTypePair<D, std::tuple<cl_int, cl_float>>

#define LAST_VECTOR_TYPES_2(D, S) \
  DeviceTypePair<D, std::tuple<cl_int ## S, cl_float ## S>>
#endif

#define LAST_ALL_TYPES_2(D)     \
  LAST_SCALAR_TYPES_2(D),       \
  LAST_VECTOR_TYPES_2(D, 2),    \
  LAST_VECTOR_TYPES_2(D, 3),    \
  LAST_VECTOR_TYPES_2(D, 4),    \
  LAST_VECTOR_TYPES_2(D, 8),    \
  LAST_VECTOR_TYPES_2(D, 16)

#define ALL_DEVICE_TYPES_2 \
  LAST_ALL_TYPES_2(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_2> OCLDevicesTypes_2;

TYPED_TEST_CASE_P(RelationalFunctions_TestCase_2);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -------------------------------------------------
 *  isfinite    |   Snf, Sid
 *  isinf       |   Snf, Sid
 *  isnan       |   Snf, Sid
 *  isnormal    |   Snf, Sid
 *  signbit     |   Snf, Sid
 */

TYPED_TEST_P(RelationalFunctions_TestCase_2, isfinite) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_HUGE_VALF);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isfinite", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isfinite", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_2, isinf) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isinf", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_HUGE_VALF);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isinf", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, -CL_HUGE_VALF);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isinf", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_2, isnan) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isnan", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_NAN);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("isnan", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_2, isnormal) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input = GENTYPE_CHECK_TUPLE_ELEMENT_DOUBLE(1) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_M_E) :
    GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_M_E_F);
  Expected =  GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(0) > 1 ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("isnormal", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isnormal", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input = GENTYPE_CHECK_TUPLE_ELEMENT_DOUBLE(1) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_HUGE_VAL) :
    GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_HUGE_VALF);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isnormal", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, NAN);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isnormal", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input = GENTYPE_CHECK_TUPLE_ELEMENT_DOUBLE(1) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_DBL_MIN/2) :
    GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_FLT_MIN/2);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("isnormal", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_2, signbit) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("signbit", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, -0.5);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT(0, cl_int) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, 1) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
  this->Invoke("signbit", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_2, isfinite,
                                                           isinf,
                                                           isnan,
                                                           isnormal,
                                                           signbit);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_2, OCLDevicesTypes_2);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class RelationalFunctions_TestCase_3 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                  typename TyTy::Type> { };

#define LAST_SCALAR_TYPES_3(D)                      \
  DeviceTypePair<D, std::tuple<cl_int, cl_char>>,   \
  DeviceTypePair<D, std::tuple<cl_int, cl_short>>,  \
  DeviceTypePair<D, std::tuple<cl_int, cl_int>>,    \
  DeviceTypePair<D, std::tuple<cl_int, cl_long>>

#define LAST_VECTOR_TYPES_3(D, S)                       \
  DeviceTypePair<D, std::tuple<cl_int, cl_char ## S>>,  \
  DeviceTypePair<D, std::tuple<cl_int, cl_short ## S>>, \
  DeviceTypePair<D, std::tuple<cl_int, cl_int ## S>>,   \
  DeviceTypePair<D, std::tuple<cl_int, cl_long ## S>>

#define LAST_ALL_TYPES_3(D)  \
  LAST_SCALAR_TYPES_3(D),    \
  LAST_VECTOR_TYPES_3(D, 2), \
  LAST_VECTOR_TYPES_3(D, 3), \
  LAST_VECTOR_TYPES_3(D, 4), \
  LAST_VECTOR_TYPES_3(D, 8), \
  LAST_VECTOR_TYPES_3(D, 16)

#define ALL_DEVICE_TYPES_3 \
  LAST_ALL_TYPES_3(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_3> OCLDevicesTypes_3;

TYPED_TEST_CASE_P(RelationalFunctions_TestCase_3);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -------------------------------------------------
 *  any         |   CnSi
 *  all         |   CnSi
 */

TYPED_TEST_P(RelationalFunctions_TestCase_3, any) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("any", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, -1);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("any", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(RelationalFunctions_TestCase_3, all) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("all", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input = GENTYPE_CREATE_TUPLE_ELEMENT(1, -1);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("all", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_3, any,
                                                           all);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_3, OCLDevicesTypes_3);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class RelationalFunctions_TestCase_4 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                    typename TyTy::Type> { };

#define LAST_SCALAR_TYPES_4(D)  \
  DeviceTypePair<D, cl_char>,   \
  DeviceTypePair<D, cl_uchar>,  \
  DeviceTypePair<D, cl_short>,  \
  DeviceTypePair<D, cl_ushort>, \
  DeviceTypePair<D, cl_int>,    \
  DeviceTypePair<D, cl_uint>,   \
  DeviceTypePair<D, cl_long>,   \
  DeviceTypePair<D, cl_ulong>

#define LAST_VECTOR_TYPES_4(D, S)       \
  DeviceTypePair<D, cl_char ## S>,      \
  DeviceTypePair<D, cl_uchar ## S>,     \
  DeviceTypePair<D, cl_short ## S>,     \
  DeviceTypePair<D, cl_ushort ## S>,    \
  DeviceTypePair<D, cl_int ## S>,       \
  DeviceTypePair<D, cl_uint ## S>,      \
  DeviceTypePair<D, cl_long ## S>,      \
  DeviceTypePair<D, cl_ulong ## S>

#define LAST_ALL_TYPES_4(D)  \
  LAST_SCALAR_TYPES_4(D),    \
  LAST_VECTOR_TYPES_4(D, 2), \
  LAST_VECTOR_TYPES_4(D, 3), \
  LAST_VECTOR_TYPES_4(D, 4), \
  LAST_VECTOR_TYPES_4(D, 8), \
  LAST_VECTOR_TYPES_4(D, 16)

#define ALL_DEVICE_TYPES_4 \
  LAST_ALL_TYPES_4(CPUDev)

// Types needs splittin so that they don't exceed the maximum number
// of allowed testing::Types template parameters (see subsequent test
// case).
typedef testing::Types<ALL_DEVICE_TYPES_4> OCLDevicesTypes_4;

TYPED_TEST_CASE_P(RelationalFunctions_TestCase_4);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -------------------------------------------------
 *  bitselect   |   iiii
 *
 */

TYPED_TEST_P(RelationalFunctions_TestCase_4, bitselect) {
  GENTYPE_DECLARE(Input_a);
  GENTYPE_DECLARE(Input_b);
  GENTYPE_DECLARE(Input_c);
  GENTYPE_DECLARE(Output);
  GENTYPE_DECLARE(Expected);

  Input_a = GENTYPE_CREATE(0x5555555555555555U);
  Input_b = GENTYPE_CREATE(0xAAAAAAAAAAAAAAAAU);
  Input_c = GENTYPE_CREATE(0xAAAAAAAAAAAAAAAAU);
  Expected =  GENTYPE_CREATE(0xFFFFFFFFFFFFFFFFU);
  this->Invoke("bitselect", Output, Input_a, Input_b, Input_c);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_4, bitselect);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_4, OCLDevicesTypes_4);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class RelationalFunctions_TestCase_5: public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                   typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_5(D)  \
  DeviceTypePair<D, cl_float>,  \
  DeviceTypePair<D, cl_double>

#define LAST_VECTOR_TYPES_5(D, S)   \
  DeviceTypePair<D, cl_float ## S>, \
  DeviceTypePair<D, cl_double ## S>
#else
#define LAST_SCALAR_TYPES_5(D)  \
  DeviceTypePair<D, cl_float>

#define LAST_VECTOR_TYPES_5(D, S)   \
  DeviceTypePair<D, cl_float ## S>
#endif

#define LAST_ALL_TYPES_5(D)  \
  LAST_SCALAR_TYPES_5(D),    \
  LAST_VECTOR_TYPES_5(D, 2), \
  LAST_VECTOR_TYPES_5(D, 3), \
  LAST_VECTOR_TYPES_5(D, 4), \
  LAST_VECTOR_TYPES_5(D, 8), \
  LAST_VECTOR_TYPES_5(D, 16)

#define ALL_DEVICE_TYPES_5 \
  LAST_ALL_TYPES_5(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_5> OCLDevicesTypes_5;

TYPED_TEST_CASE_P(RelationalFunctions_TestCase_5);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -------------------------------------------------
 *  bitselect   |   ffff, dddd
 */

TYPED_TEST_P(RelationalFunctions_TestCase_5, bitselect) {
  GENTYPE_DECLARE(Input_a);
  GENTYPE_DECLARE(Input_b);
  GENTYPE_DECLARE(Input_c);
  GENTYPE_DECLARE(Output);
  GENTYPE_DECLARE(Expected);

  if (GENTYPE_CHECK_FLOAT) {
    typedef union _float_bits {
      float f;
      int32_t i;
    } float_bits;

    float_bits a_fp32, b_fp32, c_fp32, e_fp32;

    a_fp32.i = 0x55555555;
    b_fp32.i = 0x22222222;
    c_fp32.i = 0x22222222;
    e_fp32.i = 0x77777777;

    Input_a = GENTYPE_CREATE(a_fp32.f);
    Input_b = GENTYPE_CREATE(b_fp32.f);
    Input_c = GENTYPE_CREATE(c_fp32.f);
    Expected = GENTYPE_CREATE(e_fp32.f);
  } else if (GENTYPE_CHECK_DOUBLE) {
    typedef union _double_bits {
      double d;
      int64_t l;
    } double_bits;

    double_bits a_fp64, b_fp64, c_fp64, e_fp64;

    a_fp64.l = 0x5555555555555555;
    b_fp64.l = 0x2222222222222222;
    c_fp64.l = 0x2222222222222222;
    e_fp64.l = 0x7777777777777777;

    Input_a = GENTYPE_CREATE(a_fp64.d);
    Input_b = GENTYPE_CREATE(b_fp64.d);
    Input_c = GENTYPE_CREATE(c_fp64.d);
    Expected = GENTYPE_CREATE(e_fp64.d);
  }

  this->Invoke("bitselect", Output, Input_a, Input_b, Input_c);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_5, bitselect);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_5, OCLDevicesTypes_5);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class RelationalFunctions_TestCase_Char_6: public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                        typename TyTy::Type> { };

template <typename TyTy>
class RelationalFunctions_TestCase_Char_U_6: public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                          typename TyTy::Type> { };

template <typename TyTy>
class RelationalFunctions_TestCase_Short_6: public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                         typename TyTy::Type> { };

template <typename TyTy>
class RelationalFunctions_TestCase_Short_U_6: public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                           typename TyTy::Type> { };

template <typename TyTy>
class RelationalFunctions_TestCase_Int_6: public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                       typename TyTy::Type> { };

template <typename TyTy>
class RelationalFunctions_TestCase_Int_U_6: public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                         typename TyTy::Type> { };

template <typename TyTy>
class RelationalFunctions_TestCase_Long_6: public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                        typename TyTy::Type> { };

template <typename TyTy>
class RelationalFunctions_TestCase_Long_U_6: public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                          typename TyTy::Type> { };

template <typename TyTy>
class RelationalFunctions_TestCase_Float_6: public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                         typename TyTy::Type> { };

template <typename TyTy>
class RelationalFunctions_TestCase_Float_U_6: public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                           typename TyTy::Type> { };

#define LAST_SCALAR_TYPES_6(D, Ty)                                                \
  DeviceTypePair<D, std::tuple<cl_ ## Ty, cl_ ## Ty, cl_ ## Ty, cl_char>>,        \
  DeviceTypePair<D, std::tuple<cl_ ## Ty, cl_ ## Ty, cl_ ## Ty, cl_short>>,       \
  DeviceTypePair<D, std::tuple<cl_ ## Ty, cl_ ## Ty, cl_ ## Ty, cl_int>>,         \
  DeviceTypePair<D, std::tuple<cl_ ## Ty, cl_ ## Ty, cl_ ## Ty, cl_long>>,        \
  DeviceTypePair<D, std::tuple<cl_u ## Ty, cl_u ## Ty, cl_u ## Ty, cl_char>>,     \
  DeviceTypePair<D, std::tuple<cl_u ## Ty, cl_u ## Ty, cl_u ## Ty, cl_short>>,    \
  DeviceTypePair<D, std::tuple<cl_u ## Ty, cl_u ## Ty, cl_u ## Ty, cl_int>>,      \
  DeviceTypePair<D, std::tuple<cl_u ## Ty, cl_u ## Ty, cl_u ## Ty, cl_long>>

#define LAST_SCALAR_TYPES_U_6(D, Ty)                                            \
  DeviceTypePair<D, std::tuple<cl_ ## Ty, cl_ ## Ty, cl_ ## Ty, cl_uchar>>,     \
  DeviceTypePair<D, std::tuple<cl_ ## Ty, cl_ ## Ty, cl_ ## Ty, cl_ushort>>,    \
  DeviceTypePair<D, std::tuple<cl_ ## Ty, cl_ ## Ty, cl_ ## Ty, cl_uint>>,      \
  DeviceTypePair<D, std::tuple<cl_ ## Ty, cl_ ## Ty, cl_ ## Ty, cl_ulong>>,     \
  DeviceTypePair<D, std::tuple<cl_u ## Ty, cl_u ## Ty, cl_u ## Ty, cl_uchar>>,  \
  DeviceTypePair<D, std::tuple<cl_u ## Ty, cl_u ## Ty, cl_u ## Ty, cl_ushort>>, \
  DeviceTypePair<D, std::tuple<cl_u ## Ty, cl_u ## Ty, cl_u ## Ty, cl_uint>>,   \
  DeviceTypePair<D, std::tuple<cl_u ## Ty, cl_u ## Ty, cl_u ## Ty, cl_ulong>>

#define LAST_VECTOR_TYPES_6(D, Ty, S)                                                               \
  DeviceTypePair<D, std::tuple<cl_ ## Ty ## S, cl_ ## Ty ## S, cl_ ## Ty ## S, cl_char ## S>>,      \
  DeviceTypePair<D, std::tuple<cl_ ## Ty ## S, cl_ ## Ty ## S, cl_ ## Ty ## S, cl_short ## S>>,     \
  DeviceTypePair<D, std::tuple<cl_ ## Ty ## S, cl_ ## Ty ## S, cl_ ## Ty ## S, cl_int ## S>>,       \
  DeviceTypePair<D, std::tuple<cl_ ## Ty ## S, cl_ ## Ty ## S, cl_ ## Ty ## S, cl_long ## S>>,      \
  DeviceTypePair<D, std::tuple<cl_u ## Ty ## S, cl_u ## Ty ## S, cl_u ## Ty ## S, cl_char ## S>>,   \
  DeviceTypePair<D, std::tuple<cl_u ## Ty ## S, cl_u ## Ty ## S, cl_u ## Ty ## S, cl_short ## S>>,  \
  DeviceTypePair<D, std::tuple<cl_u ## Ty ## S, cl_u ## Ty ## S, cl_u ## Ty ## S, cl_int ## S>>,    \
  DeviceTypePair<D, std::tuple<cl_u ## Ty ## S, cl_u ## Ty ## S, cl_u ## Ty ## S, cl_long ## S>>

#define LAST_VECTOR_TYPES_U_6(D, Ty, S)                                                             \
  DeviceTypePair<D, std::tuple<cl_ ## Ty ## S, cl_ ## Ty ## S, cl_ ## Ty ## S, cl_uchar ## S>>,     \
  DeviceTypePair<D, std::tuple<cl_ ## Ty ## S, cl_ ## Ty ## S, cl_ ## Ty ## S, cl_ushort ## S>>,    \
  DeviceTypePair<D, std::tuple<cl_ ## Ty ## S, cl_ ## Ty ## S, cl_ ## Ty ## S, cl_uint ## S>>,      \
  DeviceTypePair<D, std::tuple<cl_ ## Ty ## S, cl_ ## Ty ## S, cl_ ## Ty ## S, cl_ulong ## S>>,     \
  DeviceTypePair<D, std::tuple<cl_u ## Ty ## S, cl_u ## Ty ## S, cl_u ## Ty ## S, cl_uchar ## S>>,  \
  DeviceTypePair<D, std::tuple<cl_u ## Ty ## S, cl_u ## Ty ## S, cl_u ## Ty ## S, cl_ushort ## S>>, \
  DeviceTypePair<D, std::tuple<cl_u ## Ty ## S, cl_u ## Ty ## S, cl_u ## Ty ## S, cl_uint ## S>>,   \
  DeviceTypePair<D, std::tuple<cl_u ## Ty ## S, cl_u ## Ty ## S, cl_u ## Ty ## S, cl_ulong ## S>>

#ifdef cl_khr_fp64 
#define LAST_SCALAR_TYPES_FLOAT_6(D)                                            \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_char>>,         \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_short>>,        \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_int>>,          \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_long>>,         \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double, cl_char>>,      \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double, cl_short>>,     \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double, cl_int>>,       \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double, cl_long>>

#define LAST_SCALAR_TYPES_FLOAT_U_6(D)                                          \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_uchar>>,        \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_ushort>>,       \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_uint>>,         \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_ulong>>,        \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double, cl_uchar>>,     \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double, cl_ushort>>,    \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double, cl_uint>>,      \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double, cl_ulong>>

#define LAST_VECTOR_TYPES_FLOAT_6(D, S)                                                         \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_char ## S>>,     \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_short ## S>>,    \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_int ## S>>,      \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_long ## S>>,     \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_double ## S, cl_char ## S>>,  \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_double ## S, cl_short ## S>>, \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_double ## S, cl_int ## S>>,   \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_double ## S, cl_long ## S>>

#define LAST_VECTOR_TYPES_FLOAT_U_6(D, S)                                                           \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_uchar ## S>>,        \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_ushort ## S>>,       \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_uint ## S>>,         \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_ulong ## S>>,        \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_double ## S, cl_uchar ## S>>,     \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_double ## S, cl_ushort ## S>>,    \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_double ## S, cl_uint ## S>>,      \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_double ## S, cl_ulong ## S>>
#else
#define LAST_SCALAR_TYPES_FLOAT_6(D)                                            \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_char>>,         \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_short>>,        \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_int>>,          \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_long>>

#define LAST_SCALAR_TYPES_FLOAT_U_6(D)                                          \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_uchar>>,        \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_ushort>>,       \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_uint>>,         \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_ulong>>

#define LAST_VECTOR_TYPES_FLOAT_6(D, S)                                                         \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_char ## S>>,     \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_short ## S>>,    \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_int ## S>>,      \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_long ## S>>

#define LAST_VECTOR_TYPES_FLOAT_U_6(D, S)                                                       \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_uchar ## S>>,    \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_ushort ## S>>,   \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_uint ## S>>,     \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_ulong ## S>>
#endif

#define LAST_ALL_TYPES_6(D, Ty)     \
  LAST_SCALAR_TYPES_6(D, Ty),       \
  LAST_VECTOR_TYPES_6(D, Ty, 2),    \
  LAST_VECTOR_TYPES_6(D, Ty, 3),    \
  LAST_VECTOR_TYPES_6(D, Ty, 4),    \
  LAST_VECTOR_TYPES_6(D, Ty, 8),    \
  LAST_VECTOR_TYPES_6(D, Ty, 16)

#define LAST_ALL_TYPES_U_6(D, Ty)   \
  LAST_SCALAR_TYPES_U_6(D, Ty),     \
  LAST_VECTOR_TYPES_U_6(D, Ty, 2),  \
  LAST_VECTOR_TYPES_U_6(D, Ty, 3),  \
  LAST_VECTOR_TYPES_U_6(D, Ty, 4),  \
  LAST_VECTOR_TYPES_U_6(D, Ty, 8),  \
  LAST_VECTOR_TYPES_U_6(D, Ty, 16)

#define LAST_ALL_TYPES_FLOAT_6(D)   \
  LAST_SCALAR_TYPES_FLOAT_6(D),     \
  LAST_VECTOR_TYPES_FLOAT_6(D, 2),  \
  LAST_VECTOR_TYPES_FLOAT_6(D, 3),  \
  LAST_VECTOR_TYPES_FLOAT_6(D, 4),  \
  LAST_VECTOR_TYPES_FLOAT_6(D, 8),  \
  LAST_VECTOR_TYPES_FLOAT_6(D, 16)

#define LAST_ALL_TYPES_FLOAT_U_6(D)     \
  LAST_SCALAR_TYPES_FLOAT_U_6(D),       \
  LAST_VECTOR_TYPES_FLOAT_U_6(D, 2),    \
  LAST_VECTOR_TYPES_FLOAT_U_6(D, 3),    \
  LAST_VECTOR_TYPES_FLOAT_U_6(D, 4),    \
  LAST_VECTOR_TYPES_FLOAT_U_6(D, 8),    \
  LAST_VECTOR_TYPES_FLOAT_U_6(D, 16)

#define ALL_DEVICE_TYPES_CHAR_6 \
  LAST_ALL_TYPES_6(CPUDev, char)

#define ALL_DEVICE_TYPES_CHAR_U_6 \
  LAST_ALL_TYPES_U_6(CPUDev, char)

#define ALL_DEVICE_TYPES_SHORT_6 \
  LAST_ALL_TYPES_6(CPUDev, short)

#define ALL_DEVICE_TYPES_SHORT_U_6 \
  LAST_ALL_TYPES_U_6(CPUDev, short)

#define ALL_DEVICE_TYPES_INT_6 \
  LAST_ALL_TYPES_6(CPUDev, int)

#define ALL_DEVICE_TYPES_INT_U_6 \
  LAST_ALL_TYPES_U_6(CPUDev, int)

#define ALL_DEVICE_TYPES_LONG_6 \
  LAST_ALL_TYPES_6(CPUDev, long)

#define ALL_DEVICE_TYPES_LONG_U_6 \
  LAST_ALL_TYPES_U_6(CPUDev, long)

#define ALL_DEVICE_TYPES_FLOAT_6 \
  LAST_ALL_TYPES_FLOAT_6(CPUDev)

#define ALL_DEVICE_TYPES_FLOAT_U_6 \
  LAST_ALL_TYPES_FLOAT_U_6(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_CHAR_6> OCLDevicesTypes_Char_6;
typedef testing::Types<ALL_DEVICE_TYPES_CHAR_U_6> OCLDevicesTypes_Char_U_6;
typedef testing::Types<ALL_DEVICE_TYPES_SHORT_6> OCLDevicesTypes_Short_6;
typedef testing::Types<ALL_DEVICE_TYPES_SHORT_U_6> OCLDevicesTypes_Short_U_6;
typedef testing::Types<ALL_DEVICE_TYPES_INT_6> OCLDevicesTypes_Int_6;
typedef testing::Types<ALL_DEVICE_TYPES_INT_U_6> OCLDevicesTypes_Int_U_6;
typedef testing::Types<ALL_DEVICE_TYPES_LONG_6> OCLDevicesTypes_Long_6;
typedef testing::Types<ALL_DEVICE_TYPES_LONG_U_6> OCLDevicesTypes_Long_U_6;
typedef testing::Types<ALL_DEVICE_TYPES_FLOAT_6> OCLDevicesTypes_Float_6;
typedef testing::Types<ALL_DEVICE_TYPES_FLOAT_U_6> OCLDevicesTypes_Float_U_6;

TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Char_6);
TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Char_U_6);
TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Short_6);
TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Short_U_6);
TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Int_6);
TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Int_U_6);
TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Long_6);
TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Long_U_6);
TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Float_6);
TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Float_U_6);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -------------------------------------------------------------
 *  select      |   CgCgCgCSi, CgCgCgCUi, VgVgVgVSi, VgVgVgVUi
 */

#define RELATIONAL_FUNCTIONS_TEST(Ty)                                   \
TYPED_TEST_P(RelationalFunctions_TestCase_ ## Ty, select) {             \
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_a);                            \
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_b);                            \
  GENTYPE_DECLARE_TUPLE_ELEMENT(3, Input_c);                            \
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);                             \
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);                           \
                                                                        \
  Input_a = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);                         \
  Input_b = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);                         \
                                                                        \
  if (GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED(3)) {                          \
    if (GENTYPE_CHECK_TUPLE_ELEMENT_VECTOR(0)) {                        \
      Input_c = GENTYPE_CREATE_TUPLE_ELEMENT(3,                         \
          INIT_LIST(int, ({ 1, -1, 1, -1, 1, -1, 1, -1,                 \
                            1, -1, 1, -1, 1, -1, 1, -1 })));            \
      Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0,                       \
          INIT_LIST(int, ({ 0, 1, 0, 1, 0, 1, 0, 1,                     \
                            0, 1, 0, 1, 0, 1, 0, 1 })));                \
    } else {                                                            \
      Input_c = GENTYPE_CREATE_TUPLE_ELEMENT(3, 1);                     \
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);                    \
    }                                                                   \
  } else if (GENTYPE_CHECK_TUPLE_ELEMENT_UNSIGNED(3)) {                 \
    Input_c = GENTYPE_CREATE_TUPLE_ELEMENT(3, 0xFFFFFFFFFFFFFFF8U);    \
    Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);                     \
  }                                                                     \
                                                                        \
  this->Invoke("select", Output, Input_a, Input_b, Input_c);            \
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);                 \
                                                                        \
  Input_c = GENTYPE_CREATE_TUPLE_ELEMENT(3, 0);                         \
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);                       \
                                                                        \
  this->Invoke("select", Output, Input_a, Input_b, Input_c);            \
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);                 \
}

RELATIONAL_FUNCTIONS_TEST(Char_6)
RELATIONAL_FUNCTIONS_TEST(Char_U_6)
RELATIONAL_FUNCTIONS_TEST(Short_6)
RELATIONAL_FUNCTIONS_TEST(Short_U_6)
RELATIONAL_FUNCTIONS_TEST(Int_6)
RELATIONAL_FUNCTIONS_TEST(Int_U_6)
RELATIONAL_FUNCTIONS_TEST(Long_6)
RELATIONAL_FUNCTIONS_TEST(Long_U_6)
RELATIONAL_FUNCTIONS_TEST(Float_6)
RELATIONAL_FUNCTIONS_TEST(Float_U_6)

REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Char_6, select); 
REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Char_U_6, select); 
REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Short_6, select); 
REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Short_U_6, select); 
REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Int_6, select); 
REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Int_U_6, select); 
REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Long_6, select); 
REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Long_U_6, select); 
REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Float_6, select); 
REGISTER_TYPED_TEST_CASE_P(RelationalFunctions_TestCase_Float_U_6, select); 

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_Char_6, OCLDevicesTypes_Char_6);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_Char_U_6, OCLDevicesTypes_Char_U_6);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_Short_6, OCLDevicesTypes_Short_6);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_Short_U_6, OCLDevicesTypes_Short_U_6);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_Int_6, OCLDevicesTypes_Int_6);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_Int_U_6, OCLDevicesTypes_Int_U_6);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_Long_6, OCLDevicesTypes_Long_6);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_Long_U_6, OCLDevicesTypes_Long_U_6);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_Float_6, OCLDevicesTypes_Float_6);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, RelationalFunctions_TestCase_Float_U_6, OCLDevicesTypes_Float_U_6);
