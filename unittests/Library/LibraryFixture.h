
//===- LibraryFixture.h - Fixtures to automate library basic testing ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines test fixtures that automatically setup and OpenCL context
// where perform basic unit testing of library functions.
//
//===----------------------------------------------------------------------===//

#ifndef UNITTESTS_LIBRARY_LIBRARYFIXTURE_H
#define UNITTESTS_LIBRARY_LIBRARYFIXTURE_H

#include "CL/cl.hpp"

#include "gtest/gtest.h"

#include <map>
#include <tuple>
#include <valarray>

#define REGISTER_TYPED_TEST_CASE_P(CaseName, ...) \
  namespace GTEST_CASE_NAMESPACE_(CaseName) { \
  typedef ::testing::internal::Templates<__VA_ARGS__>::type gtest_AllTests_; \
  } \
  static const char* const GTEST_REGISTERED_TEST_NAMES_(CaseName) = \
      GTEST_TYPED_TEST_CASE_P_STATE_(CaseName).VerifyRegisteredTestNames(\
          __FILE__, __LINE__, #__VA_ARGS__)

//===----------------------------------------------------------------------===//
/// DeviceTraits - Traits of OpenCL devices. The LibraryFixture requires a
///  template type parameter describing the OpenCL device where the test will be
///  run. This class provides needed information. Please provide a SizeType
///  type definition in template instantiation. This because the device size_t
///  type has not a corresponding type in the host, but its size must be known
///  in order to allocate storage on the host.
//===----------------------------------------------------------------------===//
template <typename Ty>
class DeviceTraits {
public:
  static llvm::StringRef Name;
};

//===----------------------------------------------------------------------===//
/// OCLTypeTraits - Traits of OpenCL types. Usually tests operate on
///  user-provided data. Storage must be allocated on both the device and the
///  host. This class provides information on how to translate each host type to
///  the corresponding device type. Pay attention in template instantiation in
///  order to avoid template redefinitions.
//===----------------------------------------------------------------------===//
template <typename Ty>
class OCLTypeTraits {
public:
  template <typename ValTy>
  static Ty Create(ValTy Val);

  static void AssertEq(Ty A, Ty B);
  static void AssertIsNaN(Ty A);

public:
  static llvm::StringRef OCLCName;
  static unsigned VecStep;
};

//===----------------------------------------------------------------------===//
/// DeviceTypePair - A couple device type, tested type. This class allows to
///  pack two types into one type, providing specialized type names to access to
///  the device type and to the tested type type. It is needed to exploit
///  googletest typed test framework.
//===----------------------------------------------------------------------===//
template <typename DevTy, typename Ty>
class DeviceTypePair {
public:
  typedef DevTy Dev;
  typedef Ty Type;
};
 
//===----------------------------------------------------------------------===//
/// KernelArgType - Enum class to capture the expected I/O behaviour for kernel
/// arguments.
//===----------------------------------------------------------------------===//
enum class KernelArgType {
    Default,
    InputBuffer,
    OutputBuffer,
    InputOutputBuffer
};

using KernelArgAddr = void *;

//===----------------------------------------------------------------------===//
/// GENTTYPE_DECLARE - Declare a variable for generic type-based tests.
//===----------------------------------------------------------------------===//
#define GENTYPE_DECLARE(V)                          \
  typename TypeParam::Type V;                       \
  this->SetKernelArgType(&V, KernelArgType::Default)

#define GENTYPE_DECLARE_BUFFER(V)                           \
  std::valarray<typename std::remove_pointer<typename       \
    TypeParam::Type>::type> V;                              \
  this->SetKernelArgType(&V, KernelArgType::InputBuffer)

#define GENTYPE_DECLARE_OUT_BUFFER(S, V)                \
  std::valarray<typename std::remove_pointer<typename   \
    TypeParam::Type>::type> V(S);                       \
  this->SetKernelArgType(&V, KernelArgType::OutputBuffer)

#define GENTYPE_DECLARE_INOUT_BUFFER(V)                         \
  std::valarray<typename std::remove_pointer<typename           \
    TypeParam::Type>::type> V;                                  \
  this->SetKernelArgType(&V, KernelArgType::InputOutputBuffer)

#define GENTYPE_DECLARE_TUPLE_ELEMENT(I, V)                         \
  typename std::tuple_element<I, typename TypeParam::Type>::type V; \
  this->SetKernelArgType(&V, KernelArgType::Default)

#define GENTYPE_DECLARE_TUPLE_ELEMENT_BUFFER(I, V)                          \
  std::valarray<typename std::remove_pointer<typename std::tuple_element<I, \
    typename TypeParam::Type>::type>::type> V;                              \
  this->SetKernelArgType(&V, KernelArgType::InputBuffer)

#define GENTYPE_DECLARE_TUPLE_ELEMENT_OUT_BUFFER(I, S, V)                   \
  std::valarray<typename std::remove_pointer<typename std::tuple_element<I, \
    typename TypeParam::Type>::type>::type> V(S);                           \
  this->SetKernelArgType(&V, KernelArgType::OutputBuffer)

#define GENTYPE_DECLARE_TUPLE_ELEMENT_INOUT_BUFFER(I, V)                    \
  std::valarray<typename std::remove_pointer<typename std::tuple_element<I, \
    typename TypeParam::Type>::type>::type> V;                              \
  this->SetKernelArgType(&V, KernelArgType::InputOutputBuffer)

//===----------------------------------------------------------------------===//
/// GENTYPE_CREATE - Value creator for generic type-based tests.
//===----------------------------------------------------------------------===//
#define GENTYPE_CREATE(V) \
  (OCLTypeTraits<typename TypeParam::Type>::Create(V))

#define GENTYPE_CREATE_TUPLE_ELEMENT(I, V) \
  (OCLTypeTraits<typename std::tuple_element<I, \
   typename TypeParam::Type>::type>::Create(V))

#define GENTYPE_CREATE_BUFFER(V) \
  (OCLTypeTraits<std::valarray<typename std::remove_pointer<typename \
    TypeParam::Type>::type>>::Create(V))

#define GENTYPE_CREATE_TUPLE_ELEMENT_BUFFER(I, V) \
  (OCLTypeTraits<std::valarray<typename std::remove_pointer<typename std::tuple_element<I, \
   typename TypeParam::Type>::type>::type>>::Create(V))

//===----------------------------------------------------------------------===//
/// GENTYPE_GET - Get information for generic type-based tests.
//===----------------------------------------------------------------------===//
#define GENTYPE_GET_POINTEE_TYPE \
  typename std::remove_pointer<typename TypeParam::Type>::type

#define GENTYPE_GET_BUFFER_TYPE \
  typename std::valarray<typename std::remove_pointer<typename TypeParam::Type>::type>

#define GENTYPE_GET_TUPLE_ELEMENT_TYPE(I) \
  typename std::tuple_element<I, typename TypeParam::Type>::type

#define GENTYPE_GET_TUPLE_ELEMENT_POINTEE_TYPE(I) \
  typename std::remove_pointer<typename std::tuple_element<I, \
    typename TypeParam::Type>::type>::type

#define GENTYPE_GET_TUPLE_ELEMENT_BUFFER_TYPE(I) \
  typename std::valarray<typename std::remove_pointer<typename std::tuple_element<I, \
    typename TypeParam::Type>::type>::type> 

#define GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(I)    \
  (OCLTypeTraits<typename std::tuple_element<I, \
   typename TypeParam::Type>::type>::VecStep)

//===----------------------------------------------------------------------===//
/// GENTYPE_CHECK - Type checker for generic type-based tests.
//===----------------------------------------------------------------------===//
#define GENTYPE_CHECK(T) \
  (std::is_same<typename TypeParam::Type, T>::value)

#define GENTYPE_CHECK_POINTEE(T) \
  (std::is_same<typename std::remove_pointer<typename TypeParam::Type>::type, T>::value)

#define GENTYPE_CHECK_TUPLE_ELEMENT(I, T)       \
  (std::is_same<typename std::tuple_element<I,  \
   typename TypeParam::Type>::type, T>::value)

#define GENTYPE_CHECK_TUPLE_ELEMENT_POINTEE(I, T)                           \
  (std::is_same<typename std::remove_pointer<typename std::tuple_element<I, \
   typename TypeParam::Type>::type>::type, T>::value)

#define GENTYPE_CHECK_BASE_VECTOR(T, S) \
  (std::is_same<typename TypeParam::Type, T ## S>::value)

#define GENTYPE_CHECK_TUPLE_ELEMENT_BASE_VECTOR(I, T, S)    \
  (std::is_same<typename std::tuple_element<I,              \
   typename TypeParam::Type>::type, T ## S>::value)

#define GENTYPE_CHECK_POINTEE_BASE_VECTOR(T, S)         \
  (std::is_same<typename std::remove_pointer<typename   \
   TypeParam::Type>::type, T ## S>::value)

#define GENTYPE_CHECK_TUPLE_ELEMENT_POINTEE_BASE_VECTOR(I, T, S)            \
  (std::is_same<typename std::remove_pointer<typename std::tuple_element<I, \
   typename TypeParam::Type>::type>::type, T ## S>::value)

#define GENTYPE_CHECK_TUPLE_ELEMENT_VECTOR(I) \
  (GENTYPE_GET_TUPLE_ELEMENT_VECSTEP(I) > 1)

#define GENTYPE_CHECK_BASE(T)           \
  (GENTYPE_CHECK(T)                 ||  \
   GENTYPE_CHECK_BASE_VECTOR(T, 2)  ||  \
   GENTYPE_CHECK_BASE_VECTOR(T, 3)  ||  \
   GENTYPE_CHECK_BASE_VECTOR(T, 4)  ||  \
   GENTYPE_CHECK_BASE_VECTOR(T, 8)  ||  \
   GENTYPE_CHECK_BASE_VECTOR(T, 16))

#define GENTYPE_CHECK_TUPLE_ELEMENT_BASE(I, T)              \
  (GENTYPE_CHECK_TUPLE_ELEMENT(I, T)                    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_BASE_VECTOR(I, T, 2)     ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_BASE_VECTOR(I, T, 3)     ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_BASE_VECTOR(I, T, 4)     ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_BASE_VECTOR(I, T, 8)     ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_BASE_VECTOR(I, T, 16))

#define GENTYPE_CHECK_POINTEE_BASE(T)           \
  (GENTYPE_CHECK_POINTEE(T)                 ||  \
   GENTYPE_CHECK_POINTEE_BASE_VECTOR(T, 2)  ||  \
   GENTYPE_CHECK_POINTEE_BASE_VECTOR(T, 3)  ||  \
   GENTYPE_CHECK_POINTEE_BASE_VECTOR(T, 4)  ||  \
   GENTYPE_CHECK_POINTEE_BASE_VECTOR(T, 8)  ||  \
   GENTYPE_CHECK_POINTEE_BASE_VECTOR(T, 16))

#define GENTYPE_CHECK_TUPLE_ELEMENT_POINTEE_BASE(I, T)              \
  (GENTYPE_CHECK_TUPLE_ELEMENT_POINTEE(I, T)                    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_POINTEE_BASE_VECTOR(I, T, 2)     ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_POINTEE_BASE_VECTOR(I, T, 3)     ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_POINTEE_BASE_VECTOR(I, T, 4)     ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_POINTEE_BASE_VECTOR(I, T, 8)     ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_POINTEE_BASE_VECTOR(I, T, 16))

#define GENTYPE_CHECK_SIGNED_SCALAR \
  (GENTYPE_CHECK(cl_char)   ||      \
   GENTYPE_CHECK(cl_short)  ||      \
   GENTYPE_CHECK(cl_int)    ||      \
   GENTYPE_CHECK(cl_long))

#define GENTYPE_CHECK_SIGNED_VECTOR(S)  \
  (GENTYPE_CHECK(cl_char ## S)  ||      \
   GENTYPE_CHECK(cl_short ## S) ||      \
   GENTYPE_CHECK(cl_int ## S)   ||      \
   GENTYPE_CHECK(cl_long ## S))

#define GENTYPE_CHECK_SIGNED            \
  (GENTYPE_CHECK_SIGNED_SCALAR      ||  \
   GENTYPE_CHECK_SIGNED_VECTOR(2)   ||  \
   GENTYPE_CHECK_SIGNED_VECTOR(3)   ||  \
   GENTYPE_CHECK_SIGNED_VECTOR(4)   ||  \
   GENTYPE_CHECK_SIGNED_VECTOR(8)   ||  \
   GENTYPE_CHECK_SIGNED_VECTOR(16))

#define GENTYPE_CHECK_UNSIGNED_SCALAR   \
  (GENTYPE_CHECK(cl_uchar)  ||          \
   GENTYPE_CHECK(cl_ushort) ||          \
   GENTYPE_CHECK(cl_uint)   ||          \
   GENTYPE_CHECK(cl_ulong))

#define GENTYPE_CHECK_UNSIGNED_VECTOR(S)    \
  (GENTYPE_CHECK(cl_uchar ## S)     ||      \
   GENTYPE_CHECK(cl_ushort ## S)    ||      \
   GENTYPE_CHECK(cl_uint ## S)      ||      \
   GENTYPE_CHECK(cl_ulong ## S))

#define GENTYPE_CHECK_UNSIGNED          \
  (GENTYPE_CHECK_UNSIGNED_SCALAR    ||  \
   GENTYPE_CHECK_UNSIGNED_VECTOR(2) ||  \
   GENTYPE_CHECK_UNSIGNED_VECTOR(3) ||  \
   GENTYPE_CHECK_UNSIGNED_VECTOR(4) ||  \
   GENTYPE_CHECK_UNSIGNED_VECTOR(8) ||  \
   GENTYPE_CHECK_UNSIGNED_VECTOR(16))

#define GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED_SCALAR(I)    \
  (GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_char)  ||          \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_short) ||          \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_int)   ||          \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_long))

#define GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED_VECTOR(I, S) \
  (GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_char ## S)     ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_short ## S)    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_int ## S)      ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_long ## S))

#define GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED(I)           \
  (GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED_SCALAR(I)     ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED_VECTOR(I, 2)  ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED_VECTOR(I, 3)  ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED_VECTOR(I, 4)  ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED_VECTOR(I, 8)  ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_SIGNED_VECTOR(I, 16))

#define GENTYPE_CHECK_TUPLE_ELEMENT_UNSIGNED_SCALAR(I)  \
  (GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_uchar)     ||      \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_ushort)    ||      \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_uint)      ||      \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_ulong))

#define GENTYPE_CHECK_TUPLE_ELEMENT_UNSIGNED_VECTOR(I, S)   \
  (GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_uchar ## S)    ||      \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_ushort ## S)   ||      \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_uint ## S)     ||      \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_ulong ## S))

#define GENTYPE_CHECK_TUPLE_ELEMENT_UNSIGNED(I)             \
  (GENTYPE_CHECK_TUPLE_ELEMENT_UNSIGNED_SCALAR(I)       ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_UNSIGNED_VECTOR(I, 2)    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_UNSIGNED_VECTOR(I, 3)    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_UNSIGNED_VECTOR(I, 4)    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_UNSIGNED_VECTOR(I, 8)    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT_UNSIGNED_VECTOR(I, 16))

#define GENTYPE_CHECK_FLOAT         \
  (GENTYPE_CHECK(cl_float)      ||  \
   GENTYPE_CHECK(cl_float2)     ||  \
   GENTYPE_CHECK(cl_float3)     ||  \
   GENTYPE_CHECK(cl_float4)     ||  \
   GENTYPE_CHECK(cl_float8)     ||  \
   GENTYPE_CHECK(cl_float16))

#define GENTYPE_CHECK_DOUBLE        \
  (GENTYPE_CHECK(cl_double)      || \
   GENTYPE_CHECK(cl_double2)     || \
   GENTYPE_CHECK(cl_double3)     || \
   GENTYPE_CHECK(cl_double4)     || \
   GENTYPE_CHECK(cl_double8)     || \
   GENTYPE_CHECK(cl_double16))

#define GENTYPE_CHECK_TUPLE_ELEMENT_FLOAT(I)        \
  (GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_float)     ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_float2)    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_float3)    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_float4)    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_float8)    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_float16))

#define GENTYPE_CHECK_TUPLE_ELEMENT_DOUBLE(I)       \
  (GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_double)    ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_double2)   ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_double3)   ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_double4)   ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_double8)   ||  \
   GENTYPE_CHECK_TUPLE_ELEMENT(I, cl_double16))

