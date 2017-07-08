
#include "LibraryFixture.h"

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class IntegerFunctions_TestCase_1 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                 typename TyTy::Type> { };

#define LAST_SCALAR_TYPES_1(D)                                      \
  DeviceTypePair<D, std::tuple<cl_uchar, cl_char, cl_char>>,        \
  DeviceTypePair<D, std::tuple<cl_uchar, cl_uchar, cl_uchar>>,      \
  DeviceTypePair<D, std::tuple<cl_ushort, cl_short, cl_short>>,     \
  DeviceTypePair<D, std::tuple<cl_ushort, cl_ushort, cl_ushort>>,   \
  DeviceTypePair<D, std::tuple<cl_uint, cl_int, cl_int>>,           \
  DeviceTypePair<D, std::tuple<cl_uint, cl_uint, cl_uint>>,         \
  DeviceTypePair<D, std::tuple<cl_ulong, cl_long, cl_long>>,        \
  DeviceTypePair<D, std::tuple<cl_ulong, cl_ulong, cl_ulong>>

#define LAST_VECTOR_TYPES_1(D, S) \
  DeviceTypePair<D, std::tuple<cl_uchar ## S, cl_char ## S, cl_char ## S>>,        \
  DeviceTypePair<D, std::tuple<cl_uchar ## S, cl_uchar ## S, cl_uchar ## S>>,      \
  DeviceTypePair<D, std::tuple<cl_ushort ## S, cl_short ## S, cl_short ## S>>,     \
  DeviceTypePair<D, std::tuple<cl_ushort ## S, cl_ushort ## S, cl_ushort ## S>>,   \
  DeviceTypePair<D, std::tuple<cl_uint ## S, cl_int ## S, cl_int ## S>>,           \
  DeviceTypePair<D, std::tuple<cl_uint ## S, cl_uint ## S, cl_uint ## S>>,         \
  DeviceTypePair<D, std::tuple<cl_ulong ## S, cl_long ## S, cl_long ## S>>,        \
  DeviceTypePair<D, std::tuple<cl_ulong ## S, cl_ulong ## S, cl_ulong ## S>>

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

TYPED_TEST_CASE_P(IntegerFunctions_TestCase_1);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  abs         |   UiUi, UiSi
 *  abs_diff    |   Uiii
 */

TYPED_TEST_P(IntegerFunctions_TestCase_1, abs) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("abs", Output, Input_x);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  if (GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED(1)) {
    Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, -1);
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
    this->Invoke("abs", Output, Input_x);
    ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  }
}

TYPED_TEST_P(IntegerFunctions_TestCase_1, abs_diff) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 2);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("abs_diff", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 2);
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("abs_diff", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  if (GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED(1)) {
    Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, -2);
    Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, -1);
    Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
    this->Invoke("abs_diff", Output, Input_x, Input_y);
    ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  }
}

REGISTER_TYPED_TEST_CASE_P(IntegerFunctions_TestCase_1, abs,
                                                        abs_diff);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, IntegerFunctions_TestCase_1, OCLDevicesTypes_1);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class IntegerFunctions_TestCase_2 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                 typename TyTy::Type> { };

#define LAST_SCALAR_TYPES_2(D)  \
  DeviceTypePair<D, cl_char>,   \
  DeviceTypePair<D, cl_uchar>,  \
  DeviceTypePair<D, cl_short>,  \
  DeviceTypePair<D, cl_ushort>, \
  DeviceTypePair<D, cl_int>,    \
  DeviceTypePair<D, cl_uint>,   \
  DeviceTypePair<D, cl_long>,   \
  DeviceTypePair<D, cl_ulong>

#define LAST_VECTOR_TYPES_2(D, S)       \
  DeviceTypePair<D, cl_char ## S>,      \
  DeviceTypePair<D, cl_uchar ## S>,     \
  DeviceTypePair<D, cl_short ## S>,     \
  DeviceTypePair<D, cl_ushort ## S>,    \
  DeviceTypePair<D, cl_int ## S>,       \
  DeviceTypePair<D, cl_uint ## S>,      \
  DeviceTypePair<D, cl_long ## S>,      \
  DeviceTypePair<D, cl_ulong ## S>

#define LAST_ALL_TYPES_2(D)  \
  LAST_SCALAR_TYPES_2(D),    \
  LAST_VECTOR_TYPES_2(D, 2), \
  LAST_VECTOR_TYPES_2(D, 3), \
  LAST_VECTOR_TYPES_2(D, 4), \
  LAST_VECTOR_TYPES_2(D, 8), \
  LAST_VECTOR_TYPES_2(D, 16)

