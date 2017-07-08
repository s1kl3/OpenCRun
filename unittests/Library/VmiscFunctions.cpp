
#include "LibraryFixture.h"

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class VmiscFunctions_TestCase_1_CHAR : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                    typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_1_SHORT : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                     typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_1_INT : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                   typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_1_LONG : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                    typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_1_FLOAT : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                     typename TyTy::Type> { };

#define LAST_VECTOR_TYPES_N_M_1_CHAR(D, N, M)                                   \
  DeviceTypePair<D, std::tuple<cl_char ## N, cl_char ## M, cl_uchar ## N>>,     \
  DeviceTypePair<D, std::tuple<cl_uchar ## N, cl_uchar ## M, cl_uchar ## N>>

#define LAST_VECTOR_TYPES_N_M_1_SHORT(D, N, M)                                      \
  DeviceTypePair<D, std::tuple<cl_short ## N, cl_short ## M, cl_ushort ## N>>,      \
  DeviceTypePair<D, std::tuple<cl_ushort ## N, cl_ushort ## M, cl_ushort ## N>>

#define LAST_VECTOR_TYPES_N_M_1_INT(D, N, M)                                \
  DeviceTypePair<D, std::tuple<cl_int ## N, cl_int ## M, cl_uint ## N>>,    \
  DeviceTypePair<D, std::tuple<cl_uint ## N, cl_uint ## M, cl_uint ## N>>

#define LAST_VECTOR_TYPES_N_M_1_LONG(D, N, M)                                   \
  DeviceTypePair<D, std::tuple<cl_long ## N, cl_long ## M, cl_ulong ## N>>,     \
  DeviceTypePair<D, std::tuple<cl_ulong ## N, cl_ulong ## M, cl_ulong ## N>>

#ifdef cl_khr_fp64 
#define LAST_VECTOR_TYPES_N_M_1_FLOAT(D, N, M)                                      \
  DeviceTypePair<D, std::tuple<cl_float ## N, cl_float ## M, cl_uint ## N>>,        \
  DeviceTypePair<D, std::tuple<cl_double ## N, cl_double ## M, cl_ulong ## N>>
#else
#define LAST_VECTOR_TYPES_N_M_1_FLOAT(D, N, M)                                      \
  DeviceTypePair<D, std::tuple<cl_float ## N, cl_float ## M, cl_uint ## N>>
#endif
  
#define LAST_VECTOR_TYPES_N_M_1(D, T)       \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 2, 2),   \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 2, 4),   \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 2, 8),   \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 2, 16),  \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 4, 2),   \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 4, 4),   \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 4, 8),   \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 4, 16),  \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 8, 2),   \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 8, 4),   \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 8, 8),   \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 8, 16),  \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 16, 2),  \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 16, 4),  \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 16, 8),  \
  LAST_VECTOR_TYPES_N_M_1_ ## T(D, 16, 16)

#define ALL_DEVICE_TYPES_1_CHAR \
  LAST_VECTOR_TYPES_N_M_1(CPUDev, CHAR)

#define ALL_DEVICE_TYPES_1_SHORT \
  LAST_VECTOR_TYPES_N_M_1(CPUDev, SHORT)

#define ALL_DEVICE_TYPES_1_INT \
  LAST_VECTOR_TYPES_N_M_1(CPUDev, INT)

#define ALL_DEVICE_TYPES_1_LONG \
  LAST_VECTOR_TYPES_N_M_1(CPUDev, LONG)
  
#define ALL_DEVICE_TYPES_1_FLOAT \
  LAST_VECTOR_TYPES_N_M_1(CPUDev, FLOAT)

typedef testing::Types<ALL_DEVICE_TYPES_1_CHAR> OCLDevicesTypes_1_CHAR;
typedef testing::Types<ALL_DEVICE_TYPES_1_SHORT> OCLDevicesTypes_1_SHORT;
typedef testing::Types<ALL_DEVICE_TYPES_1_INT> OCLDevicesTypes_1_INT;
typedef testing::Types<ALL_DEVICE_TYPES_1_LONG> OCLDevicesTypes_1_LONG;
typedef testing::Types<ALL_DEVICE_TYPES_1_FLOAT> OCLDevicesTypes_1_FLOAT;