//===----------------------------------------------------------------------===//
/// ASSERT_GENTYPE_EQ - Equality check for generic type-based tests.
//===----------------------------------------------------------------------===//
#define ASSERT_GENTYPE_EQ(A, B) \
  (OCLTypeTraits<typename TypeParam::Type>::AssertEq(A, B))

#define ASSERT_GENTYPE_BUFFER_EQ(A, B) \
  (OCLTypeTraits<typename std::valarray<typename std::remove_pointer< \
   typename TypeParam::Type>::type>>::AssertEq(A, B))

#define ASSERT_GENTYPE_TUPLE_ELEMENT_EQ(I, A, B) \
  (OCLTypeTraits<typename std::tuple_element<I, \
   typename TypeParam::Type>::type>::AssertEq(A, B))

#define ASSERT_GENTYPE_TUPLE_ELEMENT_BUFFER_EQ(I, A, B) \
  (OCLTypeTraits<typename std::valarray<typename std::remove_pointer< \
   typename std::tuple_element<I, typename TypeParam::Type>::type>::type>>::AssertEq(A, B))

#define ASSERT_GENTYPE_IS_NAN(A) \
  (OCLTypeTraits<typename TypeParam::Type>::AssertIsNaN(A))

#define ASSERT_GENTYPE_TUPLE_ELEMENT_IS_NAN(I, A) \
  (OCLTypeTraits<typename std::tuple_element<I, \
   typename TypeParam::Type>::type>::AssertIsNaN(A))