#define ALL_DEVICE_TYPES_2 \
  LAST_ALL_TYPES_2(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_2> OCLDevicesTypes_2;

TYPED_TEST_CASE_P(IntegerFunctions_TestCase_2);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  add_sat     |   UiUiUi, SiSiSi
 *  sub_sat     |   UiUiUi, SiSiSi
 *  mad_sat     |   UiUiUiUi, SiSiSiSi
 *  mul24       |   iii
 *  mad24       |   iiii
 *  mul_hi      |   iii
 *  mad_hi      |   iiii
 *  max         |   iii
 *  min         |   iii
 *  clamp       |   iiii
 *  clz         |   ii
 *  hadd        |   iii
 *  rhadd       |   iii
 *  popcount    |   ii
 *  rotate      |   ii
 */

TYPED_TEST_P(IntegerFunctions_TestCase_2, add_sat) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Input_y);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  if (GENTYPE_CHECK_SIGNED) {
    uint64_t MinVal, MaxVal;
    if (GENTYPE_CHECK_BASE(cl_char)) {
      MaxVal = CL_CHAR_MAX; // 0x7FLL (= +127)
      MinVal = CL_CHAR_MIN; // 0x80LL (= -128)
    } else if (GENTYPE_CHECK_BASE(cl_short)) {
      MaxVal = CL_SHRT_MAX; // 0x7FFFLL (= +32767)
      MinVal = CL_SHRT_MIN; // 0x8000LL (= -32768)
    } else if (GENTYPE_CHECK_BASE(cl_int)) {
      MaxVal = CL_INT_MAX; // 0x7FFFFFFFLL (= +2147483647)
      MinVal = CL_INT_MIN; // 0x80000000LL (= -2147483648)
    } else if (GENTYPE_CHECK_BASE(cl_long)) {
      MaxVal = CL_LONG_MAX; // 0x7FFFFFFFFFFFFFFFLL (= +9223372036854775807)
      MinVal = CL_LONG_MIN; // 0x8000000000000000LL (= -9223372036854775808)
    }

    // Saturation
    Input_x = GENTYPE_CREATE(MinVal);
    Input_y = GENTYPE_CREATE(-1L);
    Expected = GENTYPE_CREATE(MinVal);
    this->Invoke("add_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);

    // Saturation
    Input_x = GENTYPE_CREATE(MaxVal);
    Input_y = GENTYPE_CREATE(1);
    Expected = GENTYPE_CREATE(MaxVal);
    this->Invoke("add_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);

    // No Saturation
    Input_x = GENTYPE_CREATE(MinVal + 1);
    Input_y = GENTYPE_CREATE(-1L);
    Expected = GENTYPE_CREATE(MinVal);
    this->Invoke("add_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);

    // No Saturation
    Input_x = GENTYPE_CREATE(MaxVal - 1);
    Input_y = GENTYPE_CREATE(1);
    Expected = GENTYPE_CREATE(MaxVal);
    this->Invoke("add_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);
  } else if (GENTYPE_CHECK_UNSIGNED) { 
    // All bits set to 1.
    uint64_t MaxVal = ~0UL;

    // Saturation
    Input_x = GENTYPE_CREATE(MaxVal);
    Input_y = GENTYPE_CREATE(1);
    Expected = GENTYPE_CREATE(MaxVal);
    this->Invoke("add_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);

    // No Saturation
    Input_x = GENTYPE_CREATE(MaxVal - 1);
    Input_y = GENTYPE_CREATE(1);
    Expected = GENTYPE_CREATE(MaxVal);
    this->Invoke("add_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);
  }
}

TYPED_TEST_P(IntegerFunctions_TestCase_2, sub_sat) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Input_y);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  if (GENTYPE_CHECK_SIGNED) {
    uint64_t MinVal, MaxVal;
    if (GENTYPE_CHECK_BASE(cl_char)) {
      MaxVal = CL_CHAR_MAX; // 0x7FLL (= +127)
      MinVal = CL_CHAR_MIN; // 0x80LL (= -128)
    } else if (GENTYPE_CHECK_BASE(cl_short)) {
      MaxVal = CL_SHRT_MAX; // 0x7FFFLL (= +32767)
      MinVal = CL_SHRT_MIN; // 0x8000LL (= -32768)
    } else if (GENTYPE_CHECK_BASE(cl_int)) {
      MaxVal = CL_INT_MAX; // 0x7FFFFFFFLL (= +2147483647)
      MinVal = CL_INT_MIN; // 0x80000000LL (= -2147483648)
    } else if (GENTYPE_CHECK_BASE(cl_long)) {
      MaxVal = CL_LONG_MAX; // 0x7FFFFFFFFFFFFFFFLL (= +9223372036854775807)
      MinVal = CL_LONG_MIN; // 0x8000000000000000LL (= -9223372036854775808)
    }

    // Saturation
    Input_x = GENTYPE_CREATE(MinVal);
    Input_y = GENTYPE_CREATE(1);
    Expected = GENTYPE_CREATE(MinVal);
    this->Invoke("sub_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);

    // Saturation
    Input_x = GENTYPE_CREATE(MaxVal);
    Input_y = GENTYPE_CREATE(-1L);
    Expected = GENTYPE_CREATE(MaxVal);
    this->Invoke("sub_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);

    // No Saturation
    Input_x = GENTYPE_CREATE(MinVal + 1);
    Input_y = GENTYPE_CREATE(1L);
    Expected = GENTYPE_CREATE(MinVal);
    this->Invoke("sub_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);

    // No Saturation
    Input_x = GENTYPE_CREATE(MaxVal - 1);
    Input_y = GENTYPE_CREATE(-1L);
    Expected = GENTYPE_CREATE(MaxVal);
    this->Invoke("sub_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);
  } else if (GENTYPE_CHECK_UNSIGNED) { 
    // All bits set to 1.
    uint64_t MaxVal = ~0UL;

    // Saturation
    Input_x = GENTYPE_CREATE(1);
    Input_y = GENTYPE_CREATE(MaxVal);
    Expected = GENTYPE_CREATE(0);
    this->Invoke("sub_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);

    // No Saturation
    Input_x = GENTYPE_CREATE(MaxVal);
    Input_y = GENTYPE_CREATE(1);
    Expected = GENTYPE_CREATE(MaxVal - 1);
    this->Invoke("sub_sat", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);
  }
}

TYPED_TEST_P(IntegerFunctions_TestCase_2, mad_sat) {
  GENTYPE_DECLARE(Input_a);
  GENTYPE_DECLARE(Input_b);
  GENTYPE_DECLARE(Input_c);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  if (GENTYPE_CHECK_SIGNED) {
    uint64_t MinVal, MaxVal;
    if (GENTYPE_CHECK_BASE(cl_char)) {
      MaxVal = CL_CHAR_MAX; // 0x7FLL (= +127)
      MinVal = CL_CHAR_MIN; // 0x80LL (= -128)
    } else if (GENTYPE_CHECK_BASE(cl_short)) {
      MaxVal = CL_SHRT_MAX; // 0x7FFFLL (= +32767)
      MinVal = CL_SHRT_MIN; // 0x8000LL (= -32768)
    } else if (GENTYPE_CHECK_BASE(cl_int)) {
      MaxVal = CL_INT_MAX; // 0x7FFFFFFFLL (= +2147483647)
      MinVal = CL_INT_MIN; // 0x80000000LL (= -2147483648)
    } else if (GENTYPE_CHECK_BASE(cl_long)) {
      MaxVal = CL_LONG_MAX; // 0x7FFFFFFFFFFFFFFFLL (= +9223372036854775807)
      MinVal = CL_LONG_MIN; // 0x8000000000000000LL (= -9223372036854775808)
    }

    Input_a = GENTYPE_CREATE(0);
    Input_b = GENTYPE_CREATE(MinVal);
    Input_c = GENTYPE_CREATE(MinVal);
    Expected = GENTYPE_CREATE(MinVal);
    this->Invoke("mad_sat", Output, Input_a, Input_b, Input_c);
    ASSERT_GENTYPE_EQ(Expected, Output);
    
    Input_a = GENTYPE_CREATE(MinVal);
    Input_b = GENTYPE_CREATE(0);
    Input_c = GENTYPE_CREATE(MinVal);
    Expected = GENTYPE_CREATE(MinVal);
    this->Invoke("mad_sat", Output, Input_a, Input_b, Input_c);
    ASSERT_GENTYPE_EQ(Expected, Output);
 
    Input_a = GENTYPE_CREATE(1);
    Input_b = GENTYPE_CREATE(1);
    Input_c = GENTYPE_CREATE(1);
    Expected = GENTYPE_CREATE(2);
    this->Invoke("mad_sat", Output, Input_a, Input_b, Input_c);
    ASSERT_GENTYPE_EQ(Expected, Output);
   
    // Saturation
    Input_a = GENTYPE_CREATE(1);
    Input_b = GENTYPE_CREATE(1);
    Input_c = GENTYPE_CREATE(MaxVal);
    Expected = GENTYPE_CREATE(MaxVal);
    this->Invoke("mad_sat", Output, Input_a, Input_b, Input_c);
    ASSERT_GENTYPE_EQ(Expected, Output);

    // Saturation
    Input_a = GENTYPE_CREATE(-1L);
    Input_b = GENTYPE_CREATE(1);
    Input_c = GENTYPE_CREATE(MinVal);
    Expected = GENTYPE_CREATE(MinVal);
    this->Invoke("mad_sat", Output, Input_a, Input_b, Input_c);
    ASSERT_GENTYPE_EQ(Expected, Output);
  } else if (GENTYPE_CHECK_UNSIGNED) { 
    // All bits set to 1.
    uint64_t MaxVal = ~0UL;

    Input_a = GENTYPE_CREATE(0);
    Input_b = GENTYPE_CREATE(MaxVal);
    Input_c = GENTYPE_CREATE(MaxVal);
    Expected = GENTYPE_CREATE(MaxVal);
    this->Invoke("mad_sat", Output, Input_a, Input_b, Input_c);
    ASSERT_GENTYPE_EQ(Expected, Output);
    
    Input_a = GENTYPE_CREATE(MaxVal);
    Input_b = GENTYPE_CREATE(0);
    Input_c = GENTYPE_CREATE(MaxVal);
    Expected = GENTYPE_CREATE(MaxVal);
    this->Invoke("mad_sat", Output, Input_a, Input_b, Input_c);
    ASSERT_GENTYPE_EQ(Expected, Output);

    Input_a = GENTYPE_CREATE(1);
    Input_b = GENTYPE_CREATE(1);
    Input_c = GENTYPE_CREATE(1);
    Expected = GENTYPE_CREATE(2);
    this->Invoke("mad_sat", Output, Input_a, Input_b, Input_c);
    ASSERT_GENTYPE_EQ(Expected, Output);

    // Saturation
    Input_a = GENTYPE_CREATE(MaxVal);
    Input_b = GENTYPE_CREATE(1);
    Input_c = GENTYPE_CREATE(1);
    Expected = GENTYPE_CREATE(MaxVal);
    this->Invoke("mad_sat", Output, Input_a, Input_b, Input_c);
    ASSERT_GENTYPE_EQ(Expected, Output);
  }
}

TYPED_TEST_P(IntegerFunctions_TestCase_2, mul_hi) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Input_y);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  if (GENTYPE_CHECK_SIGNED) {
    if (GENTYPE_CHECK_BASE(cl_char)) {
      Input_x = GENTYPE_CREATE(CL_CHAR_MIN);
      Input_y = GENTYPE_CREATE(2);
      Expected = GENTYPE_CREATE(0xFF);
    } else if (GENTYPE_CHECK_BASE(cl_short)) {
      Input_x = GENTYPE_CREATE(CL_SHRT_MIN);
      Input_y = GENTYPE_CREATE(2);
      Expected = GENTYPE_CREATE(0xFFFF);
    } else if (GENTYPE_CHECK_BASE(cl_int)) {
      Input_x = GENTYPE_CREATE(CL_INT_MIN);
      Input_y = GENTYPE_CREATE(2);
      Expected = GENTYPE_CREATE(0xFFFFFFFF);
    } else if (GENTYPE_CHECK_BASE(cl_long)) {
      Input_x = GENTYPE_CREATE((int64_t)CL_LONG_MIN);
      Input_y = GENTYPE_CREATE(2);
      Expected = GENTYPE_CREATE(0xFFFFFFFFFFFFFFFF);
    }
  } else if (GENTYPE_CHECK_UNSIGNED) {
    if (GENTYPE_CHECK_BASE(cl_uchar)) {
      Input_x = GENTYPE_CREATE(0x1FU);
      Input_y = GENTYPE_CREATE(0x2FU);
      Expected = GENTYPE_CREATE(0x5U);
    } else if (GENTYPE_CHECK_BASE(cl_ushort)) {
      Input_x = GENTYPE_CREATE(0x1FFFU);
      Input_y = GENTYPE_CREATE(0x2FFFU);
      Expected = GENTYPE_CREATE(0x5FFU);
    } else if (GENTYPE_CHECK_BASE(cl_uint)) {
      Input_x = GENTYPE_CREATE(0x1FFFFFFFU);
      Input_y = GENTYPE_CREATE(0x2FFFFFFFU);
      Expected = GENTYPE_CREATE(0x5FFFFFFU);
    } else if (GENTYPE_CHECK_BASE(cl_ulong)) {
      Input_x = GENTYPE_CREATE(0x1FFFFFFFFFFFFFFFUL);
      Input_y = GENTYPE_CREATE(0x2FFFFFFFFFFFFFFFUL);
      Expected = GENTYPE_CREATE(0x5FFFFFFFFFFFFFFUL);
    }
  }
  this->Invoke("mul_hi", Output, Input_x, Input_y);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(IntegerFunctions_TestCase_2, mad_hi) {
  GENTYPE_DECLARE(Input_a);
  GENTYPE_DECLARE(Input_b);
  GENTYPE_DECLARE(Input_c);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  if (GENTYPE_CHECK_SIGNED) {
    if (GENTYPE_CHECK_BASE(cl_char)) {
      Input_a = GENTYPE_CREATE(CL_CHAR_MIN);
      Input_b = GENTYPE_CREATE(2);
      Input_c = GENTYPE_CREATE(1);
      Expected = GENTYPE_CREATE(0);
    } else if (GENTYPE_CHECK_BASE(cl_short)) {
      Input_a = GENTYPE_CREATE(CL_SHRT_MIN);
      Input_b = GENTYPE_CREATE(2);
      Input_c = GENTYPE_CREATE(1);
      Expected = GENTYPE_CREATE(0);
    } else if (GENTYPE_CHECK_BASE(cl_int)) {
      Input_a = GENTYPE_CREATE(CL_INT_MIN);
      Input_b = GENTYPE_CREATE(2);
      Input_c = GENTYPE_CREATE(1);
      Expected = GENTYPE_CREATE(0);
    } else if (GENTYPE_CHECK_BASE(cl_long)) {
      Input_a = GENTYPE_CREATE((int64_t)CL_LONG_MIN);
      Input_b = GENTYPE_CREATE(2);
      Input_c = GENTYPE_CREATE(1);
      Expected = GENTYPE_CREATE(0);
    }
  } else if (GENTYPE_CHECK_UNSIGNED) {
    if (GENTYPE_CHECK_BASE(cl_uchar)) {
      Input_a = GENTYPE_CREATE(0x1FU);
      Input_b = GENTYPE_CREATE(0x2FU);
      Input_c = GENTYPE_CREATE(1);
      Expected = GENTYPE_CREATE(0x6U);
    } else if (GENTYPE_CHECK_BASE(cl_ushort)) {
      Input_a = GENTYPE_CREATE(0x1FFFU);
      Input_b = GENTYPE_CREATE(0x2FFFU);
      Input_c = GENTYPE_CREATE(1);
      Expected = GENTYPE_CREATE(0x600U);
    } else if (GENTYPE_CHECK_BASE(cl_uint)) {
      Input_a = GENTYPE_CREATE(0x1FFFFFFFU);
      Input_b = GENTYPE_CREATE(0x2FFFFFFFU);
      Input_c = GENTYPE_CREATE(1);
      Expected = GENTYPE_CREATE(0x6000000U);
    } else if (GENTYPE_CHECK_BASE(cl_ulong)) {
      Input_a = GENTYPE_CREATE(0x1FFFFFFFFFFFFFFFUL);
      Input_b = GENTYPE_CREATE(0x2FFFFFFFFFFFFFFFUL);
      Input_c = GENTYPE_CREATE(1);
      Expected = GENTYPE_CREATE(0x600000000000000UL);
    }
  }

  this->Invoke("mad_hi", Output, Input_a, Input_b, Input_c);
  ASSERT_GENTYPE_EQ(Expected, Output);
}


TYPED_TEST_P(IntegerFunctions_TestCase_2, hadd) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Input_y);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  if (GENTYPE_CHECK_SIGNED) {
    if (GENTYPE_CHECK_BASE(cl_char)) {
      Input_x = GENTYPE_CREATE(0x01);
      Input_y = GENTYPE_CREATE(0xF0);
      Expected = GENTYPE_CREATE(0xF8);
    } else if (GENTYPE_CHECK_BASE(cl_short)) {
      Input_x = GENTYPE_CREATE(0x0001);
      Input_y = GENTYPE_CREATE(0xFF00);
      Expected = GENTYPE_CREATE(0xFF80);
    } else if (GENTYPE_CHECK_BASE(cl_int)) {
      Input_x = GENTYPE_CREATE(0x00000001);
      Input_y = GENTYPE_CREATE(0xFFFF0000);
      Expected = GENTYPE_CREATE(0xFFFF8000);
    } else if (GENTYPE_CHECK_BASE(cl_long)) {
      Input_x = GENTYPE_CREATE(0x0000000000000001);
      Input_y = GENTYPE_CREATE(0xFFFFFFFF00000000);
      Expected = GENTYPE_CREATE(0xFFFFFFFF80000000);
    }
  } else if(GENTYPE_CHECK_UNSIGNED) {
    if (GENTYPE_CHECK_BASE(cl_uchar)) {
      Input_x = GENTYPE_CREATE(0x01U);
      Input_y = GENTYPE_CREATE(0xF0U);
      Expected = GENTYPE_CREATE(0x78U);
    } else if (GENTYPE_CHECK_BASE(cl_ushort)) {
      Input_x = GENTYPE_CREATE(0x0001U);
      Input_y = GENTYPE_CREATE(0xFF00U);
      Expected = GENTYPE_CREATE(0x7F80U);
    } else if (GENTYPE_CHECK_BASE(cl_uint)) {
      Input_x = GENTYPE_CREATE(0x00000001U);
      Input_y = GENTYPE_CREATE(0xFFFF0000U);
      Expected = GENTYPE_CREATE(0x7FFF8000U);
    } else if (GENTYPE_CHECK_BASE(cl_ulong)) {
      Input_x = GENTYPE_CREATE(0x0000000000000001U);
      Input_y = GENTYPE_CREATE(0xFFFFFFFF00000000U);
      Expected = GENTYPE_CREATE(0x7FFFFFFF80000000U);
    }
  }

  this->Invoke("hadd", Output, Input_x, Input_y);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(IntegerFunctions_TestCase_2, rhadd) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Input_y);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  if (GENTYPE_CHECK_SIGNED) {
    if (GENTYPE_CHECK_BASE(cl_char)) {
      Input_x = GENTYPE_CREATE(0x02);
      Input_y = GENTYPE_CREATE(0xF0);
      Expected = GENTYPE_CREATE(0xF9);
    } else if (GENTYPE_CHECK_BASE(cl_short)) {
      Input_x = GENTYPE_CREATE(0x0002);
      Input_y = GENTYPE_CREATE(0xFF00);
      Expected = GENTYPE_CREATE(0xFF81);
    } else if (GENTYPE_CHECK_BASE(cl_int)) {
      Input_x = GENTYPE_CREATE(0x00000002);
      Input_y = GENTYPE_CREATE(0xFFFF0000);
      Expected = GENTYPE_CREATE(0xFFFF8001);
    } else if (GENTYPE_CHECK_BASE(cl_long)) {
      Input_x = GENTYPE_CREATE(0x0000000000000002);
      Input_y = GENTYPE_CREATE(0xFFFFFFFF00000000);
      Expected = GENTYPE_CREATE(0xFFFFFFFF80000001);
    }
  } else if(GENTYPE_CHECK_UNSIGNED) {
    if (GENTYPE_CHECK_BASE(cl_uchar)) {
      Input_x = GENTYPE_CREATE(0x02U);
      Input_y = GENTYPE_CREATE(0xF0U);
      Expected = GENTYPE_CREATE(0x79U);
    } else if (GENTYPE_CHECK_BASE(cl_ushort)) {
      Input_x = GENTYPE_CREATE(0x0002U);
      Input_y = GENTYPE_CREATE(0xFF00U);
      Expected = GENTYPE_CREATE(0x7F81U);
    } else if (GENTYPE_CHECK_BASE(cl_uint)) {
      Input_x = GENTYPE_CREATE(0x00000002U);
      Input_y = GENTYPE_CREATE(0xFFFF0000U);
      Expected = GENTYPE_CREATE(0x7FFF8001U);
    } else if (GENTYPE_CHECK_BASE(cl_ulong)) {
      Input_x = GENTYPE_CREATE(0x0000000000000002U);
      Input_y = GENTYPE_CREATE(0xFFFFFFFF00000000U);
      Expected = GENTYPE_CREATE(0x7FFFFFFF80000001U);
    }
  }

  this->Invoke("rhadd", Output, Input_x, Input_y);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(IntegerFunctions_TestCase_2, max) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Input_y);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_x = GENTYPE_CREATE(0);
  Input_y = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("max", Output, Input_x, Input_y);
  ASSERT_GENTYPE_EQ(Expected, Output);

  if (GENTYPE_CHECK_SIGNED) {
    Input_x = GENTYPE_CREATE(0);
    Input_y = GENTYPE_CREATE(-1);
    Expected = GENTYPE_CREATE(0);
    this->Invoke("max", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);
  }
}

TYPED_TEST_P(IntegerFunctions_TestCase_2, min) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Input_y);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_x = GENTYPE_CREATE(0);
  Input_y = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(0);
  this->Invoke("min", Output, Input_x, Input_y);
  ASSERT_GENTYPE_EQ(Expected, Output);

  if (GENTYPE_CHECK_SIGNED) {
    Input_x = GENTYPE_CREATE(0);
    Input_y = GENTYPE_CREATE(-1);
    Expected = GENTYPE_CREATE(-1);
    this->Invoke("min", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);
  }
}

TYPED_TEST_P(IntegerFunctions_TestCase_2, clamp) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Input_minval);
  GENTYPE_DECLARE(Input_maxval);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_x = GENTYPE_CREATE(0);
  Input_minval = GENTYPE_CREATE(1);
  Input_maxval = GENTYPE_CREATE(2);
  Expected = GENTYPE_CREATE(1);
  this->Invoke("clamp", Output, Input_x, Input_minval, Input_maxval);
  ASSERT_GENTYPE_EQ(Expected, Output);
  
  if (GENTYPE_CHECK_SIGNED) {
    Input_x = GENTYPE_CREATE(0);
    Input_minval = GENTYPE_CREATE(-2);
    Input_maxval = GENTYPE_CREATE(-1);
    Expected = GENTYPE_CREATE(-1);
    this->Invoke("clamp", Output, Input_x, Input_minval, Input_maxval);
    ASSERT_GENTYPE_EQ(Expected, Output);

    Input_x = GENTYPE_CREATE(-2);
    Input_minval = GENTYPE_CREATE(-1);
    Input_maxval = GENTYPE_CREATE(0);
    Expected = GENTYPE_CREATE(-1);
    this->Invoke("clamp", Output, Input_x, Input_minval, Input_maxval);
    ASSERT_GENTYPE_EQ(Expected, Output);
  }
}

