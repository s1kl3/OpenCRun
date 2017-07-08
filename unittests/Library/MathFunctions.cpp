
#include "LibraryFixture.h"

/* INFO: cl_float3 is identical in size, alignment and behavior to cl_float4.
 * A cl_float3 vector type is actually defined by the OpenCL header files as
 * a cl_float4 type and, thus, they can't be distinguished by LibraryFixture.
 */

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class MathFunctions_TestCase_1 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                              typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_1(D)  \
  DeviceTypePair<D, cl_float>,  \
  DeviceTypePair<D, cl_double>

#define LAST_VECTOR_TYPES_1(D, S)   \
  DeviceTypePair<D, cl_float ## S>, \
  DeviceTypePair<D, cl_double ## S>
#else
#define LAST_SCALAR_TYPES_1(D)  \
  DeviceTypePair<D, cl_float>
  
#define LAST_VECTOR_TYPES_1(D, S)   \
  DeviceTypePair<D, cl_float ## S>
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

TYPED_TEST_CASE_P(MathFunctions_TestCase_1);

/*  Function        |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  cbrt            |   rr
 *  ceil            |   rr
 *  copysign        |   rrr
 *  erf             |   rr
 *  erfc            |   rr
 *  fabs            |   rr
 *  fdim            |   rrr
 *  floor           |   rr
 *  fma             |   rrrr
 *  fmax            |   rrr
 *  fmin            |   rrr
 *  fmod            |   rrr
 *  hypot           |   rrr
 *  lgamma          |   rr
 *  mad             |   rrrr
 *  maxmag          |   rrr
 *  minmag          |   rrr
 *  nextafter       |   rrr
 *  pow             |   rrr
 *  powr            |   rrr
 *  remainder       |   rrr
 *  rint            |   rr
 *  round           |   rr
 *  rsqrt           |   rr
 *  sqrt            |   rr
 *  tgamma          |   rr
 *  trunc           |   rr
 */