//===----------------------------------------------------------------------===//
/// Helper macros.
//===----------------------------------------------------------------------===//
#define IS_PTR_TY(T) \
  (OCLTypeTraits<T>::OCLCName.endswith(" *"))

#define IS_INPUT_ARG(A)                                                         \
  ( ArgTys.count(&A) ? ( ArgTys[&A] == KernelArgType::Default            ||     \
                         ArgTys[&A] == KernelArgType::InputBuffer        ||     \
                         ArgTys[&A] == KernelArgType::InputOutputBuffer )       \
                       : true )
  
#define IS_OUTPUT_ARG(A)                                                        \
  ( ArgTys.count(&A) ? ( ArgTys[&A] == KernelArgType::OutputBuffer       ||     \
                         ArgTys[&A] == KernelArgType::InputOutputBuffer )       \
                       : false )

#define ARG_TYPE(A) \
  (ArgTys.count(&A) ? ArgTys[&A] : KernelArgType::Default)

#define INIT_LIST(T, L) \
  std::valarray<T> L

//===----------------------------------------------------------------------===//
/// RuntimeFailed - OpenCL callback to report runtime errors.
//===----------------------------------------------------------------------===//
template <typename DevTy>
void RuntimeFailed(const char *Err,
                   const void *PrivInfo,
                   size_t PrivInfoSize,
                   void *UserData);