TYPED_TEST_CASE_P(VmiscFunctions_TestCase_1_CHAR);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_1_SHORT);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_1_INT);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_1_LONG);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_1_FLOAT);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  shuffle     |   ggUi 
 */

#define DEFINE_TYPED_TEST_P(T)                              \
TYPED_TEST_P(VmiscFunctions_TestCase_1_ ## T, shuffle) {    \
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);                \
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_mask);             \
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);                 \
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);               \
                                                            \
  std::valarray<int> x, mask, expected;                     \
                                                            \
  switch (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(1)) {           \
    case 2:                                                 \
      x = { 1, 2 };                                         \
      switch (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(2)) {       \
        case 2:                                             \
          mask = { 1, 0 };                                  \
          expected = { 2, 1 };                              \
          break;                                            \
        case 4:                                             \
          mask = { 1, 0, 1, 0 };                            \
          expected = { 2, 1, 2, 1 };                        \
          break;                                            \
        case 8:                                             \
          mask = { 1, 0, 1, 0, 1, 0, 1, 0 };                \
          expected = { 2, 1, 2, 1, 2, 1, 2, 1 };            \
          break;                                            \
        case 16:                                            \
          mask = { 1, 0, 1, 0, 1, 0, 1, 0,                  \
                   1, 0, 1, 0, 1, 0, 1, 0 };                \
          expected = { 2, 1, 2, 1, 2, 1, 2, 1,              \
                       2, 1, 2, 1, 2, 1, 2, 1 };            \
          break;                                            \
      }                                                     \
      break;                                                \
    case 4:                                                 \
      x = { 1, 2, 3, 4 };                                   \
      switch (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(2)) {       \
        case 2:                                             \
          mask = { 3, 2 };                                  \
          expected = { 4, 3 };                              \
          break;                                            \
        case 4:                                             \
          mask = { 3, 2, 1, 0 };                            \
          expected = { 4, 3, 2, 1 };                        \
          break;                                            \
        case 8:                                             \
          mask = { 3, 2, 1, 0, 3, 2, 1, 0 };                \
          expected = { 4, 3, 2, 1, 4, 3, 2, 1 };            \
          break;                                            \
        case 16:                                            \
          mask = { 3, 2, 1, 0, 3, 2, 1, 0,                  \
                   3, 2, 1, 0, 3, 2, 1, 0 };                \
          expected = { 4, 3, 2, 1, 4, 3, 2, 1,              \
                       4, 3, 2, 1, 4, 3, 2, 1 };            \
          break;                                            \
      }                                                     \
      break;                                                \
    case 8:                                                 \
      x = { 1, 2, 3, 4, 5, 6, 7, 8 };                       \
      switch (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(2)) {       \
        case 2:                                             \
          mask = { 7, 6 };                                  \
          expected = { 8, 7 };                              \
          break;                                            \
        case 4:                                             \
          mask = { 7, 6, 5, 4 };                            \
          expected = { 8, 7, 6, 5 };                        \
          break;                                            \
        case 8:                                             \
          mask = { 7, 6, 5, 4, 3, 2, 1, 0 };                \
          expected = { 8, 7, 6, 5, 4, 3, 2, 1 };            \
          break;                                            \
        case 16:                                            \
          mask = { 7, 6, 5, 4, 3, 2, 1, 0,                  \
                   7, 6, 5, 4, 3, 2, 1, 0 };                \
          expected = { 8, 7, 6, 5, 4, 3, 2, 1,              \
                        8, 7, 6, 5, 4, 3, 2, 1 };           \
          break;                                            \
      }                                                     \
      break;                                                \
    case 16:                                                \
      x = { 1,  2,  3,  4,  5,  6,  7,  8,                  \
            9, 10, 11, 12, 13, 14, 15, 16 };                \
      switch (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(2)) {       \
        case 2:                                             \
          mask = { 15, 14 };                                \
          expected = { 16, 15 };                            \
          break;                                            \
        case 4:                                             \
          mask = { 15, 14, 13, 12 };                        \
          expected = { 16, 15, 14, 13 };                    \
          break;                                            \
        case 8:                                             \
          mask = { 15, 14, 13, 12, 11, 10, 9, 8 };          \
          expected = { 16, 15, 14, 13, 12, 11, 10, 9 };     \
          break;                                            \
        case 16:                                            \
          mask = { 15, 14, 13, 12, 11, 10, 9, 8,            \
                    7,  6,  5,  4,  3,  2, 1, 0 };          \
          expected = { 16, 15, 14, 13, 12, 11, 10, 9,       \
                        8,  7,  6,  5,  4,  3,  2, 1 };     \
          break;                                            \
      }                                                     \
       break;                                               \
  }                                                         \
                                                            \
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, x);             \
  Input_mask = GENTYPE_CREATE_TUPLE_ELEMENT(2, mask);       \
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, expected);    \
  this->Invoke("shuffle", Output, Input_x, Input_mask);     \
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);     \
}