TYPED_TEST_P(MathFunctions_TestCase_1, cbrt) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(-8);
  Expected = GENTYPE_CREATE(-2);
  this->Invoke("cbrt", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("cbrt", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(8);
  Expected = GENTYPE_CREATE(2);
  this->Invoke("cbrt", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, ceil) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(M_PI);
  Expected = GENTYPE_CREATE(4);
  this->Invoke("ceil", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(M_E);
  Expected = GENTYPE_CREATE(3);
  this->Invoke("ceil", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(-M_PI);
  Expected = GENTYPE_CREATE(-3);
  this->Invoke("ceil", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, copysign) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(M_E);
  Input_2 = GENTYPE_CREATE(M_LN10);
  Expected = GENTYPE_CREATE(M_E);
  this->Invoke("copysign", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(M_E);
  Input_2 = GENTYPE_CREATE(-M_LN10);
  Expected = GENTYPE_CREATE(-M_E);
  this->Invoke("copysign", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_1, erf) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("erf", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CHECK_DOUBLE ? GENTYPE_CREATE(0.84270079294971489) :
                                    GENTYPE_CREATE(0.84270078f);
  this->Invoke("erf", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, erfc) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("erfc", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected =  GENTYPE_CHECK_DOUBLE ? GENTYPE_CREATE(0.15729920705028513) :
                                     GENTYPE_CREATE(0.15729921);
  this->Invoke("erfc", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, fabs) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(-M_PI);
  Expected = GENTYPE_CREATE(M_PI);
  this->Invoke("fabs", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, fdim) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(1);
  Input_2 = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("fdim", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(-M_PI);
  Input_2 = GENTYPE_CREATE(M_PI);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("fdim", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, floor) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(M_PI);
  Expected = GENTYPE_CREATE(3);
  this->Invoke("floor", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(-M_PI);
  Expected = GENTYPE_CREATE(-4);
  this->Invoke("floor", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, fma) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Input_3);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(-0.5);
  Input_2 = GENTYPE_CREATE(2);
  Input_3 = GENTYPE_CREATE(3);
  Expected = GENTYPE_CREATE(2);
  this->Invoke("fma", Output, Input_1, Input_2, Input_3);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, fmax) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(1);
  Input_2 = GENTYPE_CREATE(2);
  Expected = GENTYPE_CREATE(2);
  this->Invoke("fmax", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(2);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(2);
  this->Invoke("fmax", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, fmin) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(1);
  Input_2 = GENTYPE_CREATE(2);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("fmin", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(2);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("fmin", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, fmod) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(1);
  Input_2 = GENTYPE_CREATE(2);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("fmod", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(2);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("fmod", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, hypot) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(M_SQRT2);
  Input_2 = GENTYPE_CREATE(M_SQRT2);
  Expected = GENTYPE_CREATE(2);
  this->Invoke("hypot", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, lgamma) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("lgamma", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(3);
  Expected = GENTYPE_CREATE(CL_M_LN2);
  this->Invoke("lgamma", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, mad) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Input_3);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(0);
  Input_2 = GENTYPE_CREATE(1);
  Input_3 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("mad", Output, Input_1, Input_2, Input_3);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, maxmag) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(-2);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(-2);
  this->Invoke("maxmag", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(1);
  Input_2 = GENTYPE_CREATE(-2);
  Expected = GENTYPE_CREATE(-2);
  this->Invoke("maxmag", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(1);
  Input_2 = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("maxmag", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, minmag) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(-2);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("minmag", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(1);
  Input_2 = GENTYPE_CREATE(-2);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("minmag", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(1);
  Input_2 = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(-1);
  this->Invoke("minmag", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_1, nextafter) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(1.5);
  Input_2 = GENTYPE_CREATE(0);
  Expected = GENTYPE_CHECK_DOUBLE ? GENTYPE_CREATE(1.4999999999999998) :
                                    GENTYPE_CREATE(1.4999999);
  this->Invoke("nextafter", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, pow) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(2);
  Input_2 = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("pow", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(2);
  Input_2 = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(0.5);
  this->Invoke("pow", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(4);
  Input_2 = GENTYPE_CREATE(-0.5);
  Expected = GENTYPE_CREATE(0.5);
  this->Invoke("pow", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, powr) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(2);
  Input_2 = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("powr", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(2);
  Input_2 = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(0.5);
  this->Invoke("powr", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_1, remainder) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(0);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("remainder", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(0);
  Input_2 = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("remainder", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(1);
  Input_2 = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("remainder", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(-1);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(-0);
  this->Invoke("remainder", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(3);
  Input_2 = GENTYPE_CREATE(2);
  Expected = GENTYPE_CREATE(-1);
  this->Invoke("remainder", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(-3);
  Input_2 = GENTYPE_CREATE(2);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("remainder", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, rint) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(M_E);
  Expected = GENTYPE_CREATE(3);
  this->Invoke("rint", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1.5);
  Expected = GENTYPE_CREATE(2);
  this->Invoke("rint", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, round) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(M_E);
  Expected = GENTYPE_CREATE(3);
  this->Invoke("round", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(M_PI);
  Expected = GENTYPE_CREATE(3);
  this->Invoke("round", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1.5);
  Expected = GENTYPE_CREATE(2);
  this->Invoke("round", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(-1.5);
  Expected = GENTYPE_CREATE(-2);
  this->Invoke("round", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, rsqrt) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("rsqrt", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(4);
  Expected = GENTYPE_CREATE(0.5);
  this->Invoke("rsqrt", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_1, sqrt) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("sqrt", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("sqrt", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(4);
  Expected = GENTYPE_CREATE(2);
  this->Invoke("sqrt", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_1, tgamma) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("tgamma", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(3);
  Expected = GENTYPE_CREATE(2);
  this->Invoke("tgamma", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_1, trunc) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(M_PI);
  Expected = GENTYPE_CREATE(3);
  this->Invoke("trunc", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(-M_PI);
  Expected = GENTYPE_CREATE(-3);
  this->Invoke("trunc", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(MathFunctions_TestCase_1, cbrt,
                                                     ceil,
                                                     copysign,
// TODO: fill the gap (half_divide, native_divide).
                                                     erf,
                                                     erfc,
                                                     fabs,
                                                     fdim,
                                                     floor,
                                                     fma,
                                                     fmax,
                                                     fmin,
                                                     fmod,
                                                     hypot,
                                                     lgamma,
                                                     mad,
                                                     maxmag,
                                                     minmag,
// TODO: fill the gap (nan).
                                                     nextafter,
                                                     pow,
                                                     powr,
// TODO: fill the gap (half_powr, native_powr,
//                     half_recip, native_recip).
                                                     remainder,
                                                     rint,
                                                     round,
                                                     rsqrt,
// TODO: fill the gap (half_rsqrt, native_rsqrt).
                                                     sqrt,
// TODO: fill the gap (half_sqrt, native_sqrt).
                                                     tgamma,
                                                     trunc);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, MathFunctions_TestCase_1, OCLDevicesTypes_1);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class MathFunctions_TestCase_2 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                              typename TyTy::Type> { };

#define ALL_DEVICE_TYPES_2 \
  ALL_DEVICE_TYPES_1

typedef testing::Types<ALL_DEVICE_TYPES_2> OCLDevicesTypes_2;

TYPED_TEST_CASE_P(MathFunctions_TestCase_2);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  acos        |   rr
 *  acosh       |   rr
 *  acospi      |   rr
 *  asin        |   rr
 *  asinh       |   rr
 *  asinpi      |   rr
 *  atan        |   rr
 *  atan2       |   rrr
 *  atanh       |   rr
 *  atanpi      |   rr
 *  atan2pi     |   rrr
 *  cos         |   rr    
 *  cosh        |   rr
 *  cospi       |   rr
 *  sin         |   rr
 *  sinh        |   rr
 *  sinpi       |   rr
 *  tan         |   rr
 *  tanh        |   rr
 *  tanpi       |   rr
 */

TYPED_TEST_P(MathFunctions_TestCase_2, acos) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(M_PI);
  this->Invoke("acos", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(M_PI_2);
  this->Invoke("acos", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("acos", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, acosh) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  this->Invoke("acosh", Output, Input);
  ASSERT_GENTYPE_IS_NAN(Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("acosh", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, acospi) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("acospi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0.5);
  this->Invoke("acospi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("acospi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, asin) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(-M_PI_2);
  this->Invoke("asin", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("asin", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(M_PI_2);
  this->Invoke("asin", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, asinh) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("asinh", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Input);
}

TYPED_TEST_P(MathFunctions_TestCase_2, asinpi) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(-0.5);
  this->Invoke("asinpi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("asinpi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0.5);
  this->Invoke("asinpi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, atan) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(-M_PI_4);
  this->Invoke("atan", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("atan", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(M_PI_4);
  this->Invoke("atan", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, atan2) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(-1);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(-M_PI_4);
  this->Invoke("atan2", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(0);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("atan2", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(1);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(M_PI_4);
  this->Invoke("atan2", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, atanh) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("atanh", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Input);
}

TYPED_TEST_P(MathFunctions_TestCase_2, atanpi) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(-1);
  Expected = GENTYPE_CREATE(-0.25);
  this->Invoke("atanpi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("atanpi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0.25);
  this->Invoke("atanpi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, atan2pi) {
  GENTYPE_DECLARE(Input_1);
  GENTYPE_DECLARE(Input_2);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_1 = GENTYPE_CREATE(-1);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(-0.25);
  this->Invoke("atan2pi", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(0);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("atan2pi", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input_1 = GENTYPE_CREATE(1);
  Input_2 = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0.25);
  this->Invoke("atan2pi", Output, Input_1, Input_2);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, cos) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("cos", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  // Do no perform the check near 1/2 * pi, the FPU loss precision here!

  Input = GENTYPE_CREATE(M_PI);
  Expected = GENTYPE_CREATE(-1);
  this->Invoke("cos", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  // Do no perform the check near 3/4 * pi, the FPU loss precision here!

  Input = GENTYPE_CREATE(2 * M_PI);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("cos", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_2, cosh) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("cosh", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE((M_E + (1/M_E))/2);
  this->Invoke("cosh", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, cospi) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("cospi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(-1);
  this->Invoke("cospi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(2);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("cospi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, sin) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("sin", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(M_PI_2);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("sin", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  // Do not perform the check near pi, the FPU loss precision here!

  Input = GENTYPE_CREATE(M_PI + M_PI_2);
  Expected = GENTYPE_CREATE(-1);
  this->Invoke("sin", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  // Do not perform the check near 2 * pi, the FPU loss precision here!
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_2, sinh) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("sinh", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE((M_E - (1/M_E))/2);
  this->Invoke("sinh", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, sinpi) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("sinpi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(0.5);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("sinpi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, tan) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("tan", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(M_PI_4);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("tan", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  // Do no perform the check near pi, the FPU loss precision here!

  Input = GENTYPE_CREATE(3 * M_PI_4);
  Expected = GENTYPE_CREATE(-1);
  this->Invoke("tan", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  // Do no perform the check near 2 * pi, the FPU loss precision here!
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_2, tanh) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("tanh", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE((M_E - (1/M_E))/(M_E + (1/M_E)));
  this->Invoke("tanh", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_2, tanpi) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("tanpi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(0.25);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("tanpi", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}


REGISTER_TYPED_TEST_CASE_P(MathFunctions_TestCase_2, acos,
                                                     acosh,
                                                     acospi,
                                                     asin,
                                                     asinh,
                                                     asinpi,
                                                     atan,
                                                     atan2,
                                                     atanh,
                                                     atanpi,
                                                     atan2pi,
                                                     cos,
// TODO: fill the gap (half_cos, native_cos).
                                                     cosh,
                                                     cospi,
                                                     sin,
// TODO: fill the gap (half_sin, native_sin).
                                                     sinh,
                                                     sinpi,
                                                     tan,
// TODO: fill the gap (half_tan, native_tan).
                                                     tanh,
                                                     tanpi);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, MathFunctions_TestCase_2, OCLDevicesTypes_2);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class MathFunctions_TestCase_3 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                              typename TyTy::Type> { };

#define ALL_DEVICE_TYPES_3 \
  ALL_DEVICE_TYPES_1

typedef testing::Types<ALL_DEVICE_TYPES_3> OCLDevicesTypes_3;

TYPED_TEST_CASE_P(MathFunctions_TestCase_3);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  exp         |   rr
 *  exp2        |   rr
 *  exp10       |   rr
 *  expm1       |   rr
 *  log         |   rr
 *  log2        |   rr
 *  log10       |   rr
 *  log1p       |   rr
 *  logb        |   rr
 */

TYPED_TEST_P(MathFunctions_TestCase_3, exp) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("exp", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_3, exp2) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("exp2", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(M_LOG2E);
  Expected = GENTYPE_CREATE(M_E);
  this->Invoke("exp2", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_3, exp10) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("exp10", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(M_LOG10E);
  Expected = GENTYPE_CREATE(M_E);
  this->Invoke("exp10", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_3, expm1) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("expm1", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_3, log) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("log", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(M_E);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("log", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_3, log2) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("log2", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(2);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("log2", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_3, log10) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("log10", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(10);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("log10", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

// TODO: fill the gap.

TYPED_TEST_P(MathFunctions_TestCase_3, log1p) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("log1p", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);

  Input = GENTYPE_CREATE(M_E - 1);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("log1p", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_3, logb) {
  GENTYPE_DECLARE(Input);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input = GENTYPE_CREATE(0x1.0p-1);
  Expected = GENTYPE_CREATE(-1);
  this->Invoke("logb", Output, Input);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(MathFunctions_TestCase_3, exp,
// TODO: fill the gap (half_exp, native_exp).
                                                     exp2,
// TODO: fill the gap (half_exp2, native_exp2).
                                                     exp10,
// TODO: fill the gap (half_exp10, native_exp10).
                                                     expm1,
                                                     log,
// TODO: fill the gap (half_log, native_log).
                                                     log2,
// TODO: fill the gap (half_log2, native_log2).
                                                     log10,
// TODO: fill the gap (half_log10, native_log10).
                                                     log1p,
                                                     logb);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, MathFunctions_TestCase_3, OCLDevicesTypes_3);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class MathFunctions_TestCase_4 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                              typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_4(D)                                      \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float *>>,    \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double *>>

#define LAST_VECTOR_TYPES_4(D, S)                                                   \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S *>>,     \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_double ## S *>>
#else
#define LAST_SCALAR_TYPES_4(D)                                  \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float *>>
  
#define LAST_VECTOR_TYPES_4(D, S)                                               \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S *>>
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

TYPED_TEST_CASE_P(MathFunctions_TestCase_4);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  fract       |   ffP1f, ffP2f, ffP4f, ddP1d, ddP2d, ddP4d
 *  modf        |   ffP1f, ffP2f, ffP4f, ddP1d, ddP2d, ddP4d
 *  sincos      |   rrP1r, rrP2r, rrP4r
 */

TYPED_TEST_P(MathFunctions_TestCase_4, fract) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(2, 1, Output_iptr);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(2, Expected_iptr);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0.5);
  Expected_iptr = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(double, ({ 1 })));

  this->Invoke("fract", Output, Input_x, Output_iptr);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(2, Expected_iptr, Output_iptr);
}

TYPED_TEST_P(MathFunctions_TestCase_4, modf) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(2, 1, Output_iptr);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(2, Expected_iptr);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0.5);
  Expected_iptr = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(double, ({ 1 })));

  this->Invoke("modf", Output, Input_x, Output_iptr);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(2, Expected_iptr, Output_iptr);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, -1.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, -0.5);
  Expected_iptr = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(double, ({ -1 })));

  this->Invoke("modf", Output, Input_x, Output_iptr);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(2, Expected_iptr, Output_iptr);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_HUGE_VAL);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0.0);
  Expected_iptr = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(double, ({ CL_HUGE_VAL })));

  this->Invoke("modf", Output, Input_x, Output_iptr);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(2, Expected_iptr, Output_iptr);
}

TYPED_TEST_P(MathFunctions_TestCase_4, sincos) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(2, 1, Output_cosval);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(2, Expected_cosval);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_M_PI_4);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, CL_M_SQRT1_2);
  Expected_cosval = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(double, ({ CL_M_SQRT1_2 })));

  this->Invoke("sincos", Output, Input_x, Output_cosval);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(2, Expected_cosval, Output_cosval);
}

REGISTER_TYPED_TEST_CASE_P(MathFunctions_TestCase_4, fract,
                                                     modf,
                                                     sincos);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, MathFunctions_TestCase_4, OCLDevicesTypes_4);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class MathFunctions_TestCase_5 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                              typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_5(D)                                  \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_int *>>,  \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_int *>>

#define LAST_VECTOR_TYPES_5(D, S)                                               \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_int ## S *>>,   \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_int ## S *>>
#else
#define LAST_SCALAR_TYPES_5(D)                                  \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_int *>>
  
#define LAST_VECTOR_TYPES_5(D, S)                                               \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_int ## S *>>
#endif

#define LAST_ALL_TYPES_5(D)     \
  LAST_SCALAR_TYPES_5(D),       \
  LAST_VECTOR_TYPES_5(D, 2),    \
  LAST_VECTOR_TYPES_5(D, 3),    \
  LAST_VECTOR_TYPES_5(D, 4),    \
  LAST_VECTOR_TYPES_5(D, 8),    \
  LAST_VECTOR_TYPES_5(D, 16)

#define ALL_DEVICE_TYPES_5 \
  LAST_ALL_TYPES_5(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_5> OCLDevicesTypes_5;

TYPED_TEST_CASE_P(MathFunctions_TestCase_5);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ------------------------------------------------------------------
 *  frexp       |   rrP1Sn, rrP2Sn, rrP4Sn
 *  lgamma_r    |   ffP1Sn, ffP2Sn, ffP4Sn, ddP1Sn, ddP2Sn, ddP4Sn
 */

TYPED_TEST_P(MathFunctions_TestCase_5, frexp) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(2, 1, Output_exp);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(2, Expected_exp);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  // CL_FLT_EPSILON = 0x1.0p-23f = 0x0.8p-22f
  // Mantissa = 0x0.8 = 0.5
  // Exponent = -22
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_FLT_EPSILON);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0.5);
  Expected_exp = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ -22 })));

  this->Invoke("frexp", Output, Input_x, Output_exp);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(2, Expected_exp, Output_exp);
}

TYPED_TEST_P(MathFunctions_TestCase_5, lgamma_r) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(2, 1, Output_signp);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(2, Expected_signp);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  Expected_signp = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ 1 })));

  this->Invoke("lgamma_r", Output, Input_x, Output_signp);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(2, Expected_signp, Output_signp);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 3);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, CL_M_LN2);
  Expected_signp = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ 1 })));

  this->Invoke("lgamma_r", Output, Input_x, Output_signp);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(2, Expected_signp, Output_signp);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, -0.5);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1.2655121234846454);
  Expected_signp = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ -1 })));

  this->Invoke("lgamma_r", Output, Input_x, Output_signp);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(2, Expected_signp, Output_signp);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, CL_HUGE_VAL);
  Expected_signp = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ 1 })));

  this->Invoke("lgamma_r", Output, Input_x, Output_signp);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(2, Expected_signp, Output_signp);
}

REGISTER_TYPED_TEST_CASE_P(MathFunctions_TestCase_5, frexp,
                                                     lgamma_r);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, MathFunctions_TestCase_5, OCLDevicesTypes_5);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class MathFunctions_TestCase_6 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                              typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_6(D)                                  \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_int>>,  \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_int>>

#define LAST_VECTOR_TYPES_6(D, S)                                               \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_int ## S>>,   \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_int ## S>>
#else
#define LAST_SCALAR_TYPES_6(D)                                  \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_int>>
  
#define LAST_VECTOR_TYPES_6(D, S)                                               \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_int ## S *>>
#endif

#define LAST_ALL_TYPES_6(D)     \
  LAST_SCALAR_TYPES_6(D),       \
  LAST_VECTOR_TYPES_6(D, 2),    \
  LAST_VECTOR_TYPES_6(D, 3),    \
  LAST_VECTOR_TYPES_6(D, 4),    \
  LAST_VECTOR_TYPES_6(D, 8),    \
  LAST_VECTOR_TYPES_6(D, 16)

#define ALL_DEVICE_TYPES_6 \
  LAST_ALL_TYPES_6(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_6> OCLDevicesTypes_6;

TYPED_TEST_CASE_P(MathFunctions_TestCase_6);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ------------------------------------------------------------------
 *  rootn       |   rrSn
 *  pown        |   rrSn
 */

TYPED_TEST_P(MathFunctions_TestCase_6, rootn) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 2);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 2);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, CL_M_SQRT2);

  this->Invoke("rootn", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0.5);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 2);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, CL_M_SQRT1_2);

  this->Invoke("rootn", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

TYPED_TEST_P(MathFunctions_TestCase_6, pown) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 2);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, -1);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0.5);

  this->Invoke("pown", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(MathFunctions_TestCase_6, rootn,
                                                     pown);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, MathFunctions_TestCase_6, OCLDevicesTypes_6);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class MathFunctions_TestCase_7 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                              typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_7(D)                                                  \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_int *>>,      \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_double, cl_int *>>

#define LAST_VECTOR_TYPES_7(D, S)                                                                   \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_int ## S *>>,      \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_double ## S, cl_int ## S *>>
#else
#define LAST_SCALAR_TYPES_7(D)                                              \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_float, cl_int *>>
  
#define LAST_VECTOR_TYPES_7(D, S)                                                               \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_float ## S, cl_int ## S *>>
#endif

#define LAST_ALL_TYPES_7(D)     \
  LAST_SCALAR_TYPES_7(D),       \
  LAST_VECTOR_TYPES_7(D, 2),    \
  LAST_VECTOR_TYPES_7(D, 3),    \
  LAST_VECTOR_TYPES_7(D, 4),    \
  LAST_VECTOR_TYPES_7(D, 8),    \
  LAST_VECTOR_TYPES_7(D, 16)

#define ALL_DEVICE_TYPES_7 \
  LAST_ALL_TYPES_7(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_7> OCLDevicesTypes_7;

TYPED_TEST_CASE_P(MathFunctions_TestCase_7);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  remquo      |   rrrP1Sn, rrrP2Sn, rrrP4Sn
 */

TYPED_TEST_P(MathFunctions_TestCase_7, remquo) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(3, 1, Output_quo);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(3, Expected_quo);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 2);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  Expected_quo = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(3, INIT_LIST(int, ({ 0 })));

  this->Invoke("remquo", Output, Input_x, Input_y, Output_quo);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(3, Expected_quo, Output_quo);
}

REGISTER_TYPED_TEST_CASE_P(MathFunctions_TestCase_7, remquo);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, MathFunctions_TestCase_7, OCLDevicesTypes_7);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class MathFunctions_TestCase_8 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                              typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_8(D)                                  \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_int>>,    \
  DeviceTypePair<D, std::tuple<cl_double, cl_double, cl_int>>

#define LAST_VECTOR_TYPES_8(D, S)                                               \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_int>>,          \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_int ## S>>,     \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_int>>,        \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_double ## S, cl_int ## S>>
#else
#define LAST_SCALAR_TYPES_8(D)                              \
  DeviceTypePair<D, std::tuple<cl_float, cl_float, cl_int>>
  
#define LAST_VECTOR_TYPES_8(D, S)                                               \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_int>>,          \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_float ## S, cl_int ## S>>
#endif

#define LAST_ALL_TYPES_8(D)     \
  LAST_SCALAR_TYPES_8(D),       \
  LAST_VECTOR_TYPES_8(D, 2),    \
  LAST_VECTOR_TYPES_8(D, 3),    \
  LAST_VECTOR_TYPES_8(D, 4),    \
  LAST_VECTOR_TYPES_8(D, 8),    \
  LAST_VECTOR_TYPES_8(D, 16)

#define ALL_DEVICE_TYPES_8 \
  LAST_ALL_TYPES_8(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_8> OCLDevicesTypes_8;

TYPED_TEST_CASE_P(MathFunctions_TestCase_8);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  ldexp       |   VrVrCn, rrSn
 */

TYPED_TEST_P(MathFunctions_TestCase_8, ldexp) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_k);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
 
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Input_k = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);

  this->Invoke("ldexp", Output, Input_x, Input_k);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  std::valarray<int> k;
  std::valarray<float> expected;
  unsigned VecStep = GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(2);
  if (VecStep > 1) {
    switch (VecStep) {
      case 2:
        k = { 0, 1 };
        expected = { 1, 2 };
        break;
      case 3:
        k = { 0, 1, 2 };
        expected = { 1, 2, 4 };
        break;
      case 4:
        k = { 0, 1, 2, 3 };
        expected = { 1, 2, 4, 8 };
        break;
      case 8:
        k = { 0, 1, 2, 3, 0, 1, 2, 3 };
        expected = { 1, 2, 4, 8, 1, 2, 4, 8 };
        break;
      case 16:
        k = { 0,  1,  2,  3, 0,  1,  2,  3,
              0, -1, -2, -3, 0, -1, -2, -3 };
        expected = { 1,   2,    4,     8, 1,   2,    4,     8,
                     1, 0.5, 0.25, 0.125, 1, 0.5, 0.25, 0.125 };
        break;
    }

    Input_k = GENTYPE_CREATE_TUPLE_ELEMENT(2, k);
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, expected);
    this->Invoke("ldexp", Output, Input_x, Input_k);
    ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  }
}

