
#include "LibraryFixture.h"

//
// DeviceTraits specializations.
//

#define SPECIALIZE_DEVICE_TRAITS(D, N) \
  llvm::StringRef DeviceTraits<D>::Name = N;

SPECIALIZE_DEVICE_TRAITS(CPUDev, "CPU")

#undef SPECIALIZE_DEVICE_TRAITS

//
// OCLTypeTraits specializations.
//

///////////////////////////////////////////////////////////////////////////////

#define SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(T) \
  template <>                               \
  void OCLTypeTraits<T>::AssertIsNaN(T A) { \
    ASSERT_TRUE(std::isnan(A));             \
  }

///////////////////////////////////////////////////////////////////////////////

#define SPECIALIZE_OCL_TYPE_TRAITS_SCALAR(T, N, AE) \
  template <>                                       \
  llvm::StringRef OCLTypeTraits<T>::OCLCName = N;   \
                                                    \
  template <>                                       \
  unsigned OCLTypeTraits<T>::VecStep = 1;           \
                                                    \
  template <>                                       \
  bool OCLTypeTraits<T>::DoesOutput = false;        \
                                                    \
  template <>                                       \
  void OCLTypeTraits<T>::AssertEq(T A, T B) {       \
    AE(A, B);                                       \
  }

#define SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC(T, N, S)           \
  template <>                                                   \
  llvm::StringRef OCLTypeTraits<T ## S>::OCLCName = N #S;       \
                                                                \
  template <>                                                   \
  unsigned OCLTypeTraits<T ## S>::VecStep = S;                  \
                                                                \
  template <>                                                   \
  bool OCLTypeTraits<T ## S>::DoesOutput = false;               \
                                                                \
  template <>                                                   \
  void OCLTypeTraits<T ## S>::AssertEq(T ## S A, T ## S B) {    \
    T RawA[S], RawB[S];                                         \
                                                                \
    std::memcpy(RawA, &A, sizeof(T ## S));                      \
    std::memcpy(RawB, &B, sizeof(T ## S));                      \
                                                                \
    for(unsigned I = 0; I < S; ++I)                             \
      OCLTypeTraits<T>::AssertEq(RawA[I], RawB[I]);             \
  }                                                             \
                                                                \
  template <>                                                   \
  void OCLTypeTraits<T ## S>::AssertIsNaN(T ## S A) {           \
    T RawA[S];                                                  \
                                                                \
    std::memcpy(RawA, &A, sizeof(T ## S));                      \
                                                                \
    for(unsigned I = 0; I < S; ++I)                             \
      OCLTypeTraits<T>::AssertIsNaN(RawA[I]);                   \
  }

///////////////////////////////////////////////////////////////////////////////

#define SPECIALIZE_OCL_TYPE_TRAITS_SCALAR_BUFFER(T, N, AE)              \
  template <>                                                           \
  llvm::StringRef OCLTypeTraits<std::valarray<T>>::OCLCName = N " *";   \
                                                                        \
  template <>                                                           \
  unsigned OCLTypeTraits<std::valarray<T>>::VecStep = 1;                \
                                                                        \
  template <>                                                           \
  bool OCLTypeTraits<std::valarray<T>>::DoesOutput = false;             \
                                                                        \
  template <>                                                           \
  void OCLTypeTraits<std::valarray<T>>::AssertEq(std::valarray<T> A,    \
                                                 std::valarray<T> B) {  \
    assert(A.size() == B.size());                                       \
    for (unsigned I = 0; I < A.size(); I++)                             \
      AE(A[I], B[I]);                                                   \
  }

#define SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER(T, N, S)                            \
  template <>                                                                           \
  llvm::StringRef OCLTypeTraits<std::valarray<T ## S>>::OCLCName = N #S " *";           \
                                                                                        \
  template <>                                                                           \
  unsigned OCLTypeTraits<std::valarray<T ##S>>::VecStep = S;                            \
                                                                                        \
  template <>                                                                           \
  bool OCLTypeTraits<std::valarray<T ## S>>::DoesOutput = false;                        \
                                                                                        \
  template <>                                                                           \
  void OCLTypeTraits<std::valarray<T ## S>>::AssertEq(std::valarray<T ## S> A,          \
                                                      std::valarray<T ## S> B) {        \
    assert(A.size() == B.size());                                                       \
    T RawA[S], RawB[S];                                                                 \
                                                                                        \
    for (unsigned I = 0; I < A.size(); I++) {                                           \
      std::memcpy(RawA, &A[I], sizeof(T ## S));                                         \
      std::memcpy(RawB, &B[I], sizeof(T ## S));                                         \
                                                                                        \
      for (unsigned J = 0; J < S; ++J)                                                  \
        OCLTypeTraits<T>::AssertEq(RawA[J], RawB[J]);                                   \
    }                                                                                   \
  }                                                                                     \
                                                                                        \
  template <>                                                                           \
  void OCLTypeTraits<std::valarray<T ## S>>::AssertIsNaN(std::valarray<T ## S> A) {     \
    T RawA[S];                                                                          \
                                                                                        \
    for (unsigned I = 0; I < A.size(); I++) {                                           \
      std::memcpy(RawA, &A[I], sizeof(T ## S));                                         \
                                                                                        \
      for(unsigned J = 0; J < S; ++J)                                                   \
        OCLTypeTraits<T>::AssertIsNaN(RawA[J]);                                         \
    }                                                                                   \
  }

///////////////////////////////////////////////////////////////////////////////

// (1) Creation of a scalar value:
//      
//     (cl_int) 1 <--- (int32_t) 1
//
// (2) Creation of a scalar value from a list (used by test cases where the list
//     is used for both scalar and vector creation):
// 
//     (cl_int) 1 <--- { 1, 2, 3, ...}

#define SPECIALIZE_OCL_TYPE_TRAITS_SCALAR_CREATE(T, V)  \
  template <> template <>                               \
  T OCLTypeTraits<T>::Create(V Val) {                   \
    return static_cast<T>(Val);                         \
  }                                                     \
                                                        \
  template <> template <>                               \
  T OCLTypeTraits<T>::Create(std::valarray<V> Val) {    \
    size_t ValSz = Val.size();                          \
    assert(ValSz > 0);                                  \
                                                        \
    return static_cast<T>(Val[0]);                      \
  }

// (3) Creation of a vector value from a scalar value:
//
//     (cl_int4) { 1, 1, 1, 1 } <--- (int32_t) 1
//
// (4) Creation of a vector value from a list:
//
//     (cl_int4) { 2, 2, 2, 2 } <--- { 2 }
//     (cl_int4) { 1, 1, 0, 0 } <--- { 1, 1 }
//     (cl_int4) { 1, 2, 3, 4 } <--- { 1, 2, 3, 4 }

#define SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE(T, S, V)    \
  template <> template <>                                       \
  T ## S OCLTypeTraits<T ## S>::Create(V Val) {                 \
    T Raw[S];                                                   \
                                                                \
    for(unsigned I = 0; I < S; ++I)                             \
      Raw[I] = OCLTypeTraits<T>::Create<V>(Val);                \
                                                                \
    T ## S Vect;                                                \
    std::memcpy(&Vect, Raw, sizeof(T ## S));                    \
                                                                \
    return Vect;                                                \
  }                                                             \
                                                                \
  template <> template <>                                       \
  T ## S OCLTypeTraits<T ## S>::Create(std::valarray<V> Val) {  \
    T Raw[S];                                                   \
    size_t ValSz = Val.size();                                  \
                                                                \
    assert(ValSz > 0);                                          \
    if(ValSz == 1) {                                            \
      for(unsigned I = 0; I < S; ++I)                           \
        Raw[I] = OCLTypeTraits<T>::Create<V>(Val[0]);           \
    } else {                                                    \
      unsigned I = 0;                                           \
      for( ; I < S && I < ValSz; ++I)                           \
        Raw[I] = OCLTypeTraits<T>::Create<V>(Val[I]);           \
      for( ; I < S; ++I)                                        \
        Raw[I] = 0;                                             \
    }                                                           \
                                                                \
    T ## S Vect;                                                \
    std::memcpy(&Vect, Raw, sizeof(T ## S));                    \
                                                                \
    return Vect;                                                \
  }

///////////////////////////////////////////////////////////////////////////////

// Buffer parameters of library functions (i.e. cl_int *, cl_int2 *, etc.) are
// contiguous memory areas which can be captured by the inner storage of
// vector-like containers, like std::valarray<>.

// (1) Creation of a buffer of scalar values from a list of scalar values:
//
//     (cl_int *) { 1, 2, 3, 4 } <--- { 1, 2, 3, 4 }

#define SPECIALIZE_OCL_TYPE_TRAITS_SCALAR_BUFFER_CREATE(T, V)                       \
  template <> template<>                                                            \
  std::valarray<T> OCLTypeTraits<std::valarray<T>>::Create(std::valarray<V> Val) {  \
    std::valarray<T> Vect(Val.size());                                              \
    for (unsigned I = 0; I < Val.size(); I++)                                       \
      Vect[I] = OCLTypeTraits<T>::Create<V>(Val[I]);                                \
                                                                                    \
    return Vect;                                                                    \
  }

// (2) Creation of a buffer of vector values from a nested list of scalar values:
//
//     (cl_int4 *) { { 1, 1, 1, 1 },   <---  { { 1, 1, 1, 1 },
//                   { 2, 2, 2, 2 } }          { 2, 2, 2, 2 } }
//
// (3) Creation of a buffer of vector values from a list of scalar values: 
//
//     (cl_int2 *) { { 1, 1 }, { 2, 2 } }  <---  { 1, 2 }

#define SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER_CREATE(T, S, V)                         \
  template <> template <>                                                                   \
  std::valarray<T ## S>                                                                     \
  OCLTypeTraits<std::valarray<T ## S>>::Create(std::valarray<std::valarray<V>> Val) {       \
    std::valarray<T ## S> Vect(Val.size());                                                 \
                                                                                            \
    for (unsigned I =0; I < Val.size(); ++I)                                                \
      Vect[I] = OCLTypeTraits<T ## S>::Create<std::valarray<V>>(Val[I]);                    \
                                                                                            \
    return Vect;                                                                            \
  }                                                                                         \
                                                                                            \
  template <> template <>                                                                   \
  std::valarray<T ## S>                                                                     \
  OCLTypeTraits<std::valarray<T ## S>>::Create(std::valarray<V> Val) {                      \
    std::valarray<T ## S> Vect(Val.size());                                                 \
                                                                                            \
    for (unsigned I = 0; I < Val.size(); ++I)                                               \
      Vect[I] = OCLTypeTraits<T ## S>::Create<V>(Val[I]);                                   \
                                                                                            \
    return Vect;                                                                            \
  }
  
///////////////////////////////////////////////////////////////////////////////

#define SPECIALIZE_OCL_TYPE_TRAITS(T, N, AE)        \
  SPECIALIZE_OCL_TYPE_TRAITS_SCALAR(T, N, AE)       \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC(T, N,  2)    \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC(T, N,  4)    \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC(T, N,  8)    \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC(T, N, 16)

#define SPECIALIZE_OCL_TYPE_TRAITS_BUFFER(T, N, AE)     \
  SPECIALIZE_OCL_TYPE_TRAITS_SCALAR_BUFFER(T, N, AE)    \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER(T, N,  2) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER(T, N,  4) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER(T, N,  8) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER(T, N, 16)

#define SPECIALIZE_OCL_TYPE_TRAITS_CREATE(T, V)         \
  SPECIALIZE_OCL_TYPE_TRAITS_SCALAR_CREATE(T, V)        \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE(T,  2, V) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE(T,  4, V) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE(T,  8, V) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE(T, 16, V)

#define SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE(T, V)          \
  SPECIALIZE_OCL_TYPE_TRAITS_SCALAR_BUFFER_CREATE(T, V)         \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER_CREATE(T,  2, V)  \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER_CREATE(T,  4, V)  \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER_CREATE(T,  8, V)  \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER_CREATE(T, 16, V)

#define SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_CREATE(V)    \
  SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_char, V)         \
  SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_uchar, V)        \
  SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_short, V)        \
  SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_ushort, V)       \
  SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_int, V)          \
  SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_uint, V)         \
  SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_long, V)         \
  SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_ulong, V)

