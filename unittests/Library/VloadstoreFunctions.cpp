
#include "LibraryFixture.h"

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class VloadstoreFunctions_TestCase_1 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                    typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_VECTOR_FLOAT_TYPES_1(D, S)                                 \
  DeviceTypePair<D, std::tuple<cl_float ## S, size_t, cl_float *>>,     \
  DeviceTypePair<D, std::tuple<cl_double ## S, size_t, cl_double *>>
#else
#define LAST_VECTOR_FLOAT_TYPES_1(D, S)                                 \
  DeviceTypePair<D, std::tuple<cl_float ## S, size_t, cl_float *>>
#endif

#define LAST_VECTOR_TYPES_1(D, S)                                       \
  DeviceTypePair<D, std::tuple<cl_char ## S, size_t, cl_char *>>,       \
  DeviceTypePair<D, std::tuple<cl_uchar ## S, size_t, cl_uchar *>>,     \
  DeviceTypePair<D, std::tuple<cl_short ## S, size_t, cl_short *>>,     \
  DeviceTypePair<D, std::tuple<cl_ushort ## S, size_t, cl_ushort *>>,   \
  DeviceTypePair<D, std::tuple<cl_int ## S, size_t, cl_int *>>,         \
  DeviceTypePair<D, std::tuple<cl_uint ## S, size_t, cl_uint *>>,       \
  DeviceTypePair<D, std::tuple<cl_long ## S, size_t, cl_long *>>,       \
  DeviceTypePair<D, std::tuple<cl_ulong ## S, size_t, cl_ulong *>>,     \
  LAST_VECTOR_FLOAT_TYPES_1(D, S)

#define LAST_ALL_TYPES_1(D)     \
  LAST_VECTOR_TYPES_1(D, 2),    \
  LAST_VECTOR_TYPES_1(D, 3),    \
  LAST_VECTOR_TYPES_1(D, 4),    \
  LAST_VECTOR_TYPES_1(D, 8),    \
  LAST_VECTOR_TYPES_1(D, 16)

#define ALL_DEVICE_TYPES_1 \
  LAST_ALL_TYPES_1(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_1> OCLDevicesTypes_1;

TYPED_TEST_CASE_P(VloadstoreFunctions_TestCase_1);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  vload<n>    |   VgzP1Sg, VgzP2Sg, VgzP3Sg, VgzP4Sg
 *
 *  n := { 2, 3, 4, 8, 16 }
 */

TYPED_TEST_P(VloadstoreFunctions_TestCase_1, vloadn) {
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_offset);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(2, Input_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Expected);
  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Output);

  unsigned VecStep = GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(0); 
  switch (VecStep) {
    case 2:
      Input_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ 1, 1, 2, 2 })));
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, INIT_LIST(int, ({ 2, 2 })));
      break;
    case 3:
      Input_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ 1, 1, 1, 2, 2, 2 })));
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, INIT_LIST(int, ({ 2, 2, 2 })));
      break;
    case 4:
      Input_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ 1, 1, 1, 1, 2, 2, 2, 2 })));
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, INIT_LIST(int, ({ 2, 2, 2, 2 })));
      break;
    case 8:
      Input_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2 })));
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, INIT_LIST(int, ({ 2, 2, 2, 2, 2, 2, 2, 2 })));
      break;
    case 16:
      Input_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2,
          INIT_LIST(int, ({ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 })));
      Expected = GENTYPE_CREATE_TUPLE_ELEMENT(0, INIT_LIST(int, ({ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 })));
      break;
  }

  // Load the second gentypen element from *p .
  Input_offset = GENTYPE_CREATE_TUPLE_ELEMENT(1, 1);
  
  this->Invoke("vload" + std::to_string(VecStep), Output, Input_offset, Input_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(0, Expected, Output);
}

REGISTER_TYPED_TEST_CASE_P(VloadstoreFunctions_TestCase_1, vloadn);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VloadstoreFunctions_TestCase_1, OCLDevicesTypes_1);

// ---------------------------------------------------------------------------------------- //

template <typename TyTy>
class VloadstoreFunctions_TestCase_2 : public LibraryGenTypeFixture<typename TyTy::Dev,
                                                                    typename TyTy::Type> { };

#ifdef cl_khr_fp64
#define LAST_VECTOR_FLOAT_TYPES_2(D, S)                                         \
  DeviceTypePair<D, std::tuple</*void,*/ cl_float ## S, size_t, cl_float *>>,   \
  DeviceTypePair<D, std::tuple</*void,*/ cl_double ## S, size_t, cl_double *>>
