
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

#define SPECIALIZE_OCL_TYPE_TRAITS(T, N, AE)      \
  template <>                                     \
  llvm::StringRef OCLTypeTraits<T>::OCLCName = N; \
                                                  \
  template <>                                     \
  void OCLTypeTraits<T>::AssertEq(T A, T B) {     \
    AE(A, B);                                     \
  }

#define SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC(T, N, S)        \
  template <>                                                \
  llvm::StringRef OCLTypeTraits<T ## S>::OCLCName = N #S;    \
                                                             \
  template <>                                                \
  void OCLTypeTraits<T ## S>::AssertEq(T ## S A, T ## S B) { \
    T RawA[S], RawB[S];                                      \
                                                             \
    std::memcpy(RawA, &A, sizeof(T ## S));                   \
    std::memcpy(RawB, &B, sizeof(T ## S));                   \
                                                             \
    for(unsigned I = 0; I < S; ++I)                          \
      OCLTypeTraits<T>::AssertEq(RawA[I], RawB[I]);          \
  }                                                          \
                                                             \
  template <>                                                \
  void OCLTypeTraits<T ## S>::AssertIsNaN(T ## S A) {        \
    T RawA[S];                                               \
                                                             \
    std::memcpy(RawA, &A, sizeof(T ## S));                   \
                                                             \
    for(unsigned I = 0; I < S; ++I)                          \
      OCLTypeTraits<T>::AssertIsNaN(RawA[I]);                \
  }

#define SPECIALIZE_OCL_TYPE_TRAITS_VEC(T, N)     \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC(T, N,  2) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC(T, N,  4) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC(T, N,  8) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC(T, N, 16)

#define SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(T) \
  template <>                               \
  void OCLTypeTraits<T>::AssertIsNaN(T A) { \
    ASSERT_TRUE(std::isnan(A));             \
  }

#define SPECIALIZE_OCL_TYPE_TRAITS_CREATE(T, V) \
  template <> template <>                       \
  T OCLTypeTraits<T>::Create(V Val) {           \
    return static_cast<T>(Val);                 \
  }

#define SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE(T, S, V) \
  template <> template <>                                    \
  T ## S OCLTypeTraits<T ## S>::Create(V Val) {              \
    T Raw[S];                                                \
                                                             \
    for(unsigned I = 0; I < S; ++I)                          \
      Raw[I] = OCLTypeTraits<T>::Create<V>(Val);             \
                                                             \
    T ## S Vect;                                             \
    std::memcpy(&Vect, Raw, sizeof(T ## S));                 \
                                                             \
    return Vect;                                             \
  }

#define SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(T, V)     \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE(T,  2, V) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE(T,  4, V) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE(T,  8, V) \
  SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE(T, 16, V)

// Fixed width integer specializations.

SPECIALIZE_OCL_TYPE_TRAITS(cl_char, "char", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_uchar, "uchar", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_short, "short", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_ushort, "ushort", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_int, "int", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_uint, "uint", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_long, "long", ASSERT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS(cl_ulong, "ulong", ASSERT_EQ)

SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_char)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_uchar)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_short)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_ushort)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_int)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_uint)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_long)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_ulong)

SPECIALIZE_OCL_TYPE_TRAITS_VEC(cl_char, "char")
SPECIALIZE_OCL_TYPE_TRAITS_VEC(cl_uchar, "uchar")
SPECIALIZE_OCL_TYPE_TRAITS_VEC(cl_short, "short")
SPECIALIZE_OCL_TYPE_TRAITS_VEC(cl_ushort, "ushort")
SPECIALIZE_OCL_TYPE_TRAITS_VEC(cl_int, "int")
SPECIALIZE_OCL_TYPE_TRAITS_VEC(cl_uint, "uint")
SPECIALIZE_OCL_TYPE_TRAITS_VEC(cl_long, "long")
SPECIALIZE_OCL_TYPE_TRAITS_VEC(cl_ulong, "ulong")

SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_char, int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_uchar, int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_short, int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_ushort, int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_int, int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_uint, int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_long, int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_ulong, int)

SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_char, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_uchar, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_short, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_ushort, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_int, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_uint, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_long, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_ulong, unsigned int)

SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_char, long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_uchar, long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_short, long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_ushort, long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_int, long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_uint, long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_long, long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_ulong, long)

SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_char, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_uchar, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_short, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_ushort, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_int, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_uint, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_long, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_ulong, unsigned long)

SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_char, int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_uchar, int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_short, int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_ushort, int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_int, int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_uint, int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_long, int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_ulong, int)

SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_char, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_uchar, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_short, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_ushort, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_int, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_uint, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_long, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_ulong, unsigned int)

SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_char, long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_uchar, long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_short, long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_ushort, long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_int, long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_uint, long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_long, long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_ulong, long)

SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_char, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_uchar, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_short, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_ushort, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_int, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_uint, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_long, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_ulong, unsigned long)

// Single precision floating point specializations.

SPECIALIZE_OCL_TYPE_TRAITS(cl_float, "float", ASSERT_FLOAT_EQ)
SPECIALIZE_OCL_TYPE_TRAITS_ISNAN(cl_float)
SPECIALIZE_OCL_TYPE_TRAITS_VEC(cl_float, "float")
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_float, int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_float, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_float, long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_float, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_float, float)
SPECIALIZE_OCL_TYPE_TRAITS_CREATE(cl_float, double)

SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_float, int) 
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_float, unsigned int)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_float, long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_float, unsigned long)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_float, float)
SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE(cl_float, double)

#undef SPECIALIZE_OCL_TYPE_TRAITS
#undef SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC
#undef SPECIALIZE_OCL_TYPE_TRAITS_VEC
#undef SPECIALIZE_OCL_TYPE_TRAITS_ISNAN
#undef SPECIALIZE_OCL_TYPE_TRAITS_CREATE
#undef SPECIALIZE_OCL_TYPE_TRAITS_SIZED_VEC_CREATE
#undef SPECIALIZE_OCL_TYPE_TRAITS_VEC_CREATE

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

#define SPECIALIZE_TYPE(T)            \
  SPECIALIZE_TYPE_NAME(T)             \
  SPECIALIZE_COMPOUND_TYPE(CPUDev, T)

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
SPECIALIZE_VEC_TYPE(cl_float)

#undef SPECIALIZE_TYPE_NAME
#undef SPECIALIZE_COMPOUND_TYPE
#undef SPECIALIZE_TYPE
#undef SPECIALIZE_VECT_TYPE

} // End namespace internal.
} // End namespace testing.