TYPED_TEST_P(IntegerFunctions_TestCase_2, clz) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  if (GENTYPE_CHECK_SIGNED) {
    Input_x = GENTYPE_CREATE(1L);
    if (GENTYPE_CHECK_BASE(cl_char))
      Expected = GENTYPE_CREATE(7);
    else if (GENTYPE_CHECK_BASE(cl_short)) 
      Expected = GENTYPE_CREATE(15);
    else if (GENTYPE_CHECK_BASE(cl_int)) 
      Expected = GENTYPE_CREATE(31);
    else if (GENTYPE_CHECK_BASE(cl_long)) 
      Expected = GENTYPE_CREATE(63);
  } else if (GENTYPE_CHECK_UNSIGNED) {
    Input_x = GENTYPE_CREATE(-1L);
    Expected = GENTYPE_CREATE(0);
  }
  this->Invoke("clz", Output, Input_x);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(IntegerFunctions_TestCase_2, popcount) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  if (GENTYPE_CHECK_BASE(cl_char) || GENTYPE_CHECK_BASE(cl_uchar)) {
    Input_x = GENTYPE_CREATE((uint8_t)0x55UL);
    Expected = GENTYPE_CREATE(4);
  } else if (GENTYPE_CHECK_BASE(cl_short) || GENTYPE_CHECK_BASE(cl_ushort)) {
    Input_x = GENTYPE_CREATE((uint16_t)0x5555UL);
    Expected = GENTYPE_CREATE(8);
  } else if (GENTYPE_CHECK_BASE(cl_int) || GENTYPE_CHECK_BASE(cl_uint)) {
    Input_x = GENTYPE_CREATE((uint32_t)0x55555555UL);
    Expected = GENTYPE_CREATE(16);
  } else if (GENTYPE_CHECK_BASE(cl_long) || GENTYPE_CHECK_BASE(cl_ulong)) {
    Input_x = GENTYPE_CREATE((uint64_t)0x5555555555555555UL);
    Expected = GENTYPE_CREATE(32);
  }
  this->Invoke("popcount", Output, Input_x);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