DEFINE_TYPED_TEST_P(CHAR)
DEFINE_TYPED_TEST_P(SHORT)
DEFINE_TYPED_TEST_P(INT)
DEFINE_TYPED_TEST_P(LONG)
DEFINE_TYPED_TEST_P(FLOAT)

REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_1_CHAR, shuffle);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_1_SHORT, shuffle);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_1_INT, shuffle);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_1_LONG, shuffle);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_1_FLOAT, shuffle);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_1_CHAR, OCLDevicesTypes_1_CHAR);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_1_SHORT, OCLDevicesTypes_1_SHORT);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_1_INT, OCLDevicesTypes_1_INT);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_1_LONG, OCLDevicesTypes_1_LONG);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_1_FLOAT, OCLDevicesTypes_1_FLOAT);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class VmiscFunctions_TestCase_2_CHAR : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                    typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_2_SHORT : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                     typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_2_INT : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                   typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_2_LONG : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                    typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_2_FLOAT : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                     typename TyTy::Type> { };

#define LAST_VECTOR_TYPES_N_M_2_CHAR(D, N, M)                                               \
  DeviceTypePair<D, std::tuple<cl_char ## N, cl_char ## M, cl_char ## M, cl_uchar ## N>>,   \
  DeviceTypePair<D, std::tuple<cl_uchar ## N, cl_uchar ## M, cl_uchar ## M, cl_uchar ## N>>

#define LAST_VECTOR_TYPES_N_M_2_SHORT(D, N, M)                                                  \
  DeviceTypePair<D, std::tuple<cl_short ## N, cl_short ## M, cl_short ## M, cl_ushort ## N>>,   \
  DeviceTypePair<D, std::tuple<cl_ushort ## N, cl_ushort ## M, cl_ushort ## M, cl_ushort ## N>>

#define LAST_VECTOR_TYPES_N_M_2_INT(D, N, M)                                            \
  DeviceTypePair<D, std::tuple<cl_int ## N, cl_int ## M, cl_int ## M, cl_uint ## N>>,   \
  DeviceTypePair<D, std::tuple<cl_uint ## N, cl_uint ## M, cl_uint ## M, cl_uint ## N>>

#define LAST_VECTOR_TYPES_N_M_2_LONG(D, N, M)                                               \
  DeviceTypePair<D, std::tuple<cl_long ## N, cl_long ## M, cl_long ## M, cl_ulong ## N>>,   \
  DeviceTypePair<D, std::tuple<cl_ulong ## N, cl_ulong ## M, cl_ulong ## M, cl_ulong ## N>>

#ifdef cl_khr_fp64 
#define LAST_VECTOR_TYPES_N_M_2_FLOAT(D, N, M)                                                  \
  DeviceTypePair<D, std::tuple<cl_float ## N, cl_float ## M, cl_float ## M, cl_uint ## N>>,     \
  DeviceTypePair<D, std::tuple<cl_double ## N, cl_double ## M, cl_double ## M, cl_ulong ## N>>
