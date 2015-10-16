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

cpu_sampler_t opencrun::cpu::getCPUSampler(const Sampler *Smplr) {
  if (!Smplr)
    return 0;

  cpu_sampler_t flags = 0;

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

  return flags;
}