REGISTER_TYPED_TEST_CASE_P(MathFunctions_TestCase_8, ldexp);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, MathFunctions_TestCase_8, OCLDevicesTypes_8);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class MathFunctions_TestCase_9 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                              typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_9(D)                          \
  DeviceTypePair<D, std::tuple<cl_float, cl_uint>>,     \
  DeviceTypePair<D, std::tuple<cl_double, cl_ulong>>

#define LAST_VECTOR_TYPES_9(D, S)                               \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_uint ## S>>,   \
  DeviceTypePair<D, std::tuple<cl_double ## S, cl_ulong ## S>>
#else
#define LAST_SCALAR_TYPES_9(D) \
  DeviceTypePair<D, std::tuple<cl_float, cl_uint>>
  
#define LAST_VECTOR_TYPES_9(D, S) \
  DeviceTypePair<D, std::tuple<cl_float ## S, cl_uint ## S>>
#endif

#define LAST_ALL_TYPES_9(D)     \
  LAST_SCALAR_TYPES_9(D),       \
  LAST_VECTOR_TYPES_9(D, 2),    \
  LAST_VECTOR_TYPES_9(D, 3),    \
  LAST_VECTOR_TYPES_9(D, 4),    \
  LAST_VECTOR_TYPES_9(D, 8),    \
  LAST_VECTOR_TYPES_9(D, 16)