TYPED_TEST_P(IntegerFunctions_TestCase_2, rotate) {
  GENTYPE_DECLARE(Input_v);
  GENTYPE_DECLARE(Input_i);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  if (GENTYPE_CHECK_UNSIGNED) {
    if (GENTYPE_CHECK_BASE(cl_uchar)) {
      Input_v = GENTYPE_CREATE(0x11U);
      Input_i = GENTYPE_CREATE(6);
      Expected = GENTYPE_CREATE(0x44);
    } else if (GENTYPE_CHECK_BASE(cl_ushort)) {
      Input_v = GENTYPE_CREATE(0x1111U);
      Input_i = GENTYPE_CREATE(6);
      Expected = GENTYPE_CREATE(0x4444U);
    } else if (GENTYPE_CHECK_BASE(cl_uint)) {
      Input_v = GENTYPE_CREATE(0x11111111U);
      Input_i = GENTYPE_CREATE(6);
      Expected = GENTYPE_CREATE(0x44444444U);
    } else if (GENTYPE_CHECK_BASE(cl_ulong)) {
      Input_v = GENTYPE_CREATE(0x1111111111111111UL);
      Input_i = GENTYPE_CREATE(6);
      Expected = GENTYPE_CREATE(0x4444444444444444UL);
    }
  } else if (GENTYPE_CHECK_SIGNED) {
    if (GENTYPE_CHECK_BASE(cl_char)) {
      Input_v = GENTYPE_CREATE(0xEFU);
      Input_i = GENTYPE_CREATE(6);
      Expected = GENTYPE_CREATE(0xFBU);
    } else if (GENTYPE_CHECK_BASE(cl_short)) {
      Input_v = GENTYPE_CREATE(0xEEEFU);
      Input_i = GENTYPE_CREATE(6);
      Expected = GENTYPE_CREATE(0xBBFBU);
    } else if (GENTYPE_CHECK_BASE(cl_int)) {
      Input_v = GENTYPE_CREATE(0xEEEEEEEFU);
      Input_i = GENTYPE_CREATE(6);
      Expected = GENTYPE_CREATE(0xBBBBBBFBU);
    } else if (GENTYPE_CHECK_BASE(cl_long)) {
      Input_v = GENTYPE_CREATE(0xEEEEEEEEEEEEEEEFU);
      Input_i = GENTYPE_CREATE(6);
      Expected = GENTYPE_CREATE(0xBBBBBBBBBBBBBBFBU);
    }
  }

  this->Invoke("rotate", Output, Input_v, Input_i);
  ASSERT_GENTYPE_EQ(Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(IntegerFunctions_TestCase_2, add_sat,
                                                        sub_sat,
                                                        mad_sat,
                                                        mul_hi,
                                                        mad_hi,
                                                        hadd,
                                                        rhadd,
                                                        max,
                                                        min,
                                                        clamp,
                                                        clz,
                                                        popcount,
                                                        rotate);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, IntegerFunctions_TestCase_2, OCLDevicesTypes_2);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class IntegerFunctions_TestCase_3 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                 typename TyTy::Type> { };

#define LAST_SCALAR_TYPES_3(D)  \
  DeviceTypePair<D, cl_int>,    \
  DeviceTypePair<D, cl_uint>

#define LAST_VECTOR_TYPES_3(D, S)   \
  DeviceTypePair<D, cl_int ## S>,   \
  DeviceTypePair<D, cl_uint ## S>

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

TYPED_TEST_CASE_P(IntegerFunctions_TestCase_3);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  mul24       |   iii (*)
 *  mad24       |   iiii (*)
 *
 *  (*) Fast 24-bit arithmetic operations take a subset of integer scalar
 *      and vector types (see the ALL_DEVICE_TYPES_3 macro).
 */

TYPED_TEST_P(IntegerFunctions_TestCase_3, mul24) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Input_y);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_x = GENTYPE_CREATE(20);
  Input_y = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(20);
  this->Invoke("mul24", Output, Input_x, Input_y);
  ASSERT_GENTYPE_EQ(Expected, Output);

  if (GENTYPE_CHECK_SIGNED) {
    Input_x = GENTYPE_CREATE(20);
    Input_y = GENTYPE_CREATE(-1);
    Expected = GENTYPE_CREATE(-20);
    this->Invoke("mul24", Output, Input_x, Input_y);
    ASSERT_GENTYPE_EQ(Expected, Output);
  }
}