#else
#define LAST_VECTOR_TYPES_N_M_2_FLOAT(D, N, M)                                                  \
  DeviceTypePair<D, std::tuple<cl_float ## N, cl_float ## M, cl_float ## M, cl_uint ## N>>
#endif
  
#define LAST_VECTOR_TYPES_N_M_2(D, T)       \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 2, 2),   \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 2, 4),   \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 2, 8),   \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 2, 16),  \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 4, 2),   \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 4, 4),   \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 4, 8),   \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 4, 16),  \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 8, 2),   \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 8, 4),   \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 8, 8),   \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 8, 16),  \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 16, 2),  \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 16, 4),  \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 16, 8),  \
  LAST_VECTOR_TYPES_N_M_2_ ## T(D, 16, 16)

#define ALL_DEVICE_TYPES_2_CHAR \
  LAST_VECTOR_TYPES_N_M_2(CPUDev, CHAR)

#define ALL_DEVICE_TYPES_2_SHORT \
  LAST_VECTOR_TYPES_N_M_2(CPUDev, SHORT)

#define ALL_DEVICE_TYPES_2_INT \
  LAST_VECTOR_TYPES_N_M_2(CPUDev, INT)

#define ALL_DEVICE_TYPES_2_LONG \
  LAST_VECTOR_TYPES_N_M_2(CPUDev, LONG)
  
#define ALL_DEVICE_TYPES_2_FLOAT \
  LAST_VECTOR_TYPES_N_M_2(CPUDev, FLOAT)

typedef testing::Types<ALL_DEVICE_TYPES_2_CHAR> OCLDevicesTypes_2_CHAR;
typedef testing::Types<ALL_DEVICE_TYPES_2_SHORT> OCLDevicesTypes_2_SHORT;
typedef testing::Types<ALL_DEVICE_TYPES_2_INT> OCLDevicesTypes_2_INT;
typedef testing::Types<ALL_DEVICE_TYPES_2_LONG> OCLDevicesTypes_2_LONG;
typedef testing::Types<ALL_DEVICE_TYPES_2_FLOAT> OCLDevicesTypes_2_FLOAT;

TYPED_TEST_CASE_P(VmiscFunctions_TestCase_2_CHAR);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_2_SHORT);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_2_INT);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_2_LONG);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_2_FLOAT);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  shuffle2    |   gggUi 
 */