#define ALL_DEVICE_TYPES_9 \
  LAST_ALL_TYPES_9(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_9> OCLDevicesTypes_9;

TYPED_TEST_CASE_P(MathFunctions_TestCase_9);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  nan         |   fUn_dUl
 */

TYPED_TEST_P(MathFunctions_TestCase_9, nan) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_nancode);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  
  Input_nancode = GENTYPE_CREATE_TUPLE_ELEMENT(1, 5);
  this->Invoke("nan", Output, Input_nancode);
  ASSERT_GENTYPE_TUPLE_ELEMENT_IS_NAN(0, Output);
}

REGISTER_TYPED_TEST_CASE_P(MathFunctions_TestCase_9, nan);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, MathFunctions_TestCase_9, OCLDevicesTypes_9);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class MathFunctions_TestCase_10 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                               typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_SCALAR_TYPES_10(D)                         \
  DeviceTypePair<D, std::tuple<cl_int, cl_float>>,      \
  DeviceTypePair<D, std::tuple<cl_int, cl_double>>

#define LAST_VECTOR_TYPES_10(D, S)                              \
  DeviceTypePair<D, std::tuple<cl_int ## S, cl_float ## S>>,    \
  DeviceTypePair<D, std::tuple<cl_int ## S, cl_double ## S>>