//===----------------------------------------------------------------------===//
/// LibraryFixture - Library testing automation fixture. This fixture
///  automatically setup an OpenCL context to run library unit testing for the
///  device specified by the template parameter. In order to execute the test,
///  simply call the Invoke method from inside a TYPED_TEST_P().
//===----------------------------------------------------------------------===//
template <typename DevTy>
class LibraryFixture : public testing::Test {
public:
  void SetUp() {
    // Get all platforms.
    std::vector<cl::Platform> Plats;
    cl::Platform::get(&Plats);

    // Since we link with OpenCRun, the only platform available is OpenCRun.
    Plat = Plats.front();

    // Get all devices.
    std::vector<cl::Device> Devs;
    Plat.getDevices(CL_DEVICE_TYPE_ALL, &Devs);

    // Look for a specific device.
    for(std::vector<cl::Device>::iterator I = Devs.begin(),
                                          E = Devs.end();
                                          I != E && !Dev();
                                          ++I)
      if(DeviceTraits<DevTy>::Name == I->getInfo<CL_DEVICE_NAME>())
        Dev = cl::Device(*I);

    ASSERT_TRUE(Dev());
    
    // Get a queue for the device.
    cl_context_properties Props[] =
      { CL_CONTEXT_PLATFORM,
        reinterpret_cast<cl_context_properties>(Plat()),
        0
      };
    Ctx = cl::Context(std::vector<cl::Device>(1, Dev),
                      Props,
                      ::RuntimeFailed<DevTy>,
                      this);
    Queue = cl::CommandQueue(Ctx, Dev);
  }

