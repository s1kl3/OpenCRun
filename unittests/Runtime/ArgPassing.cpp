
#include "RuntimeFixture.h"

template <typename DevTy>
class ArgPassingTest : public RuntimeFixture<DevTy> { };

#define ALL_DEVICE_TYPES \
  CPUDev

typedef testing::Types<ALL_DEVICE_TYPES> OCLDevicesType;

TYPED_TEST_CASE_P(ArgPassingTest);

TYPED_TEST_P(ArgPassingTest, Buffers) {
  cl_uint HostOut = 0, HostIn = 7;

  cl::Buffer Out = this->AllocReturnBuffer(sizeof(cl_uint));
  cl::Buffer In = this->AllocArgBuffer(sizeof(cl_uint));

  const char *Src = "kernel void copy(global uint *out, \n"
                    "                 global uint *in) {\n"
                    "  *out = *in;                      \n"
                    "}                                  \n";
  cl::Kernel Kern = this->BuildKernel("copy", Src);

  Kern.setArg(0, Out);
  Kern.setArg(1, In);

  cl::CommandQueue Queue = this->GetQueue();

  Queue.enqueueWriteBuffer(In, true, 0, sizeof(cl_uint), &HostIn);
  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             cl::NDRange(1),
                             cl::NDRange(1));
  Queue.enqueueReadBuffer(Out, true, 0, sizeof(cl_uint), &HostOut);

  EXPECT_EQ(7u, HostOut);
}

TYPED_TEST_P(ArgPassingTest, ByValue) {
  cl_uint HostOut = 0;

  cl::Buffer Out = this->AllocReturnBuffer(sizeof(cl_uint));

  const char *Src = "kernel void copy(global uint *out,\n"
                    "                 uint in) {       \n"
                    " *out = in;                       \n"
                    "}                                 \n";
  cl::Kernel Kern = this->BuildKernel("copy", Src);

  Kern.setArg(0, Out);
  Kern.setArg(1, 7);

  cl::CommandQueue Queue = this->GetQueue();

  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             cl::NDRange(1),
                             cl::NDRange(1));
  Queue.enqueueReadBuffer(Out, true, 0, sizeof(cl_uint), &HostOut);

  EXPECT_EQ(7u, HostOut);
}

TYPED_TEST_P(ArgPassingTest, Vector) {
  cl_int3 HostOut, HostIn;

  HostOut.s[0] = HostOut.s[1] = HostOut.s[2] = HostOut.s[3] = 0;
  HostIn.s[0] = -1;
  HostIn.s[1] = 0;
  HostIn.s[2] = +1;

  cl::Buffer Out = this->AllocReturnBuffer(sizeof(cl_int3));

  const char *Src = "kernel void copy(global int3 *out,\n"
                    "                 int3 in) {       \n"
                    " *out = in;                       \n"
                    "}                                 \n";
  cl::Kernel Kern = this->BuildKernel("copy", Src);

  Kern.setArg(0, Out);
  Kern.setArg(1, HostIn);

  cl::CommandQueue Queue = this->GetQueue();

  Queue.enqueueNDRangeKernel(Kern,
                             cl::NullRange,
                             cl::NDRange(1),
                             cl::NDRange(1));
  Queue.enqueueReadBuffer(Out, true, 0, sizeof(cl_int3), &HostOut);

  EXPECT_EQ(-1, HostOut.s[0]);
  EXPECT_EQ(0, HostOut.s[1]);
  EXPECT_EQ(+1, HostOut.s[2]);
}

REGISTER_TYPED_TEST_CASE_P(ArgPassingTest, Buffers,
                                           ByValue,
                                           Vector);

INSTANTIATE_TYPED_TEST_CASE_P(OCLDev, ArgPassingTest, OCLDevicesType);