TYPED_TEST_P(IntegerFunctions_TestCase_3, mad24) {
  GENTYPE_DECLARE(Input_x);
  GENTYPE_DECLARE(Input_y);
  GENTYPE_DECLARE(Input_z);
  GENTYPE_DECLARE(Expected);
  GENTYPE_DECLARE(Output);

  Input_x = GENTYPE_CREATE(20);
  Input_y = GENTYPE_CREATE(1);
  Input_z = GENTYPE_CREATE(1);
  Expected = GENTYPE_CREATE(21);
  this->Invoke("mad24", Output, Input_x, Input_y, Input_z);
  ASSERT_GENTYPE_EQ(Expected, Output);

  if (GENTYPE_CHECK_SIGNED) {
    Input_x = GENTYPE_CREATE(20);
    Input_y = GENTYPE_CREATE(-1);
    Input_z = GENTYPE_CREATE(-1);
    Expected = GENTYPE_CREATE(-21);
    this->Invoke("mad24", Output, Input_x, Input_y, Input_z);
    ASSERT_GENTYPE_EQ(Expected, Output);
  }
}

REGISTER_TYPED_TEST_CASE_P(IntegerFunctions_TestCase_3, mul24,
                                                        mad24);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, IntegerFunctions_TestCase_3, OCLDevicesTypes_3);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class IntegerFunctions_TestCase_4 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                 typename TyTy::Type> { };