#define DEFINE_TYPED_TEST_P_2(T)                                    \
TYPED_TEST_P(VmiscFunctions_TestCase_2_ ## T, shuffle2) {           \
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_x);                        \
  GENTYPE_DECLARE_TUPLE_ELEMENT(2, Input_y);                        \
  GENTYPE_DECLARE_TUPLE_ELEMENT(3, Input_mask);                     \
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);                         \
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);                       \
                                                                    \
  std::valarray<int> x, y, mask, expected;                          \
                                                                    \
  switch (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(1)) {                   \
    case 2:                                                         \
      x = { 1, 2 };                                                 \
      y = { 3, 4 };                                                 \
      switch (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(3)) {               \
        case 2:                                                     \
          mask = { 3, 0 };                                          \
          expected = { 4, 1 };                                      \
          break;                                                    \
        case 4:                                                     \
          mask = { 3, 2, 1, 0 };                                    \
          expected = { 4, 3, 2, 1 };                                \
          break;                                                    \
        case 8:                                                     \
          mask = { 3, 2, 1, 0, 3, 2, 1, 0 };                        \
          expected = { 4, 3, 2, 1, 4, 3, 2, 1 };                    \
          break;                                                    \
        case 16:                                                    \
          mask = { 3, 2, 1, 0, 3, 2, 1, 0,                          \
                   3, 2, 1, 0, 3, 2, 1, 0 };                        \
          expected = { 4, 3, 2, 1, 4, 3, 2, 1,                      \
                       4, 3, 2, 1, 4, 3, 2, 1 };                    \
          break;                                                    \
      }                                                             \
      break;                                                        \
    case 4:                                                         \
      x = { 1, 2, 3, 4 };                                           \
      y = { 5, 6, 7, 8 };                                           \
      switch (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(3)) {               \
        case 2:                                                     \
          mask = { 7, 0 };                                          \
          expected = { 8, 1 };                                      \
          break;                                                    \
        case 4:                                                     \
          mask = { 7, 6, 1, 0 };                                    \
          expected = { 8, 7, 2, 1 };                                \
          break;                                                    \
        case 8:                                                     \
          mask = { 7, 6, 5, 4, 3, 2, 1, 0 };                        \
          expected = { 8, 7, 6, 5, 4, 3, 2, 1 };                    \
          break;                                                    \
        case 16:                                                    \
          mask = { 7, 6, 5, 4, 3, 2, 1, 0,                          \
                   7, 6, 5, 4, 3, 2, 1, 0 };                        \
          expected = { 8, 7, 6, 5, 4, 3, 2, 1,                      \
                       8, 7, 6, 5, 4, 3, 2, 1 };                    \
          break;                                                    \
      }                                                             \
      break;                                                        \
    case 8:                                                         \
      x = { 1, 2, 3, 4, 5, 6, 7, 8 };                               \
      y = { 9, 10, 11, 12, 13, 14, 15, 16 };                        \
      switch (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(3)) {               \
        case 2:                                                     \
          mask = { 15, 0 };                                         \
          expected = { 16, 1 };                                     \
          break;                                                    \
        case 4:                                                     \
          mask = { 15, 14, 1, 0 };                                  \
          expected = { 16, 15, 2, 1 };                              \
          break;                                                    \
        case 8:                                                     \
          mask = { 15, 14, 13, 12, 3, 2, 1, 0 };                    \
          expected = { 16, 15, 14, 13, 4, 3, 2, 1 };                \
          break;                                                    \
        case 16:                                                    \
          mask = { 15, 14, 13, 12, 11, 10, 9, 8,                    \
                    7,  6,  5,  4,  3,  2, 1, 0 };                  \
          expected = { 16, 15, 14, 13, 12, 11, 10, 9,               \
                        8,  7,  6,  5,  4,  3,  2, 1 };             \
          break;                                                    \
      }                                                             \
      break;                                                        \
    case 16:                                                        \
      x = { 1,  2,  3,  4,  5,  6,  7,  8,                          \
            9, 10, 11, 12, 13, 14, 15, 16 };                        \
      y = { 17, 18, 19, 20, 21, 22, 23, 24,                         \
            25, 26, 27, 28, 29, 30, 31, 32 };                       \
      switch (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(3)) {               \
        case 2:                                                     \
          mask = { 31, 0 };                                         \
          expected = { 32, 1 };                                     \
          break;                                                    \
        case 4:                                                     \
          mask = { 31, 30, 1, 0 };                                  \
          expected = { 32, 31, 2, 1 };                              \
          break;                                                    \
        case 8:                                                     \
          mask = { 31, 30, 29, 28, 3, 2, 1, 0 };                    \
          expected = { 32, 31, 30, 29, 4, 3, 2, 1 };                \
          break;                                                    \
        case 16:                                                    \
          mask = { 31, 30, 29, 28, 27, 26, 25, 24,                  \
                    7,  6,  5,  4,  3,  2,  1,  0 };                \
          expected = { 32, 31, 30, 29, 28, 27, 26, 25,              \
                        8,  7,  6,  5,  4,  3,  2,  1 };            \
          break;                                                    \
      }                                                             \
      break;                                                        \
  }                                                                 \
                                                                    \
  Input_x = GENTYPE_CREATE_TUPLE_ELEMENT(1, x);                     \
  Input_y = GENTYPE_CREATE_TUPLE_ELEMENT(2, y);                     \
  Input_mask = GENTYPE_CREATE_TUPLE_ELEMENT(3, mask);               \
  Expected =  GENTYPE_CREATE_TUPLE_ELEMENT(0, expected);            \
  this->Invoke("shuffle2", Output, Input_x, Input_y, Input_mask);   \
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);             \
}