  void TearDown() {
    // OpenCL resources released automatically by the destructor, via
    // cl:{Platform,Device,Context,CommandQueue} destructors.
  }

  template <typename RetTy>
  void Invoke(llvm::StringRef Fun,
              RetTy &R,
              cl::NDRange GlobalSpace = cl::NDRange(1),
              cl::NDRange LocalSpace = cl::NDRange(1)) {
    cl::Buffer RetBuf = AllocReturnBuffer<RetTy>();

    cl::Kernel Kern = BuildKernel<RetTy>(Fun);
    Kern.setArg(0, RetBuf);

    Queue.enqueueNDRangeKernel(Kern, cl::NullRange, GlobalSpace, LocalSpace);
    Queue.enqueueReadBuffer(RetBuf, true, 0, sizeof(RetTy), &R);
  }

  template <typename RetTy, typename A1Ty>
  void Invoke(llvm::StringRef Fun,
              RetTy &R,
              A1Ty &A1,
              cl::NDRange GlobalSpace = cl::NDRange(1),
              cl::NDRange LocalSpace = cl::NDRange(1)) {
    cl::Buffer RetBuf = AllocReturnBuffer<RetTy>();
    cl::Buffer A1Buf = AllocArgBuffer<>(A1, ARG_TYPE(A1));

    cl::Kernel Kern = BuildKernel<RetTy, A1Ty>(Fun);
    Kern.setArg(0, RetBuf);
    Kern.setArg(1, A1Buf);

    if (IS_INPUT_ARG(A1)) WriteBuffer(A1Buf, A1);

    Queue.enqueueNDRangeKernel(Kern, cl::NullRange, GlobalSpace, LocalSpace);

    if (IS_OUTPUT_ARG(A1)) ReadBuffer(A1Buf, A1);
    ReadBuffer(RetBuf, R);
  }

  template <typename RetTy, typename A1Ty, typename A2Ty>
  void Invoke(llvm::StringRef Fun,
              RetTy &R,
              A1Ty &A1,
              A2Ty &A2,
              cl::NDRange GlobalSpace = cl::NDRange(1),
              cl::NDRange LocalSpace = cl::NDRange(1)) {
    cl::Buffer RetBuf = AllocReturnBuffer<RetTy>();
    cl::Buffer A1Buf = AllocArgBuffer<>(A1, ARG_TYPE(A1));
    cl::Buffer A2Buf = AllocArgBuffer<>(A2, ARG_TYPE(A2));

    cl::Kernel Kern = BuildKernel<RetTy, A1Ty, A2Ty>(Fun);
    Kern.setArg(0, RetBuf);
    Kern.setArg(1, A1Buf);
    Kern.setArg(2, A2Buf);
   
    if (IS_INPUT_ARG(A1)) WriteBuffer(A1Buf, A1);
    if (IS_INPUT_ARG(A2)) WriteBuffer(A2Buf, A2);

    Queue.enqueueNDRangeKernel(Kern, cl::NullRange, GlobalSpace, LocalSpace);

    if (IS_OUTPUT_ARG(A1)) ReadBuffer(A1Buf, A1);
    if (IS_OUTPUT_ARG(A2)) ReadBuffer(A2Buf, A2);
    ReadBuffer(RetBuf, R);
  }

