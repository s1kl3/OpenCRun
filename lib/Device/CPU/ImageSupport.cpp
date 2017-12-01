#include "ImageSupport.h"

#include "opencrun/Core/Context.h"
#include "opencrun/Core/Sampler.h"

using namespace opencrun;
using namespace opencrun::cpu;

#define CLK_NORMALIZED_COORDS_FALSE 0x0000
#define CLK_NORMALIZED_COORDS_TRUE  0x0008

#define CLK_ADDRESS_NONE            0x0000
#define CLK_ADDRESS_MIRRORED_REPEAT 0x0001
#define CLK_ADDRESS_REPEAT          0x0002
#define CLK_ADDRESS_CLAMP_TO_EDGE   0x0003
#define CLK_ADDRESS_CLAMP           0x0004

#define CLK_FILTER_NEAREST          0x0000
#define CLK_FILTER_LINEAR           0x0010

#define CLK_R                       0x10B0
#define CLK_A                       0x10B1
#define CLK_RG                      0x10B2
#define CLK_RA                      0x10B3
#define CLK_RGB                     0x10B4
#define CLK_RGBA                    0x10B5
#define CLK_BGRA                    0x10B6
#define CLK_ARGB                    0x10B7
#define CLK_INTENSITY               0x10B8
#define CLK_LUMINANCE               0x10B9
#define CLK_Rx                      0x10BA
#define CLK_RGx                     0x10BB
#define CLK_RGBx                    0x10BC
#define CLK_DEPTH                   0x10BD
#define CLK_DEPTH_STENCIL           0x10BE

#define CLK_SNORM_INT8              0x10D0
#define CLK_SNORM_INT16             0x10D1
#define CLK_UNORM_INT8              0x10D2
#define CLK_UNORM_INT16             0x10D3
#define CLK_UNORM_SHORT_565         0x10D4
#define CLK_UNORM_SHORT_555         0x10D5
#define CLK_UNORM_INT_101010        0x10D6
#define CLK_SIGNED_INT8             0x10D7
#define CLK_SIGNED_INT16            0x10D8
#define CLK_SIGNED_INT32            0x10D9
#define CLK_UNSIGNED_INT8           0x10DA
#define CLK_UNSIGNED_INT16          0x10DB
#define CLK_UNSIGNED_INT32          0x10DC
#define CLK_HALF_FLOAT              0x10DD
#define CLK_FLOAT                   0x10DE
#define CLK_UNORM_INT24             0x10DF

#ifdef CLANG_GT_4
// The __translate_sampler_initializer function definition is trivial for the
// CPU target since, in this case, sampler_t is defined as an i32*.
// The function would simply need to copy its literal argument inside a 4 byte
// memory area whose address would be returned to the caller. However, in order
// to avoid any dynamic memory allocation, a statically allocated look-up table
// seems the perfect choice. The function argument is taken as the index of the
// array containing all the admitted values for the literal itself (i.e. the
// precomputed function results).
static cl_uint Sampler_LUT[32] = {
  0x00, 0x01, 0x02, 0x03, 0x04, /* Unused -> */ 0x05, 0x06, 0x07, /* <- */
  0x08, 0x09, 0x0A, 0x0B, 0x0C, /* Unused -> */ 0x0D, 0x0E, 0x0F, /* <- */
  0x10, 0x11, 0x12, 0x13, 0x14, /* Unused -> */ 0x15, 0x16, 0x17, /* <- */
  0x18, 0x19, 0x1A, 0x1B, 0x1C, /* Unused -> */ 0x1D, 0x1E, 0x1F  /* <- */
};

cpu_sampler_t opencrun::cpu::TranslateSamplerInitializer(unsigned InitVal) {
  // The LUT i8 value is expliticly casted back to an i32.
  return &Sampler_LUT[InitVal];
}
#endif

cpu_sampler_t opencrun::cpu::getCPUSampler(const Sampler *Smplr) {
  if (!Smplr)
    return 0;

#ifdef CLANG_GT_4
  unsigned flags = 0;
#else
  cpu_sampler_t flags = 0; 
#endif

  if (Smplr->GetNormalizedCoords()) {
    flags |= CLK_NORMALIZED_COORDS_TRUE;
  } else {
    flags |= CLK_NORMALIZED_COORDS_FALSE;
  }

  switch (Smplr->GetAddressingMode()) {
  case Sampler::AddressNone:
    flags |= CLK_ADDRESS_NONE;
    break;
  case Sampler::AddressClampToEdge:
    flags |= CLK_ADDRESS_CLAMP_TO_EDGE;
    break;
  case Sampler::AddressClamp:
    flags |= CLK_ADDRESS_CLAMP;
    break;
  case Sampler::AddressRepeat:
    flags |= CLK_ADDRESS_REPEAT;
    break;
  case Sampler::AddressMirroredRepeat:
    flags |= CLK_ADDRESS_MIRRORED_REPEAT;
    break;
  case Sampler::NoAddressing:
    break;
  }

  switch (Smplr->GetFilterMode()) {
  case Sampler::FilterNearest:
    flags |= CLK_FILTER_NEAREST;
    break;
  case Sampler::FilterLinear:
    flags |= CLK_FILTER_LINEAR;
    break;
  case Sampler::NoFilter:
    break;
  }

#ifdef CLANG_GT_4
  return TranslateSamplerInitializer(flags);
#else
  return flags;
#endif
}