DEFINE_TYPED_TEST_P_2(CHAR)
DEFINE_TYPED_TEST_P_2(SHORT)
DEFINE_TYPED_TEST_P_2(INT)
DEFINE_TYPED_TEST_P_2(LONG)
DEFINE_TYPED_TEST_P_2(FLOAT)

REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_2_CHAR, shuffle2);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_2_SHORT, shuffle2);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_2_INT, shuffle2);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_2_LONG, shuffle2);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_2_FLOAT, shuffle2);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_2_CHAR, OCLDevicesTypes_2_CHAR);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_2_SHORT, OCLDevicesTypes_2_SHORT);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_2_INT, OCLDevicesTypes_2_INT);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_2_LONG, OCLDevicesTypes_2_LONG);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_2_FLOAT, OCLDevicesTypes_2_FLOAT);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class VmiscFunctions_TestCase_3_CHAR : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                    typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_3_SHORT : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                     typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_3_INT : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                   typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_3_LONG : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                    typename TyTy::Type> { };
template <typename TyTy>
class VmiscFunctions_TestCase_3_FLOAT : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                     typename TyTy::Type> { };

#define LAST_SCALAR_TYPES_3_CHAR(D)                 \
  DeviceTypePair<D, std::tuple<cl_int, cl_char>>,   \
  DeviceTypePair<D, std::tuple<cl_int, cl_uchar>>

#define LAST_VECTOR_TYPES_3_CHAR(D, S)                  \
  DeviceTypePair<D, std::tuple<cl_int, cl_char ## S>>,  \
  DeviceTypePair<D, std::tuple<cl_int, cl_uchar ## S>>

#define LAST_SCALAR_TYPES_3_SHORT(D)                \
  DeviceTypePair<D, std::tuple<cl_int, cl_short>>,  \
  DeviceTypePair<D, std::tuple<cl_int, cl_ushort>>

#define LAST_VECTOR_TYPES_3_SHORT(D, S)                  \
  DeviceTypePair<D, std::tuple<cl_int, cl_short ## S>>,  \
  DeviceTypePair<D, std::tuple<cl_int, cl_ushort ## S>>

#define LAST_SCALAR_TYPES_3_INT(D)                  \
  DeviceTypePair<D, std::tuple<cl_int, cl_int>>,    \
  DeviceTypePair<D, std::tuple<cl_int, cl_uint>>

#define LAST_VECTOR_TYPES_3_INT(D, S)                   \
  DeviceTypePair<D, std::tuple<cl_int, cl_int ## S>>,   \
  DeviceTypePair<D, std::tuple<cl_int, cl_uint ## S>>

#define LAST_SCALAR_TYPES_3_LONG(D)                 \
  DeviceTypePair<D, std::tuple<cl_int, cl_long>>,   \
  DeviceTypePair<D, std::tuple<cl_int, cl_ulong>>

#define LAST_VECTOR_TYPES_3_LONG(D, S)                   \
  DeviceTypePair<D, std::tuple<cl_int, cl_long ## S>>,   \
  DeviceTypePair<D, std::tuple<cl_int, cl_ulong ## S>>

#ifdef cl_khr_fp64 
#define LAST_SCALAR_TYPES_3_FLOAT(D)                \
  DeviceTypePair<D, std::tuple<cl_int, cl_float>>,  \
  DeviceTypePair<D, std::tuple<cl_int, cl_double>>

#define LAST_VECTOR_TYPES_3_FLOAT(D, S)                 \
  DeviceTypePair<D, std::tuple<cl_int, cl_float ## S>>, \
  DeviceTypePair<D, std::tuple<cl_int, cl_double ## S>>
#else
#define LAST_SCALAR_TYPES_3_FLOAT(D)                \
  DeviceTypePair<D, std::tuple<cl_int, cl_float>>

#define LAST_VECTOR_TYPES_3_FLOAT(D, S)                 \
  DeviceTypePair<D, std::tuple<cl_int, cl_float ## S>>