#define LAST_VECTOR_TYPES_4(D, S)                                                       \
  DeviceTypePair<D, std::tuple<cl_char ## S, cl_char ## S, cl_char, cl_char>>,          \
  DeviceTypePair<D, std::tuple<cl_uchar ## S, cl_uchar ## S, cl_uchar, cl_uchar>>,      \
  DeviceTypePair<D, std::tuple<cl_short ## S, cl_short ## S, cl_short, cl_short>>,      \
  DeviceTypePair<D, std::tuple<cl_ushort ## S, cl_ushort ## S, cl_ushort, cl_ushort>>,  \
  DeviceTypePair<D, std::tuple<cl_int ## S, cl_int ## S, cl_int, cl_int>>,              \
  DeviceTypePair<D, std::tuple<cl_uint ## S, cl_uint ## S, cl_uint, cl_uint>>,          \
  DeviceTypePair<D, std::tuple<cl_long ## S, cl_long ## S, cl_long, cl_long>>,          \
  DeviceTypePair<D, std::tuple<cl_ulong ## S, cl_ulong ## S, cl_ulong, cl_ulong>>

#define LAST_ALL_TYPES_4(D)  \
  LAST_VECTOR_TYPES_4(D, 2), \
  LAST_VECTOR_TYPES_4(D, 3), \
  LAST_VECTOR_TYPES_4(D, 4), \
  LAST_VECTOR_TYPES_4(D, 8), \
  LAST_VECTOR_TYPES_4(D, 16)

