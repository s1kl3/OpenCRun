
//
// The Image library is auto-generated.
//

#define OPENCRUN_BUILTIN_IMAGE 
#include "Builtins.CPU.inc"

__opencrun_overload
float4 read_imagef(read_only image1d_t image, sampler_t sampler, float coord) {
  device_image_t *dev_image = __builtin_astype(image, device_image_t *);
  device_sampler_t dev_sampler = *(__builtin_astype(sampler, device_sampler_t *));

  if(dev_sampler & CLK_FILTER_LINEAR)
    return apply_filter_linear(dev_image, dev_sampler, map_to_float4(coord), 1);
  else {
    int4 coord_4 = apply_filter_nearest(dev_image, dev_sampler, map_to_float4(coord));
    return read_pixelf(dev_image, coord_4);
  }
}

__opencrun_overload
float4 read_imagef(read_only image1d_array_t image, sampler_t sampler, float2 coord) {
  device_image_t *dev_image = __builtin_astype(image, device_image_t *);
  device_sampler_t dev_sampler = *(__builtin_astype(sampler, device_sampler_t *));

  if(dev_sampler & CLK_FILTER_LINEAR)
    return apply_filter_linear(dev_image, dev_sampler, map_to_float4(coord), 2);
  else {
    int4 coord_4 = apply_filter_nearest(dev_image, dev_sampler, map_to_float4(coord));
    return read_pixelf(dev_image, coord_4);
  }
}

__opencrun_overload
float4 read_imagef(read_only image2d_t image, sampler_t sampler, float2 coord) {
  device_image_t *dev_image = __builtin_astype(image, device_image_t *);
  device_sampler_t dev_sampler = *(__builtin_astype(sampler, device_sampler_t *));

  if(dev_sampler & CLK_FILTER_LINEAR)
    return apply_filter_linear(dev_image, dev_sampler, map_to_float4(coord), 2);
  else {
    int4 coord_4 = apply_filter_nearest(dev_image, dev_sampler, map_to_float4(coord));
    return read_pixelf(dev_image, coord_4);
  }
}

__opencrun_overload
float4 read_imagef(read_only image2d_array_t image, sampler_t sampler, float4 coord) {
  device_image_t *dev_image = __builtin_astype(image, device_image_t *);
  device_sampler_t dev_sampler = *(__builtin_astype(sampler, device_sampler_t *));

  if(dev_sampler & CLK_FILTER_LINEAR)
    return apply_filter_linear(dev_image, dev_sampler, map_to_float4(coord), 3);
  else {
    int4 coord_4 = apply_filter_nearest(dev_image, dev_sampler, map_to_float4(coord));
    return read_pixelf(dev_image, coord_4);
  }
}

__opencrun_overload
float4 read_imagef(read_only image3d_t image, sampler_t sampler, float4 coord) {
  device_image_t *dev_image = __builtin_astype(image, device_image_t *);
  device_sampler_t dev_sampler = *(__builtin_astype(sampler, device_sampler_t *));

  if(dev_sampler & CLK_FILTER_LINEAR)
    return apply_filter_linear(dev_image, dev_sampler, map_to_float4(coord), 3);
  else {
    int4 coord_4 = apply_filter_nearest(dev_image, dev_sampler, map_to_float4(coord));
    return read_pixelf(dev_image, coord_4);
  }
}

#undef OPENCRUN_BUILTIN_IMAGE 