#endif

#define LAST_ALL_TYPES_3(D, T)      \
  LAST_SCALAR_TYPES_3_ ## T(D),     \
  LAST_VECTOR_TYPES_3_ ## T(D, 2),  \
  LAST_VECTOR_TYPES_3_ ## T(D, 3),  \
  LAST_VECTOR_TYPES_3_ ## T(D, 4),  \
  LAST_VECTOR_TYPES_3_ ## T(D, 8),  \
  LAST_VECTOR_TYPES_3_ ## T(D, 16)

#define ALL_DEVICE_TYPES_3_CHAR \
  LAST_ALL_TYPES_3(CPUDev, CHAR)

#define ALL_DEVICE_TYPES_3_SHORT \
  LAST_ALL_TYPES_3(CPUDev, SHORT)

#define ALL_DEVICE_TYPES_3_INT \
  LAST_ALL_TYPES_3(CPUDev, INT)

#define ALL_DEVICE_TYPES_3_LONG \
  LAST_ALL_TYPES_3(CPUDev, LONG)

#define ALL_DEVICE_TYPES_3_FLOAT \
  LAST_ALL_TYPES_3(CPUDev, FLOAT)

typedef testing::Types<ALL_DEVICE_TYPES_3_CHAR> OCLDevicesTypes_3_CHAR;
typedef testing::Types<ALL_DEVICE_TYPES_3_SHORT> OCLDevicesTypes_3_SHORT;
typedef testing::Types<ALL_DEVICE_TYPES_3_INT> OCLDevicesTypes_3_INT;
typedef testing::Types<ALL_DEVICE_TYPES_3_LONG> OCLDevicesTypes_3_LONG;
typedef testing::Types<ALL_DEVICE_TYPES_3_FLOAT> OCLDevicesTypes_3_FLOAT;

TYPED_TEST_CASE_P(VmiscFunctions_TestCase_3_CHAR);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_3_SHORT);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_3_INT);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_3_LONG);
TYPED_TEST_CASE_P(VmiscFunctions_TestCase_3_FLOAT);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  vec_step    |   Cng, CnVg 
 *
 *  The vec_step function is natively provided by Clang along with
 *  other common unary expressions (sizeof, alignof, ...). Thus, the
 *  following tests have been added only for completeness.
 */

#define DEFINE_TYPED_TEST_P_3(T)                                    \
TYPED_TEST_P(VmiscFunctions_TestCase_3_ ## T, vec_step) {           \
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_a);                        \
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);                         \
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);                       \
                                                                    \
  Input_a = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);                     \
  switch (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(1)) {                   \
  case 1:                                                           \
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 1);                  \
    break;                                                          \
  case 2:                                                           \
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 2);                  \
    break;                                                          \
  case 3:                                                           \
  case 4:                                                           \
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 4);                  \
    break;                                                          \
  case 8:                                                           \
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 8);                  \
    break;                                                          \
  case 16:                                                          \
    Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, 16);                 \
    break;                                                          \
  }                                                                 \
                                                                    \
  this->Invoke("vec_step", Output, Input_a);                        \
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);             \
}

DEFINE_TYPED_TEST_P_3(CHAR)
DEFINE_TYPED_TEST_P_3(SHORT)
DEFINE_TYPED_TEST_P_3(INT)
DEFINE_TYPED_TEST_P_3(LONG)
DEFINE_TYPED_TEST_P_3(FLOAT)

REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_3_CHAR, vec_step);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_3_SHORT, vec_step);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_3_INT, vec_step);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_3_LONG, vec_step);
REGISTER_TYPED_TEST_CASE_P(VmiscFunctions_TestCase_3_FLOAT, vec_step);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_3_CHAR, OCLDevicesTypes_3_CHAR);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_3_SHORT, OCLDevicesTypes_3_SHORT);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_3_INT, OCLDevicesTypes_3_INT);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_3_LONG, OCLDevicesTypes_3_LONG);
INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VmiscFunctions_TestCase_3_FLOAT, OCLDevicesTypes_3_FLOAT);