#define ALL_DEVICE_TYPES_4 \
  LAST_ALL_TYPES_4(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_4> OCLDevicesTypes_4;

TYPED_TEST_CASE_P(IntegerFunctions_TestCase_4);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  max         |   ViViCi
 *  min         |   ViViCi
 *  clamp       |   ViViCiCi
 */

TYPED_TEST_P(IntegerFunctions_TestCase_4, max) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("max", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  if (GENTYPE_CHECK_SIGNED) {
    Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
    Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, -1);
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
    this->Invoke("max", Output, Input_x, Input_y);
    ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  }

}

TYPED_TEST_P(IntegerFunctions_TestCase_4, min) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0,Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0);
  this->Invoke("min", Output, Input_x, Input_y);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

  if (GENTYPE_CHECK_SIGNED) {
    Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
    Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, -1);
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
    this->Invoke("min", Output, Input_x, Input_y);
    ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  }
}

TYPED_TEST_P(IntegerFunctions_TestCase_4, clamp) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_minval);
  GENTYPE_DECLARE_TUPLE_ELEMENT(3, Input_maxval);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
  Input_minval = GENTYPE_CREATE_TUPLE_ELEMENT(2, 1);
  Input_maxval = GENTYPE_CREATE_TUPLE_ELEMENT(3, 2);
  Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);
  this->Invoke("clamp", Output, Input_x, Input_minval, Input_maxval);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  
  if (GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED(2)) {
    Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
    Input_minval = GENTYPE_CREATE_TUPLE_ELEMENT(2, -2);
    Input_maxval = GENTYPE_CREATE_TUPLE_ELEMENT(3, -1);
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
    this->Invoke("clamp", Output, Input_x, Input_minval, Input_maxval);
    ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);

    Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, -2);
    Input_minval = GENTYPE_CREATE_TUPLE_ELEMENT(2, -1);
    Input_maxval = GENTYPE_CREATE_TUPLE_ELEMENT(3, 0);
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, -1);
    this->Invoke("clamp", Output, Input_x, Input_minval, Input_maxval);
    ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
  }
}