  template <typename RetTy, typename A1Ty, typename A2Ty, typename A3Ty>
  void Invoke(llvm::StringRef Fun,
              RetTy &R,
              A1Ty &A1,
              A2Ty &A2,
              A3Ty &A3,
              cl::NDRange GlobalSpace = cl::NDRange(1),
              cl::NDRange LocalSpace = cl::NDRange(1)) {
    cl::Buffer RetBuf = AllocReturnBuffer<RetTy>();
    cl::Buffer A1Buf = AllocArgBuffer<>(A1, ARG_TYPE(A1));
    cl::Buffer A2Buf = AllocArgBuffer<>(A2, ARG_TYPE(A2));
    cl::Buffer A3Buf = AllocArgBuffer<>(A3, ARG_TYPE(A3));

    cl::Kernel Kern = BuildKernel<RetTy, A1Ty, A2Ty, A3Ty>(Fun);
    Kern.setArg(0, RetBuf);
    Kern.setArg(1, A1Buf);
    Kern.setArg(2, A2Buf);
    Kern.setArg(3, A3Buf);

    if (IS_INPUT_ARG(A1)) WriteBuffer(A1Buf, A1);
    if (IS_INPUT_ARG(A2)) WriteBuffer(A2Buf, A2);
    if (IS_INPUT_ARG(A3)) WriteBuffer(A3Buf, A3);

    Queue.enqueueNDRangeKernel(Kern, cl::NullRange, GlobalSpace, LocalSpace);

    if (IS_OUTPUT_ARG(A1)) ReadBuffer(A1Buf, A1);
    if (IS_OUTPUT_ARG(A2)) ReadBuffer(A2Buf, A2);
    if (IS_OUTPUT_ARG(A3)) ReadBuffer(A3Buf, A3);
    ReadBuffer(RetBuf, R);
  }

  template <typename A1Ty, typename A2Ty, typename A3Ty>
  void VoidRetTyInvoke(llvm::StringRef Fun,
              A1Ty &A1,
              A2Ty &A2,
              A3Ty &A3,
              cl::NDRange GlobalSpace = cl::NDRange(1),
              cl::NDRange LocalSpace = cl::NDRange(1)) {
    cl::Buffer A1Buf = AllocArgBuffer<>(A1, ARG_TYPE(A1));
    cl::Buffer A2Buf = AllocArgBuffer<>(A2, ARG_TYPE(A2));
    cl::Buffer A3Buf = AllocArgBuffer<>(A3, ARG_TYPE(A3));

    cl::Kernel Kern = BuildVoidRetTyKernel<A1Ty, A2Ty, A3Ty>(Fun);
    Kern.setArg(0, A1Buf);
    Kern.setArg(1, A2Buf);
    Kern.setArg(2, A3Buf);

    if (IS_INPUT_ARG(A1)) WriteBuffer(A1Buf, A1);
    if (IS_INPUT_ARG(A2)) WriteBuffer(A2Buf, A2);
    if (IS_INPUT_ARG(A3)) WriteBuffer(A3Buf, A3);

    Queue.enqueueNDRangeKernel(Kern, cl::NullRange, GlobalSpace, LocalSpace);

    if (IS_OUTPUT_ARG(A1)) ReadBuffer(A1Buf, A1);
    if (IS_OUTPUT_ARG(A2)) ReadBuffer(A2Buf, A2);
    if (IS_OUTPUT_ARG(A3)) ReadBuffer(A3Buf, A3);
  }

protected:
  template <typename RetTy>
  cl::Kernel BuildKernel(llvm::StringRef Fun) {
    std::string KernName;
    BuildKernelName(Fun, KernName);

    bool EnableFP64 = (OCLTypeTraits<RetTy>::OCLCName.startswith("double"));

    std::ostringstream SrcStream;

    SrcStream << (EnableFP64 ? "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n\n" : "") 
              << "kernel void " << KernName
              << "(\n"
              << "  global " << OCLTypeTraits<RetTy>::OCLCName.str() << " *r\n"
              << ")\n"
              << "{\n"
              << "  *r = " << Fun.str() << "();\n"
              << "}\n";
    std::string Src = SrcStream.str();

    return GetKernel(KernName, Src);
  }

  template <typename RetTy, typename A1Ty>
  cl::Kernel BuildKernel(llvm::StringRef Fun) {
    std::string KernName;
    BuildKernelName(Fun, KernName);

    bool EnableFP64 = (OCLTypeTraits<RetTy>::OCLCName.startswith("double") ||
                       OCLTypeTraits<A1Ty>::OCLCName.startswith("double"));

    std::ostringstream SrcStream;

    SrcStream << (EnableFP64 ? "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n\n" : "") 
              << "kernel void " << KernName
              << "(\n"
              << "  global " << OCLTypeTraits<RetTy>::OCLCName.str() << " *r,\n"
              << "  global " << OCLTypeTraits<A1Ty>::OCLCName.str()
                << (IS_PTR_TY(A1Ty) ? "a1" : "*a1") << "\n"
              << ")\n"
              << "{\n"
              << "  *r = " << Fun.str() << "("
                << (IS_PTR_TY(A1Ty) ? "a1" : "*a1")
                << ");\n"
              << "}\n";
    std::string Src = SrcStream.str();

    return GetKernel(KernName, Src);
  }