#else
#define LAST_SCALAR_TYPES_10(D) \
  DeviceTypePair<D, std::tuple<cl_int, cl_float>>
  
#define LAST_VECTOR_TYPES_10(D, S) \
  DeviceTypePair<D, std::tuple<cl_int ## S, cl_float ## S>>
#endif

#define LAST_ALL_TYPES_10(D)    \
  LAST_SCALAR_TYPES_10(D),      \
  LAST_VECTOR_TYPES_10(D, 2),   \
  LAST_VECTOR_TYPES_10(D, 3),   \
  LAST_VECTOR_TYPES_10(D, 4),   \
  LAST_VECTOR_TYPES_10(D, 8),   \
  LAST_VECTOR_TYPES_10(D, 16)

#define ALL_DEVICE_TYPES_10 \
  LAST_ALL_TYPES_10(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_10> OCLDevicesTypes_10;

TYPED_TEST_CASE_P(MathFunctions_TestCase_10);

/*  Function    |   Type Patterns (See OpenCL.td)
 * -----------------------------------------------------
 *  ilogb       |   Snr 
 */

TYPED_TEST_P(MathFunctions_TestCase_10, ilogb) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input = GENTYPE_CHECK_TUPLE_ELEMENT_DOUBLE(1) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_DBL_MIN) :
    GENTYPE_CREATE_TUPLE_ELEMENT(1, CL_FLT_MIN);
  Expected = GENTYPE_CHECK_TUPLE_ELEMENT_DOUBLE(1) ?
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -1022) :
    GENTYPE_CREATE_TUPLE_ELEMENT(0, -126);
  this->Invoke("ilogb", Output, Input);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(MathFunctions_TestCase_10, ilogb);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, MathFunctions_TestCase_10, OCLDevicesTypes_10);
