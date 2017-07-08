
#include "LibraryFixture.h"

// ---------------------------------------------------------------------------------------- //

template <typename DevTy>
class ExplicitMemFenceFunctions_TestCase_1 : public LibraryFixture<DevTy> { };

#define ALL_DEVICE_TYPES \
  CPUDev

typedef testing::Types<ALL_DEVICE_TYPES> OCLDevicesType;

TYPED_TEST_CASE_P(ExplicitMemFenceFunctions_TestCase_1);

/*  Function        |   Type Patterns (See OpenCL.td)
 * ----------------------------------------------------------------
 *  mem_fence       |   vCUn
 *  read_mem_fence  |   vCUn
 *  write_mem_fence |   vCUn
 *
 * Given that the read_mem_fence and write_mem_fence functions implement an
 * acquire and a release fence respectively, while the mem_fence function
 * represents a full fence, synchronization between concurrent work-items
 * that shar a resource can be achieved as follows:
 * 
 * "A release fence A synchronizes with an acquire fence B if there exist
 * atomic operations X and Y, both operating on some atomic object M, such
 * that A is sequenced before X, X modifies M, Y is sequenced before B, and
 * Y reads the value written by X or a value written by any side effect in
 * the hypothetical release sequence X would head if it were a release
 * operation."                                          
 *                                                      [[ Working Draft,
 *                                  Standard for Programming Language C++ ]]
 *
 * All the atomic buil-in functions defined in the Atomic.td TableGen source
 * file specify a relaxed memory model (__ATOMIC_RELAXED) and, thus, they
 * don't force any ordering.
 * 
 * For more informations:
 *
 * http://preshing.com/20120913/acquire-and-release-semantics/
 * http://preshing.com/20130823/the-synchronizes-with-relation/
 * http://preshing.com/20130922/acquire-and-release-fences/
 *
 */

TYPED_TEST_P(ExplicitMemFenceFunctions_TestCase_1, mem_fence) {
  cl_uint HostIn = 0;
  cl_uint HostOut = 0;

  cl::Buffer In = this->AllocArgBuffer(HostIn);
  cl::Buffer Out = this->AllocReturnBuffer(sizeof(cl_uint));


  const char *Src = "kernel void mem_fence_test(global uint *out,          \n"
                    "                           global uint *shared_data,  \n"
                    "                           local  uint *guard) {      \n"
                    "  uint id = get_global_id(0);                         \n"
                    "  atomic_xor(guard, *guard);                          \n"
                    "                                                      \n"
                    "  if (id == 0) {                                      \n"
                    "    *shared_data += 1;                                \n"
                    "    mem_fence(CLK_LOCAL_MEM_FENCE);                   \n"
                    "    atomic_inc(guard);                                \n"
                    "  }                                                   \n"
                    "                                                      \n"
                    "  if (id == 1) {                                      \n"
                    "    while (atomic_cmpxchg(guard, 1, 1))               \n"
                    "     ;                                                \n"
                    "                                                      \n"
                    "    mem_fence(CLK_LOCAL_MEM_FENCE);                   \n"
                    "    *out = ++(*shared_data);                          \n"
                    "  }                                                   \n"
                    "}                                                     \n";

  cl::Kernel Kern = this->GetKernel("mem_fence_test", Src);

  Kern.setArg(0, Out);
  Kern.setArg(1, In);
  Kern.setArg(2, cl::Local(sizeof(cl_uint)));

  cl::CommandQueue Queue = this->GetQueue();
  Queue.enqueueWriteBuffer(In, true, 0, sizeof(cl_uint), &HostIn);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             cl::NDRange(2),
                             cl::NDRange(2));
  Queue.enqueueReadBuffer(Out, true, 0, sizeof(cl_uint), &HostOut);

  EXPECT_EQ(2U, HostOut);
}

TYPED_TEST_P(ExplicitMemFenceFunctions_TestCase_1, read_write_mem_fence) {
  cl_uint HostIn = 0;
  cl_uint HostOut = 0;

  cl::Buffer In = this->AllocArgBuffer(HostIn);
  cl::Buffer Out = this->AllocReturnBuffer(sizeof(cl_uint));


  const char *Src = "kernel void mem_fence_test(global uint *out,          \n"
                    "                           global uint *shared_data,  \n"
                    "                           local  uint *guard) {      \n"
                    "  uint id = get_global_id(0);                         \n"
                    "  atomic_xor(guard, *guard);                          \n"
                    "                                                      \n"
                    "  if (id == 0) {                                      \n"
                    "    *shared_data += 1;                                \n"
                    "    write_mem_fence(CLK_LOCAL_MEM_FENCE);             \n"
                    "    atomic_inc(guard);                                \n"
                    "  }                                                   \n"
                    "                                                      \n"
                    "  if (id == 1) {                                      \n"
                    "    while (atomic_cmpxchg(guard, 1, 1))               \n"
                    "     ;                                                \n"
                    "                                                      \n"
                    "    read_mem_fence(CLK_LOCAL_MEM_FENCE);              \n"
                    "    *out = ++(*shared_data);                          \n"
                    "  }                                                   \n"
                    "}                                                     \n";

  cl::Kernel Kern = this->GetKernel("mem_fence_test", Src);

  Kern.setArg(0, Out);
  Kern.setArg(1, In);
  Kern.setArg(2, cl::Local(sizeof(cl_uint)));

  cl::CommandQueue Queue = this->GetQueue();
  Queue.enqueueWriteBuffer(In, true, 0, sizeof(cl_uint), &HostIn);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             cl::NDRange(2),
                             cl::NDRange(2));
  Queue.enqueueReadBuffer(Out, true, 0, sizeof(cl_uint), &HostOut);

  EXPECT_EQ(2U, HostOut);
}

REGISTER_TYPED_TEST_CASE_P(ExplicitMemFenceFunctions_TestCase_1, mem_fence,
                                                                 read_write_mem_fence);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, ExplicitMemFenceFunctions_TestCase_1, OCLDevicesType);