#define SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE(V)  \
  SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_float, V)    \
  SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_double, V)

#define SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_BUFFER_CREATE(V) \
  SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE(cl_char, V)      \
  SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE(cl_uchar, V)     \
  SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE(cl_short, V)     \
  SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE(cl_ushort, V)    \
  SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE(cl_int, V)       \
  SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE(cl_uint, V)      \
  SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE(cl_long, V)      \
  SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE(cl_ulong, V)

#define SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE(V)   \
  SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE(cl_float, V)     \
  SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE(cl_double, V)

///////////////////////////////////////////////////////////////////////////////

// Fixed width integer specializations.

SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_char)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_uchar)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_short)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_ushort)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_int)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_uint)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_long)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_ulong)

SPECIALIZE_OCL_TYPE_TRAITS(cl_char, "char", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_uchar, "uchar", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_short, "short", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_ushort, "ushort", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_int, "int", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_uint, "uint", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_long, "long", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_ulong, "ulong", ASSERT_EQ)

SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_CREATE(int8_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_CREATE(uint8_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_CREATE(int16_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_CREATE(uint16_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_CREATE(int32_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_CREATE(uint32_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_CREATE(int64_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_CREATE(uint64_t)

// Single precision floating point specializations.

SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_float)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_double)

SPECIALIZE_OCL_TYPE_TRAITS(cl_float, "float", ASSERT_FLOAT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_double, "double", ASSERT_DOUBLE_EQ)

SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE(int8_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE(uint8_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE(int16_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE(uint16_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE(int32_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE(uint32_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE(int64_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE(uint64_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE(float)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE(double)

// Buffer specializations.

SPECIALIZE_OCL_TYPE_TRAITS_BUFFER(cl_char, "char", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS_BUFFER(cl_uchar, "uchar", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS_BUFFER(cl_short, "short", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS_BUFFER(cl_ushort, "ushort", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS_BUFFER(cl_int, "int", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS_BUFFER(cl_uint, "uint", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS_BUFFER(cl_long, "long", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS_BUFFER(cl_ulong, "ulong", ASSERT_EQ)
 
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_BUFFER_CREATE(int8_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_BUFFER_CREATE(uint8_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_BUFFER_CREATE(int16_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_BUFFER_CREATE(uint16_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_BUFFER_CREATE(int32_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_BUFFER_CREATE(uint32_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_BUFFER_CREATE(int64_t)
SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_BUFFER_CREATE(uint64_t)

SPECIALIZE_OCL_TYPE_TRAITS_BUFFER(cl_float, "float", ASSERT_FLOAT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS_BUFFER(cl_double, "double", ASSERT_FLOAT_EQ)

SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE(int8_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE(uint8_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE(int16_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE(uint16_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE(int32_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE(uint32_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE(int64_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE(uint64_t)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE(float)
SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE(double)

#undef SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_BUFFER_CREATE
#undef SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_BUFFER_CREATE
#undef SPECIALIZE_OCL_TYPE_TRAITS_INTEGER_CREATE
#undef SPECIALIZE_OCL_TYPE_TRAITS_FLOAT_CREATE

#undef SPECIALIZE_OCL_TYPE_TRAITS_BUFFER_CREATE
#undef SPECIALIZE_OCL_TYPE_TRAITS_SCALAR_BUFFER_CREATE
#undef SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER_CREATE
#undef SPECIALIZE_OCL_TYPE_TRAITS_CREATE
#undef SPECIALIZE_OCL_TYPE_TRAITS_SCALAR_CREATE
#undef SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE

#undef SPECIALIZE_OCL_TYPE_TRAITS_BUFFER
#undef SPECIALIZE_OCL_TYPE_TRAITS_SCALAR_BUFFER
#undef SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_BUFFER
#undef SPECIALIZE_OCL_TYPE_TRAITS
#undef SPECIALIZE_OCL_TYPE_TRAITS_SCALAR
#undef SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC

#undef SPECIALIZE_OCL_TYPE_TRAITS_ISNAN

//
// Specialization of googletest templates.
//

namespace testing {
namespace internal {

#define SPECIALIZE_TYPE_NAME(T) \
  template <>                   \
  String GetTypeName<T>() {     \
    return #T;                  \
  }

#define SPECIALIZE_COMPOUND_TYPE(D, T)                                  \
  template <>                                                           \
  String GetTypeName< DeviceTypePair<D, T> >() {                        \
    String DevName = GetTypeName<D>();                                  \
    String TyName = GetTypeName<T>();                                   \
                                                                        \
    return String::Format("(%s, %s)", DevName.c_str(), TyName.c_str()); \
  }

#define SPECIALIZE_TYPE(T)              \
  SPECIALIZE_TYPE_NAME(T)               \
  SPECIALIZE_TYPE_NAME(T *)             \
  SPECIALIZE_COMPOUND_TYPE(CPUDev, T)   \
  SPECIALIZE_COMPOUND_TYPE(CPUDev, T *)

// Do not specialize vector with 3 elements. OpenCL uses the same
// storage class as for 4 element vector.

#define SPECIALIZE_VEC_TYPE(T) \
  SPECIALIZE_TYPE(T ##  2)     \
  SPECIALIZE_TYPE(T ##  4)     \
  SPECIALIZE_TYPE(T ##  8)     \
  SPECIALIZE_TYPE(T ## 16)

// Device type specializations.

SPECIALIZE_TYPE_NAME(CPUDev)

// Fixed width integer specializations.

SPECIALIZE_TYPE(cl_char)
SPECIALIZE_TYPE(cl_uchar)
SPECIALIZE_TYPE(cl_short)
SPECIALIZE_TYPE(cl_ushort)
SPECIALIZE_TYPE(cl_int)
SPECIALIZE_TYPE(cl_uint)
SPECIALIZE_TYPE(cl_long)
SPECIALIZE_TYPE(cl_ulong)
SPECIALIZE_VEC_TYPE(cl_char)
SPECIALIZE_VEC_TYPE(cl_uchar)
SPECIALIZE_VEC_TYPE(cl_short)
SPECIALIZE_VEC_TYPE(cl_ushort)
SPECIALIZE_VEC_TYPE(cl_int)
SPECIALIZE_VEC_TYPE(cl_uint)
SPECIALIZE_VEC_TYPE(cl_long)
SPECIALIZE_VEC_TYPE(cl_ulong)

// Single precision floating point specializations.

SPECIALIZE_TYPE(cl_float)
SPECIALIZE_TYPE(cl_double)
SPECIALIZE_VEC_TYPE(cl_float)
SPECIALIZE_VEC_TYPE(cl_double)

#undef SPECIALIZE_TYPE_NAME
#undef SPECIALIZE_COMPOUND_TYPE
#undef SPECIALIZE_TYPE
#undef SPECIALIZE_VECT_TYPE

} // End namespace internal.
} // End namespace testing.