  template <typename RetTy, typename A1Ty, typename A2Ty>
  cl::Kernel BuildKernel(llvm::StringRef Fun) {
    std::string KernName;
    BuildKernelName(Fun, KernName);

    bool EnableFP64 = (OCLTypeTraits<RetTy>::OCLCName.startswith("double") ||
                       OCLTypeTraits<A1Ty>::OCLCName.startswith("double") ||
                       OCLTypeTraits<A2Ty>::OCLCName.startswith("double"));

    std::ostringstream SrcStream;

    SrcStream << (EnableFP64 ? "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n\n" : "") 
              << "kernel void " << KernName
              << "(\n"
              << "  global " << OCLTypeTraits<RetTy>::OCLCName.str() << " *r,\n"
              << "  global " << OCLTypeTraits<A1Ty>::OCLCName.str()
                << (IS_PTR_TY(A1Ty) ? "a1" : " *a1") << ",\n"
              << "  global " << OCLTypeTraits<A2Ty>::OCLCName.str()
                << (IS_PTR_TY(A2Ty) ? "a2" : " *a2") << "\n"
              << ")\n"
              << "{\n"
              << "  *r = " << Fun.str() << "("
                << (IS_PTR_TY(A1Ty) ? "a1" : "*a1") << ", "
                << (IS_PTR_TY(A2Ty) ? "a2" : "*a2") 
                << ");\n"
              << "}\n";
    std::string Src = SrcStream.str();

    return GetKernel(KernName, Src);
  }

  template <typename RetTy, typename A1Ty, typename A2Ty, typename A3Ty>
  cl::Kernel BuildKernel(llvm::StringRef Fun) {
    std::string KernName;
    BuildKernelName(Fun, KernName);
     
    bool EnableFP64 = (OCLTypeTraits<RetTy>::OCLCName.startswith("double") ||
                       OCLTypeTraits<A1Ty>::OCLCName.startswith("double") ||
                       OCLTypeTraits<A2Ty>::OCLCName.startswith("double") ||
                       OCLTypeTraits<A3Ty>::OCLCName.startswith("double"));

    std::ostringstream SrcStream;

    SrcStream << (EnableFP64 ? "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n\n" : "") 
              << "kernel void " << KernName
              << "(\n"
              << "  global " << OCLTypeTraits<RetTy>::OCLCName.str() << " *r,\n"
              << "  global " << OCLTypeTraits<A1Ty>::OCLCName.str()
                << (IS_PTR_TY(A1Ty) ? "a1" : " *a1") << ",\n"
              << "  global " << OCLTypeTraits<A2Ty>::OCLCName.str()
                << (IS_PTR_TY(A2Ty) ? "a2" : " *a2") << ",\n"
              << "  global " << OCLTypeTraits<A3Ty>::OCLCName.str()
                << (IS_PTR_TY(A3Ty) ? "a3" : " *a3") << "\n"
              << ")\n"
              << "{\n"
              << "  *r = " << Fun.str() << "("
                << (IS_PTR_TY(A1Ty) ? "a1" : "*a1") << ", "
                << (IS_PTR_TY(A2Ty) ? "a2" : "*a2") << ", "
                << (IS_PTR_TY(A3Ty) ? "a3" : "*a3")
                << ");\n"
              << "}\n";
    std::string Src = SrcStream.str();

    return GetKernel(KernName, Src);
  }

  template <typename A1Ty, typename A2Ty, typename A3Ty>
  cl::Kernel BuildVoidRetTyKernel(llvm::StringRef Fun) {
    std::string KernName;
    BuildKernelName(Fun, KernName);
     
    bool EnableFP64 = (OCLTypeTraits<A1Ty>::OCLCName.startswith("double") ||
                       OCLTypeTraits<A2Ty>::OCLCName.startswith("double") ||
                       OCLTypeTraits<A3Ty>::OCLCName.startswith("double"));

    std::ostringstream SrcStream;

    SrcStream << (EnableFP64 ? "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n\n" : "") 
              << "kernel void " << KernName
              << "(\n"
              << "  global " << OCLTypeTraits<A1Ty>::OCLCName.str()
                << (IS_PTR_TY(A1Ty) ? "a1" : " *a1") << ",\n"
              << "  global " << OCLTypeTraits<A2Ty>::OCLCName.str()
                << (IS_PTR_TY(A2Ty) ? "a2" : " *a2") << ",\n"
              << "  global " << OCLTypeTraits<A3Ty>::OCLCName.str()
                << (IS_PTR_TY(A3Ty) ? "a3" : " *a3") << "\n"
              << ")\n"
              << "{\n"
              << Fun.str() << "("
                << (IS_PTR_TY(A1Ty) ? "a1" : "*a1") << ", "
                << (IS_PTR_TY(A2Ty) ? "a2" : "*a2") << ", "
                << (IS_PTR_TY(A3Ty) ? "a3" : "*a3")
                << ");\n"
              << "}\n";
    std::string Src = SrcStream.str();

    return GetKernel(KernName, Src);
  }

  template <typename Ty>
  cl::Buffer AllocReturnBuffer(Ty T = Ty()) {
    return cl::Buffer(Ctx, CL_MEM_WRITE_ONLY, sizeof(Ty));
  }

