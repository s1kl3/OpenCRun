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

using cpu_sampler_t = cl_uint;

cpu_sampler_t getCPUSampler(const Sampler *Smplr);

}
}

#endif
