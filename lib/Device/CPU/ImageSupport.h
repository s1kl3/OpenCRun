#ifndef IMAGESUPPORT_H
#define IMAGESUPPORT_H

#include "CL/cl.h"

namespace opencrun {

class Sampler;

namespace cpu {

//// WARNING: These definition must match the one in Library/Image.td.

struct cpu_image_t {
  cl_uint image_channel_order;
  cl_uint image_channel_data_type;
  cl_uint num_channels;
  cl_uint element_size;

  cl_uint width;
  cl_uint height;
  cl_uint depth;

  cl_uint row_pitch;
  cl_uint slice_pitch;

  cl_uint array_size;

  cl_uint num_mip_levels;
  cl_uint num_samples;

  void *data; // Pointer to actual image data in __global AS.
};

#ifdef CLANG_GT_4
// Since Clang 4, sampler types are represented as opaque pointer types (i.e.
// %opencl.sampler_t*) instead of the previously used i32. 
using cpu_sampler_t = cl_uint *;
#else
using cpu_sampler_t = cl_uint;
#endif

// Initializer for function and program scope samplers.
cpu_sampler_t TranslateSamplerInitializer(unsigned InitVal);

cpu_sampler_t getCPUSampler(const Sampler *Smplr);

}
}

#endif