  template <typename Ty>
  cl::Buffer AllocReturnBuffer(std::valarray<Ty> &T) {
    return cl::Buffer(Ctx, CL_MEM_WRITE_ONLY, T.size() * sizeof(Ty));
  }

  template <typename Ty>
  cl::Buffer AllocArgBuffer(Ty T, KernelArgType ArgTy = KernelArgType::Default) {
    switch (ArgTy) {
    case KernelArgType::OutputBuffer:
      return cl::Buffer(Ctx, CL_MEM_WRITE_ONLY, sizeof(Ty));
    case KernelArgType::InputOutputBuffer:
      return cl::Buffer(Ctx, CL_MEM_READ_WRITE, sizeof(Ty));
    case KernelArgType::InputBuffer:
    case KernelArgType::Default:
    default:
      return cl::Buffer(Ctx, CL_MEM_READ_ONLY, sizeof(Ty));
    }
  }

  template <typename Ty>
  cl::Buffer AllocArgBuffer(std::valarray<Ty> &T, KernelArgType ArgTy = KernelArgType::Default) {
    switch (ArgTy) {
    case KernelArgType::OutputBuffer:
      return cl::Buffer(Ctx, CL_MEM_WRITE_ONLY, T.size() * sizeof(Ty));
    case KernelArgType::InputOutputBuffer:
      return cl::Buffer(Ctx, CL_MEM_READ_WRITE, T.size() * sizeof(Ty));
    case KernelArgType::InputBuffer:
    case KernelArgType::Default:
    default:
      return cl::Buffer(Ctx, CL_MEM_READ_ONLY, T.size() * sizeof(Ty));
    }
  }

  template <typename Ty>
  cl_int WriteBuffer(cl::Buffer &Buffer, Ty T) {
    return Queue.enqueueWriteBuffer(Buffer, true, 0, sizeof(Ty), &T);
  }

  template <typename Ty>
  cl_int WriteBuffer(cl::Buffer &Buffer, std::valarray<Ty> &T) {
    return Queue.enqueueWriteBuffer(Buffer, true, 0, T.size() * sizeof(Ty), &T[0]);
  }

  template <typename Ty>
  cl_int ReadBuffer(cl::Buffer &Buffer, Ty &T) {
    return Queue.enqueueReadBuffer(Buffer, true, 0, sizeof(Ty), &T);
  }

  template <typename Ty>
  cl_int ReadBuffer(cl::Buffer &Buffer, std::valarray<Ty> &T) {
    return Queue.enqueueReadBuffer(Buffer, true, 0, T.size() * sizeof(Ty), &T[0]);
  }

  void BuildKernelName(llvm::StringRef Fun, std::string &KernName) {
    KernName = Fun.str() + "_test";
  }

  cl::Kernel GetKernel(const std::string &Name, const std::string &Src) {
    cl::Program::Sources Srcs;
    Srcs.push_back(std::make_pair(Src.c_str(), 0));

    cl::Program Prog(Ctx, Srcs);
    Prog.build(std::vector<cl::Device>());

    return cl::Kernel(Prog, Name.c_str());
  }

  void SetKernelArgType(KernelArgAddr ArgAddr, KernelArgType ArgTy) { ArgTys[ArgAddr] = ArgTy; }

  cl::Platform GetPlatform() { return Plat; }
  cl::Device GetDevice() { return Dev; }
  cl::Context GetContext() { return Ctx; }
  cl::CommandQueue GetQueue() { return Queue; }

private:
  void RuntimeFailed(llvm::StringRef Err) {
    FAIL() << Err.str() << "\n";
  }

  std::map<KernelArgAddr, KernelArgType> ArgTys;

protected:
  cl::Platform Plat;
  cl::Device Dev;
  cl::Context Ctx;
  cl::CommandQueue Queue;

  friend void ::RuntimeFailed<DevTy>(const char *,
                                     const void *,
                                     size_t,
                                     void *);
};

template <typename DevTy, typename Ty>
class LibraryGenTypeFixture : public LibraryFixture<DevTy> { };

class CPUDev { };

template <>
class DeviceTraits<CPUDev> {
public:
  #if defined(__x86_64__)
  #define cl_khr_fp64
  #define cl_khr_int64_base_atomics
  #define cl_khr_int64_extended_atomics
  typedef uint64_t SizeType;
  #elif defined(__i386__)
  #define cl_khr_fp64
  typedef uint32_t SizeType;
  #else
  #error "architecture not supported"
  #endif

public:
  static llvm::StringRef Name;
};

template <typename DevTy>
void RuntimeFailed(const char *Err,
                   const void *PrivInfo,
                   size_t PrivInfoSize,
                   void *UserData) {
  LibraryFixture<DevTy> *Fix = static_cast<LibraryFixture<DevTy> *>(UserData);

  Fix->RuntimeFailed(Err);
}

#endif // UNITTESTS_LIBRARY_LIBRARYFIXTURE_H