#else
#define LAST_VECTOR_FLOAT_TYPES_2(D, S)                                         \
  DeviceTypePair<D, std::tuple</*void,*/ cl_float ## S, size_t, cl_float *>>
#endif

#define LAST_VECTOR_TYPES_2(D, S)                                               \
  DeviceTypePair<D, std::tuple</*void,*/ cl_char ## S, size_t, cl_char *>>,     \
  DeviceTypePair<D, std::tuple</*void,*/ cl_uchar ## S, size_t, cl_uchar *>>,   \
  DeviceTypePair<D, std::tuple</*void,*/ cl_short ## S, size_t, cl_short *>>,   \
  DeviceTypePair<D, std::tuple</*void,*/ cl_ushort ## S, size_t, cl_ushort *>>, \
  DeviceTypePair<D, std::tuple</*void,*/ cl_int ## S, size_t, cl_int *>>,       \
  DeviceTypePair<D, std::tuple</*void,*/ cl_uint ## S, size_t, cl_uint *>>,     \
  DeviceTypePair<D, std::tuple</*void,*/ cl_long ## S, size_t, cl_long *>>,     \
  DeviceTypePair<D, std::tuple</*void,*/ cl_ulong ## S, size_t, cl_ulong *>>,   \
  LAST_VECTOR_FLOAT_TYPES_2(D, S)

#define LAST_ALL_TYPES_2(D)     \
  LAST_VECTOR_TYPES_2(D, 2),    \
  LAST_VECTOR_TYPES_2(D, 3),    \
  LAST_VECTOR_TYPES_2(D, 4),    \
  LAST_VECTOR_TYPES_2(D, 8),    \
  LAST_VECTOR_TYPES_2(D, 16)

#define ALL_DEVICE_TYPES_2 \
  LAST_ALL_TYPES_2(CPUDev)

typedef testing::Types<ALL_DEVICE_TYPES_2> OCLDevicesTypes_2;

TYPED_TEST_CASE_P(VloadstoreFunctions_TestCase_2);

/*  Function    |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  vstore<n>    |  vVgzP1Sg, vVgzP2Sg, vVgzP4Sg
 *
 *  n := { 2, 3, 4, 8, 16 }
 */

TYPED_TEST_P(VloadstoreFunctions_TestCase_2, vstoren) {
  unsigned VecStep = GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(0); 

  GENTYPE_DECLARE_TUPLE_ELEMENT(0, Input_data);
  GENTYPE_DECLARE_TUPLE_ELEMENT(1, Input_offset);
  GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(2, VecStep, Output_p);
  GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(2, Expected_p);

  switch (VecStep) {
    case 2:
      Input_data = GENTYPE_CREATE_TUPLE_ELEMENT(0, INIT_LIST(int, ({ 1, 1, 2, 2 })));
      Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ 1, 1 })));
      break;
    case 3:
      Input_data = GENTYPE_CREATE_TUPLE_ELEMENT(0, INIT_LIST(int, ({ 1, 1, 1, 2, 2, 2 })));
      Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ 1, 1, 1 })));
      break;
    case 4:
      Input_data = GENTYPE_CREATE_TUPLE_ELEMENT(0, INIT_LIST(int, ({ 1, 1, 1, 1, 2, 2, 2, 2 })));
      Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ 1, 1, 1, 1 })));
      break;
    case 8:
      Input_data = GENTYPE_CREATE_TUPLE_ELEMENT(0, INIT_LIST(int, ({ 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2 })));
      Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2, INIT_LIST(int, ({ 1, 1, 1, 1, 1, 1, 1, 1 })));
      break;
    case 16:
      Input_data = GENTYPE_CREATE_TUPLE_ELEMENT(0,
          INIT_LIST(int, ({ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 })));
      Expected_p = GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(2,
          INIT_LIST(int, ({ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 })));
      break;
  }

  Input_offset = GENTYPE_CREATE_TUPLE_ELEMENT(1, 0);
  
  this->VoidRetTyInvoke("vstore" + std::to_string(VecStep), Input_data, Input_offset, Output_p);
  ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(2, Expected_p, Output_p);
}

REGISTER_TYPED_TEST_CASE_P(VloadstoreFunctions_TestCase_2, vstoren);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, VloadstoreFunctions_TestCase_2, OCLDevicesTypes_2);