REGISTER_TYPED_TEST_CASE_P(IntegerFunctions_TestCase_4, max,
                                                        min,
                                                        clamp);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, IntegerFunctions_TestCase_4, OCLDevicesTypes_4);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class IntegerFunctions_TestCase_5 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                 typename TyTy::Type> { };

#define LAST_SCALAR_TYPES_5(D)                                  \
  DeviceTypePair<D, std::tuple<cl_short, cl_char, cl_uchar>>,   \
  DeviceTypePair<D, std::tuple<cl_ushort, cl_uchar, cl_uchar>>, \
  DeviceTypePair<D, std::tuple<cl_int, cl_short, cl_ushort>>,   \
  DeviceTypePair<D, std::tuple<cl_uint, cl_ushort, cl_ushort>>, \
  DeviceTypePair<D, std::tuple<cl_long, cl_int, cl_uint>>,      \
  DeviceTypePair<D, std::tuple<cl_ulong, cl_uint, cl_uint>>

#define LAST_VECTOR_TYPES_5(D, S)                                               \
  DeviceTypePair<D, std::tuple<cl_short ## S, cl_char ## S, cl_uchar ## S>>,    \
  DeviceTypePair<D, std::tuple<cl_ushort ## S, cl_uchar ## S, cl_uchar ## S>>,  \
  DeviceTypePair<D, std::tuple<cl_int ## S, cl_short ## S, cl_ushort ## S>>,    \
  DeviceTypePair<D, std::tuple<cl_uint ## S, cl_ushort ## S, cl_ushort ## S>>,  \
  DeviceTypePair<D, std::tuple<cl_long ## S, cl_int ## S, cl_uint ## S>>,      \
  DeviceTypePair<D, std::tuple<cl_ulong ## S, cl_uint ## S, cl_uint ## S>>

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

TYPED_TEST_CASE_P(IntegerFunctions_TestCase_5);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  upsample    |   SsScUc, UsUcUc, SiSsUs, UiUsUs, SlSiUi, UlUiUi
 */

TYPED_TEST_P(IntegerFunctions_TestCase_5, upsample) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_hi);
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_lo);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  if (GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED(0)) {
    if (GENTYPE_CHECK_TUPLE_ELEMENT_BASE(1, cl_char)) {
      Input_hi = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0xFF); // -1
      Input_lo = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0xFE); // -2
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0xFFFE);
    } else if (GENTYPE_CHECK_TUPLE_ELEMENT_BASE(1, cl_short)) {
      Input_hi = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0xFFFF); // -1
      Input_lo = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0x1); // 1
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0xFFFF0001);
    } else if (GENTYPE_CHECK_TUPLE_ELEMENT_BASE(1, cl_int)) {
      Input_hi = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0xFFFFFFFF); // -1
      Input_lo = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0x1); // 1
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0xFFFFFFFF00000001);
    }
  } else if (GENTYPE_CHECK_TUPLE_ELEMENT_UNSIGNED(0)) {
    if (GENTYPE_CHECK_TUPLE_ELEMENT_BASE(1, cl_uchar)) {
      Input_hi = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0xABU);
      Input_lo = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0xCDU);
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0xABCDU);
    } else if (GENTYPE_CHECK_TUPLE_ELEMENT_BASE(1, cl_ushort)) {
      Input_hi = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0x1234U);
      Input_lo = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0x5678U);
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0x12345678U);
    } else if (GENTYPE_CHECK_TUPLE_ELEMENT_BASE(1, cl_uint)) {
      Input_hi = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0x1A2B3C4DU);
      Input_lo = GENTYPE_CREATE_TUPLE_ELEMENT(2, 0x5A6B7C8DU);
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 0x1A2B3C4D5A6B7C8D);
    }
  }

  this->Invoke("upsample", Output, Input_hi, Input_lo);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(IntegerFunctions_TestCase_5, upsample);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, IntegerFunctions_TestCase_5, OCLDevicesTypes_5);

