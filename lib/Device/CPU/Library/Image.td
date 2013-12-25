//===----------------------------------------------------------------------===//
// Image builtin implementation utils
//===----------------------------------------------------------------------===//

def device_image : OCLCodeBlock<[{ 
typedef struct device_image_t {
  uint image_channel_order;
  uint image_channel_data_type;
  uint num_channels;
  uint element_size;

  uint width;
  uint height;
  uint depth;

  uint row_pitch;
  uint slice_pitch;

  uint array_size;
 
  uint num_mip_levels;
  uint num_samples;

  void *data;
} device_image_t; 
}]>;

def device_sampler : OCLCodeBlock<[{
typedef uint device_sampler_t; 
}]>;

def map_to_float4 : OCLCodeBlock<[{
__opencrun_overload
float4 map_to_float4(int coord) {
  return (float4)(coord, 0.0f, 0.0f, 0.0f);
}

__opencrun_overload
float4 map_to_float4(int2 coord) {
  return (float4)(coord.x, coord.y, 0.0f, 0.0f);
}

__opencrun_overload
float4 map_to_float4(int4 coord) {
  return (float4)(coord.x, coord.y, coord.z, coord.w);
}

__opencrun_overload
float4 map_to_float4(float coord) {
  return (float4)(coord, 0.0f, 0.0f, 0.0f);
}

__opencrun_overload
float4 map_to_float4(float2 coord) {
  return (float4)(coord.x, coord.y, 0.0f, 0.0f);
}

__opencrun_overload
float4 map_to_float4(float4 coord) {
  return coord;
}
}]>;

def map_to_int4 : OCLCodeBlock<[{
__opencrun_overload
int4 map_to_int4(int coord) {
  return (int4)(coord, 0, 0, 0);
}

__opencrun_overload
int4 map_to_int4(int2 coord) {
  return (int4)(coord.x, coord.y, 0, 0);
}

__opencrun_overload
int4 map_to_int4(int4 coord) {
  return (int4)(coord.x, coord.y, coord.z, coord.w);
}
}]>;

def address_mode : OCLCodeBlock<[{
int4 address_mode(const device_image_t *dev_image, const device_sampler_t dev_sampler, const int4 coord) {
  int4 coord_out;

  switch(dev_sampler & 0x0f) {
  case CLK_ADDRESS_NONE:
    coord_out = coord;
    break;
  case CLK_ADDRESS_CLAMP:
    coord_out = clamp(coord, (int4)(-1), (int4)(dev_image->width, dev_image->height, dev_image->depth, 0));
    break;
  case CLK_ADDRESS_CLAMP_TO_EDGE:
    coord_out = clamp(coord, (int4)(0), (int4)(dev_image->width - 1, dev_image->height - 1, dev_image->depth - 1, 0));
    break;
  default:
    coord_out = coord;
  }

  return coord_out;
}
}]>;

def apply_filter_nearest : OCLCodeBlock<[{
int4 apply_filter_nearest(const device_image_t *dev_image, const device_sampler_t dev_sampler, const float4 coord) {
  int4 coord_out;
  float4 coord_tmp; 

  coord_tmp = coord;

  if(dev_sampler & CLK_NORMALIZED_COORDS_TRUE) {
    coord_tmp = (float4)(coord.x * dev_image->width, coord.y * dev_image->height, coord.z * dev_image->depth, 0);
    
    if(dev_sampler & CLK_ADDRESS_REPEAT) {
      float u = (coord.x - floor(coord.x)) * dev_image->width;
      coord_tmp.x = floor(u);
      if(coord_tmp.x > dev_image->width - 1)
        coord_tmp.x -= dev_image->width;

      float v = (coord.y - floor(coord.y)) * dev_image->height;
      coord_tmp.y = floor(v);
      if(coord_tmp.y > dev_image->height - 1)
        coord_tmp.y -= dev_image->height;

      float w = (coord.z - floor(coord.z)) * dev_image->depth;
      coord_tmp.z = floor(w);
      if(coord_tmp.z > dev_image->depth - 1)
        coord_tmp.z -= dev_image->depth;
    }

    else if(dev_sampler & CLK_ADDRESS_MIRRORED_REPEAT) {
      float u = fabs(coord.x - 2.0f * rint(0.5f * coord.x)) * dev_image->width; 
      coord_tmp.x = floor(u);
      if(coord_tmp.x == dev_image->width)
        --coord_tmp.x;

      float v = fabs(coord.y - 2.0f * rint(0.5f * coord.y)) * dev_image->height; 
      coord_tmp.x = floor(v);
      if(coord_tmp.y == dev_image->height)
        --coord_tmp.y;

      float w = fabs(coord.z - 2.0f * rint(0.5f * coord.z)) * dev_image->depth; 
      coord_tmp.z = floor(w);
      if(coord_tmp.z == dev_image->depth)
        --coord_tmp.z;
    }

  }

  switch(dev_sampler & 0x0f) {
  case CLK_ADDRESS_NONE:
  case CLK_ADDRESS_CLAMP:
  case CLK_ADDRESS_CLAMP_TO_EDGE:
    coord_out = address_mode(dev_image, dev_sampler, convert_int4(floor(coord_tmp)));
    break;
  default:
    coord_out = convert_int4(coord_tmp);
  }

  return coord_out;
}
}]>;

def apply_filter_linear : OCLCodeBlock<[{
float4 apply_filter_linear(const device_image_t *dev_image,
                           const device_sampler_t dev_sampler,
                           const float4 coord,
                           const unsigned size) {
  float4 color_out;
  float u, v, w;
  int i0, j0, k0;
  int i1, j1, k1;
  float a, b, c;

  switch(dev_sampler & 0x0f) {
  case CLK_ADDRESS_NONE:
  case CLK_ADDRESS_CLAMP:
  case CLK_ADDRESS_CLAMP_TO_EDGE:
    u = (dev_sampler & CLK_NORMALIZED_COORDS_TRUE) ? (coord.x * dev_image->width) : coord.x;
    v = (dev_sampler & CLK_NORMALIZED_COORDS_TRUE) ? (coord.y * dev_image->height) : coord.y;
    w = (dev_sampler & CLK_NORMALIZED_COORDS_TRUE) ? (coord.z * dev_image->depth) : coord.z;

    int4 coord_0 = address_mode(dev_image,
                                dev_sampler,
                                (int4)(floor(u - 0.5f), floor(v - 0.5f), floor(w - 0.5f), 0));

    i0 = coord_0.x;
    j0 = coord_0.y;
    k0 = coord_0.z;

    int4 coord_1 = address_mode(dev_image,
                                dev_sampler,
                                (int4)(floor(u - 0.5f) + 1, floor(v - 0.5f) + 1, floor(w - 0.5f) + 1, 0));

    i1 = coord_1.x;
    j1 = coord_1.y;
    k1 = coord_1.z;
  
	a = u - 0.5f - floor(u - 0.5f);
	b = v - 0.5f - floor(v - 0.5f);
	c = w - 0.5f - floor(w - 0.5f);

    break;
  case CLK_ADDRESS_REPEAT:
    u = (coord.x - floor(coord.x)) * dev_image->width;
	i0 = (int)floor(u - 0.5f);
	i1 = i0 + 1;
	if(i0 < 0) i0 += dev_image->width;
	if(i1 > (int)(dev_image->width - 1)) i1 -= dev_image->width;
	
	v = (coord.y - floor(coord.y)) * dev_image->height;
	j0 = (int)floor(v - 0.5f);
	j1 = j0 + 1;
	if(j0 < 0) j0 += dev_image->height;
	if(j1 > (int)(dev_image->height - 1)) j1 -= dev_image->height;

    w = (coord.z - floor(coord.z)) * dev_image->depth;
    k0 = (int)floor(w - 0.5f);
	k1 = k0 + 1;
	if(k0 < 0) k0 += dev_image->depth;
	if(k1 > (int)(dev_image->depth - 1)) k1 -= dev_image->depth;
	    
	a = u - 0.5f - floor(u - 0.5f);
	b = v - 0.5f - floor(v - 0.5f);
	c = w - 0.5f - floor(w - 0.5f);

    break;
  case CLK_ADDRESS_MIRRORED_REPEAT:
    u = fabs(coord.x - 2.0f * rint(0.5f * coord.x)) * dev_image->width; 
    i0 = (int)floor(u - 0.5f);
    i1 = i0 + 1;
    i0 = max(i0, 0);
    i1 = min(i1, (int)(dev_image->width - 1));

    v = fabs(coord.y - 2.0f * rint(0.5f * coord.y)) * dev_image->height; 
    j0 = (int)floor(v - 0.5f);
    j1 = j0 + 1;
    j0 = max(j0, 0);
    j1 = min(j1, (int)(dev_image->height - 1));

    w = fabs(coord.z - 2.0f * rint(0.5f * coord.z)) * dev_image->depth; 
    k0 = (int)floor(w - 0.5f);
    k1 = k0 + 1;
    k0 = max(k0, 0);
    k1 = min(k1, (int)(dev_image->depth - 1));
      
	a = u - 0.5f - floor(u - 0.5f);
	b = v - 0.5f - floor(v - 0.5f);
	c = w - 0.5f - floor(w - 0.5f);
   
    break;
  default:
    return (float4)(0.0f); 
  }

  switch(size) {
  case 1:
    color_out = (float4)((1.0f - a) * read_pixelf(dev_image, (int4)(i0, 0, 0, 0)) +
                         a * read_pixelf(dev_image, (int4)(i1, 0, 0, 0)));
    break;
  case 2:
    color_out = (float4)((1.0f - a) * (1.0f - b) * read_pixelf(dev_image, (int4)(i0, j0, 0, 0)) +
                  a * (1.0f - b)  * read_pixelf(dev_image, (int4)(i1, j0, 0, 0)) +
                  (1.0f - a) * b * read_pixelf(dev_image, (int4)(i0, j1, 0, 0)) +
                  a * b * read_pixelf(dev_image, (int4)(i1, j1, 0, 0)));
    break;
  case 3:
  default:
    color_out = (float4)((1.0f - a) * (1.0f - b) * (1.0f - c) * read_pixelf(dev_image, (int4)(i0, j0, k0, 0)) +
                  a * (1.0f - b) * (1.0f - c) * read_pixelf(dev_image, (int4)(i1, j0, k0, 0)) +
                  (1.0f - a) * b * (1.0f - c) * read_pixelf(dev_image, (int4)(i0, j1, k0, 0)) +
                  a * b * (1.0f - c) * read_pixelf(dev_image, (int4)(i1, j1, k0, 0)) +
                  (1.0f - a) * (1.0f - b) * c * read_pixelf(dev_image, (int4)(i0, j0, k1, 0)) +
                  a * (1.0f - b) * c * read_pixelf(dev_image, (int4)(i1, j0, k1, 0)) +
                  (1.0f - a) * b * c * read_pixelf(dev_image, (int4)(i0, j1, k1, 0)) +
                  a * b * c * read_pixelf(dev_image, (int4)(i1, j1, k1, 0)));
    break;
  }
  
  return color_out;
}
}]>;

def map_channels_read : OCLCodeBlock<[{
__opencrun_overload
int4 map_channels_read(const device_image_t *dev_image, const int4 color) {
  int4 color_out;
  
  switch(dev_image->image_channel_order) {
  case CLK_R:
  case CLK_Rx:
    color_out = (int4)(color.x, 0, 0, 1);
    break;
  case CLK_A:
    color_out = (int4)(0, 0, 0, color.x);
    break;
  case CLK_RG:
  case CLK_RGx:
    color_out = (int4)(color.x, color.y, 0, 1);
    break;
  case CLK_RA:
    color_out = (int4)(color.x, 0, 0, color.y);
    break;
  case CLK_RGB:
  case CLK_RGBx:
    color_out = (int4)(color.x, color.y, color.z, 1);
    break;
  case CLK_RGBA:
    color_out = color;
    break;
  case CLK_BGRA:
    color_out = (int4)(color.z, color.y, color.x, color.w);
    break;
  case CLK_ARGB:
    color_out = (int4)(color.y, color.z, color.w, color.x);
    break;
  case CLK_INTENSITY:
    color_out = (int4)(color.x);
    break;
  case CLK_LUMINANCE:
    color_out = (int4)(color.x, color.x, color.x, 1);
    break;
  default:
    color_out = color;
  }

  return color_out;
}

__opencrun_overload
uint4 map_channels_read(const device_image_t *dev_image, const uint4 color) {
  uint4 color_out;
  
  switch(dev_image->image_channel_order) {
  case CLK_R:
  case CLK_Rx:
    color_out = (uint4)(color.x, 0, 0, 1);
    break;
  case CLK_A:
    color_out = (uint4)(0, 0, 0, color.x);
    break;
  case CLK_RG:
  case CLK_RGx:
    color_out = (uint4)(color.x, color.y, 0, 1);
    break;
  case CLK_RA:
    color_out = (uint4)(color.x, 0, 0, color.y);
    break;
  case CLK_RGB:
  case CLK_RGBx:
    color_out = (uint4)(color.x, color.y, color.z, 1);
    break;
  case CLK_RGBA:
    color_out = color;
    break;
  case CLK_BGRA:
    color_out = (uint4)(color.z, color.y, color.x, color.w);
    break;
  case CLK_ARGB:
    color_out = (uint4)(color.y, color.z, color.w, color.x);
    break;
  case CLK_INTENSITY:
    color_out = (uint4)(color.x);
    break;
  case CLK_LUMINANCE:
    color_out = (uint4)(color.x, color.x, color.x, 1);
    break;
  default:
    color_out = color;
  }

  return color_out;
}

__opencrun_overload
float4 map_channels_read(const device_image_t *dev_image, const float4 color) {
  float4 color_out;

  switch(dev_image->image_channel_order) {
  case CLK_R:
  case CLK_Rx:
    color_out = (float4)(color.x, 0.0f, 0.0f, 1.0f);
    break;
  case CLK_A:
    color_out = (float4)(0.0f, 0.0f, 0.0f, color.x);
    break;
  case CLK_RG:
  case CLK_RGx:
    color_out = (float4)(color.x, color.y, 0.0f, 1.0f);
    break;
  case CLK_RA:
    color_out = (float4)(color.x, 0.0f, 0.0f, color.y);
    break;
  case CLK_RGB:
  case CLK_RGBx:
    color_out = (float4)(color.x, color.y, color.z, 1.0f);
    break;
  case CLK_RGBA:
    color_out = color;
    break;
  case CLK_BGRA:
    color_out = (float4)(color.z, color.y, color.x, color.w);
    break;
  case CLK_ARGB:
    color_out = (float4)(color.y, color.z, color.w, color.x);
    break;
  case CLK_INTENSITY:
    color_out = (float4)(color.x);
    break;
  case CLK_LUMINANCE:
    color_out = (float4)(color.x, color.x, color.x, 1.0f);
    break;
  default:
    color_out = color;
  }

  return color_out;
}
}]>;

def map_channels_write : OCLCodeBlock<[{
__opencrun_overload
int4 map_channels_write(const device_image_t *dev_image, const int4 color) {
  int4 color_out;

  switch(dev_image->image_channel_order) {
  case CLK_R: case CLK_Rx:
    color_out.x = color.x;
    break;
  case CLK_A:
    color_out.x = color.w;
    break;
  case CLK_RG: case CLK_RGx:
    color_out.x = color.x;
    color_out.y = color.y;
    break;
  case CLK_RA:
    color_out.x = color.x;
    color_out.y = color.w;
    break;
  case CLK_RGB: case CLK_RGBx:
    color_out.x = color.x;
    color_out.y = color.y;
    color_out.z = color.z;
    break;
  case CLK_RGBA:
    color_out = color;
    break;
  case CLK_BGRA:
    color_out.x = color.z;
    color_out.y = color.y;
    color_out.z = color.x;
    color_out.w = color.w;
    break;
  case CLK_ARGB:
    color_out.x = color.w;
    color_out.y = color.x;
    color_out.z = color.y;
    color_out.w = color.z;
    break;
  case CLK_INTENSITY:
    color_out.x = color.x;
    break;
  case CLK_LUMINANCE:
    color_out.x = color.x;
    break;
  default:
    color_out = color;
  }

  return color_out;
}

__opencrun_overload
uint4 map_channels_write(const device_image_t *dev_image, const uint4 color) {
  uint4 color_out;

  switch(dev_image->image_channel_order) {
  case CLK_R: case CLK_Rx:
    color_out.x = color.x;
    break;
  case CLK_A:
    color_out.x = color.w;
    break;
  case CLK_RG: case CLK_RGx:
    color_out.x = color.x;
    color_out.y = color.y;
    break;
  case CLK_RA:
    color_out.x = color.x;
    color_out.y = color.w;
    break;
  case CLK_RGB: case CLK_RGBx:
    color_out.x = color.x;
    color_out.y = color.y;
    color_out.z = color.z;
    break;
  case CLK_RGBA:
    color_out = color;
    break;
  case CLK_BGRA:
    color_out.x = color.z;
    color_out.y = color.y;
    color_out.z = color.x;
    color_out.w = color.w;
    break;
  case CLK_ARGB:
    color_out.x = color.w;
    color_out.y = color.x;
    color_out.z = color.y;
    color_out.w = color.z;
    break;
  case CLK_INTENSITY:
    color_out.x = color.x;
    break;
  case CLK_LUMINANCE:
    color_out.x = color.x;
    break;
  default:
    color_out = color;
  }

  return color_out;
}

__opencrun_overload
float4 map_channels_write(const device_image_t *dev_image, const float4 color) {
  float4 color_out;

  switch(dev_image->image_channel_order) {
  case CLK_R: case CLK_Rx:
    color_out.x = color.x;
    break;
  case CLK_A:
    color_out.x = color.w;
    break;
  case CLK_RG: case CLK_RGx:
    color_out.x = color.x;
    color_out.y = color.y;
    break;
  case CLK_RA:
    color_out.x = color.x;
    color_out.y = color.w;
    break;
  case CLK_RGB: case CLK_RGBx:
    color_out.x = color.x;
    color_out.y = color.y;
    color_out.z = color.z;
    break;
  case CLK_RGBA:
    color_out = color;
    break;
  case CLK_BGRA:
    color_out.x = color.z;
    color_out.y = color.y;
    color_out.z = color.x;
    color_out.w = color.w;
    break;
  case CLK_ARGB:
    color_out.x = color.w;
    color_out.y = color.x;
    color_out.z = color.y;
    color_out.w = color.z;
    break;
  case CLK_INTENSITY:
    color_out.x = color.x;
    break;
  case CLK_LUMINANCE:
    color_out.x = color.x;
    break;
  default:
    color_out = color;
  }

  return color_out;
}
}]>;

def read_pixel : OCLCodeBlock<[{
int4 read_pixeli(const device_image_t *dev_image, const int4 coord) {
  if((coord.x <= -1 || coord.x >= (int)dev_image->width) ||
     (coord.y <= -1 || coord.y >= (int)dev_image->height) ||
     (coord.z <= -1 || coord.z >= (int)dev_image->depth)) {
    switch(dev_image->image_channel_order) {
    case CLK_A: case CLK_INTENSITY: case CLK_Rx: case CLK_RA:
    case CLK_RGx: case CLK_RGBx: case CLK_ARGB: case CLK_BGRA:
    case CLK_RGBA:
      return (int4)(0, 0, 0, 0);
    case CLK_R: case CLK_RG: case CLK_RGB: case CLK_LUMINANCE:
      return (int4)(0, 0, 0, 1);
    }
  }
  
  const void *src = (const void *)((uintptr_t)dev_image->data +
                                   dev_image->element_size * coord.x +
                                   dev_image->row_pitch * coord.y +
                                   dev_image->slice_pitch * coord.z);
  int4 color;
  
  switch(dev_image->image_channel_data_type) {
  case CLK_SIGNED_INT8:
    switch(dev_image->num_channels) {
    case 1:
      color = (int4)(((const char *)src)[0], 0, 0, 0);
      break;
    case 2:
      color = (int4)(((const char *)src)[0], ((const char *)src)[1], 0, 0);
      break;
    case 3:
      color = (int4)(((const char *)src)[0], ((const char *)src)[1], ((const char *)src)[2], 0);
      break;
    case 4:
      color = (int4)(((const char *)src)[0], ((const char *)src)[1], ((const char *)src)[2], ((const char *)src)[3]);
      break;
    }
    break;
  case CLK_SIGNED_INT16:
    switch(dev_image->num_channels) {
    case 1:
      color = (int4)(((const short *)src)[0], 0, 0, 0);
      break;
    case 2:
      color = (int4)(((const short *)src)[0], ((const short *)src)[1], 0, 0);
      break;
    case 3:
      color = (int4)(((const short *)src)[0], ((const short *)src)[1], ((const short *)src)[2], 0);
      break;
    case 4:
      color = (int4)(((const short *)src)[0], ((const short *)src)[1], ((const short *)src)[2], ((const short *)src)[3]);
      break;
    }
    break;
  case CLK_SIGNED_INT32:
    switch(dev_image->num_channels) {
    case 1:
      color = (int4)(((const int *)src)[0], 0, 0, 0);
      break;
    case 2:
      color = (int4)(((const int *)src)[0], ((const int *)src)[1], 0, 0);
      break;
    case 3:
      color = (int4)(((const int *)src)[0], ((const int *)src)[1], ((const int *)src)[2], 0);
      break;
    case 4:
      color = (int4)(((const int *)src)[0], ((const int *)src)[1], ((const int *)src)[2], ((const int *)src)[3]);
      break;
    }
    break;
  default:
    color = (int4)(0, 0, 0, 0);
  }

  return map_channels_read(dev_image, color);
}

__opencrun_overload
uint4 read_pixelui(const device_image_t *dev_image, const int4 coord) {
  if((coord.x <= -1 || coord.x >= (int)dev_image->width) ||
     (coord.y <= -1 || coord.y >= (int)dev_image->height) ||
     (coord.z <= -1 || coord.z >= (int)dev_image->depth)) {
    switch(dev_image->image_channel_order) {
    case CLK_A: case CLK_INTENSITY: case CLK_Rx: case CLK_RA:
    case CLK_RGx: case CLK_RGBx: case CLK_ARGB: case CLK_BGRA:
    case CLK_RGBA:
      return (uint4)(0, 0, 0, 0);
    case CLK_R: case CLK_RG: case CLK_RGB: case CLK_LUMINANCE:
      return (uint4)(0, 0, 0, 1);
    }
  }

  const void *src = (const void *)((uintptr_t)dev_image->data +
                                   dev_image->element_size * coord.x +
                                   dev_image->row_pitch * coord.y +
                                   dev_image->slice_pitch * coord.z);
  uint4 color;
 
  switch(dev_image->image_channel_data_type) {
  case CLK_UNSIGNED_INT8:
    switch(dev_image->num_channels) {
    case 1:
      color = (uint4)(((const uchar *)src)[0], 0, 0, 0);
      break;
    case 2:
      color = (uint4)(((const uchar *)src)[0], ((const uchar *)src)[1], 0, 0);
      break;
    case 3:
      color = (uint4)(((const uchar *)src)[0], ((const uchar *)src)[1], ((const uchar *)src)[2], 0);
      break;
    case 4:
      color = (uint4)(((const uchar *)src)[0], ((const uchar *)src)[1], ((const uchar *)src)[2], ((const uchar *)src)[3]);
      break;
    }
    break;
  case CLK_UNSIGNED_INT16:
    switch(dev_image->num_channels) {
    case 1:
      color = (uint4)(((const ushort *)src)[0], 0, 0, 0);
      break;
    case 2:
      color = (uint4)(((const ushort *)src)[0], ((const ushort *)src)[1], 0, 0);
      break;
    case 3:
      color = (uint4)(((const ushort *)src)[0], ((const ushort *)src)[1], ((const ushort *)src)[2], 0);
      break;
    case 4:
      color = (uint4)(((const ushort *)src)[0], ((const ushort *)src)[1], ((const ushort *)src)[2], ((const ushort *)src)[3]);
      break;
    }
    break;
  case CLK_UNSIGNED_INT32:
    switch(dev_image->num_channels) {
    case 1:
      color = (uint4)(((const uint *)src)[0], 0, 0, 0);
      break;
    case 2:
      color = (uint4)(((const uint *)src)[0], ((const uint *)src)[1], 0, 0);
      break;
    case 3:
      color = (uint4)(((const uint *)src)[0], ((const uint *)src)[1], ((const uint *)src)[2], 0);
      break;
    case 4:
      color = (uint4)(((const uint *)src)[0], ((const uint *)src)[1], ((const uint *)src)[2], ((const uint *)src)[3]);
      break;
    }
    break;
  default:
    color = (uint4)(0, 0, 0, 0);
  }

  return map_channels_read(dev_image, color);
}

__opencrun_overload
float4 read_pixelf(const device_image_t *dev_image, const int4 coord) {
  if((coord.x <= -1 || coord.x >= (int)dev_image->width) ||
     (coord.y <= -1 || coord.y >= (int)dev_image->height) ||
     (coord.z <= -1 || coord.z >= (int)dev_image->depth)) {
    switch(dev_image->image_channel_order) {
    case CLK_A: case CLK_INTENSITY: case CLK_Rx: case CLK_RA:
    case CLK_RGx: case CLK_RGBx: case CLK_ARGB: case CLK_BGRA:
    case CLK_RGBA:
      return (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    case CLK_R: case CLK_RG: case CLK_RGB: case CLK_LUMINANCE:
      return (float4)(0.0f, 0.0f, 0.0f, 1.0f);
    }
  }

  const void *src = (const void *)((uintptr_t)dev_image->data +
                                   dev_image->element_size * coord.x +
                                   dev_image->row_pitch * coord.y +
                                   dev_image->slice_pitch * coord.z);
  float4 color;
 
  switch(dev_image->image_channel_data_type) {
  case CLK_SNORM_INT8:
    switch(dev_image->num_channels) {
    case 1:
      color = fmax((float4)(-1.0f), (float4)(((const char *)src)[0], 0.0f, 0.0f, 0.0f) / 127.0f);
      break;
    case 2:
      color = fmax((float4)(-1.0f), (float4)(((const char *)src)[0], ((const char *)src)[1], 0.0f , 0.0f) / 127.0f);
      break;
    case 3:
      color = fmax((float4)(-1.0f), (float4)(((const char *)src)[0], ((const char *)src)[1], ((const char *)src)[2], 0.0f) / 127.0f);
      break;
    case 4:
      color = fmax((float4)(-1.0f), (float4)(((const char *)src)[0], ((const char *)src)[1], ((const char *)src)[2], ((const char *)src)[3]) / 127.0f);
      break;
    }
    break;
  case CLK_SNORM_INT16:
    switch(dev_image->num_channels) {
    case 1:
      color = fmax((float4)(-1.0f), (float4)(((const short *)src)[0], 0.0f, 0.0f, 0.0f) / 32767.0f);
      break;
    case 2:
      color = fmax((float4)(-1.0f), (float4)(((const short *)src)[0], ((const short *)src)[1], 0.0f , 0.0f) / 32767.0f);
      break;
    case 3:
      color = fmax((float4)(-1.0f), (float4)(((const short *)src)[0], ((const short *)src)[1], ((const short *)src)[2], 0.0f) / 32767.0f);
      break;
    case 4:
      color = fmax((float4)(-1.0f), (float4)(((const short *)src)[0], ((const short *)src)[1], ((const short *)src)[2], ((const short *)src)[3]) / 32767.0f);
      break;
    }
    break;
  case CLK_UNORM_INT8:
    switch(dev_image->num_channels) {
    case 1:
      color = (float4)(((const uchar *)src)[0], 0.0f, 0.0f, 0.0f) / 255.0f;
      break;
    case 2:
      color = (float4)(((const uchar *)src)[0], ((const uchar *)src)[1], 0.0f , 0.0f) / 255.0f;
      break;
    case 3:
      color = (float4)(((const uchar *)src)[0], ((const uchar *)src)[1], ((const uchar *)src)[2], 0.0f) / 255.0f;
      break;
    case 4:
      color = (float4)(((const uchar *)src)[0], ((const uchar *)src)[1], ((const uchar *)src)[2], ((const uchar *)src)[3]) / 255.0f;
      break;
    }
    break;
  case CLK_UNORM_INT16:
    switch(dev_image->num_channels) {
    case 1:
      color = (float4)(((const ushort *)src)[0], 0.0f, 0.0f, 0.0f) / 65535.0f;
      break;
    case 2:
      color = (float4)(((const ushort *)src)[0], ((const ushort *)src)[1], 0.0f , 0.0f) / 65535.0f;
      break;
    case 3:
      color = (float4)(((const ushort *)src)[0], ((const ushort *)src)[1], ((const ushort *)src)[2], 0.0f) / 65535.0f;
      break;
    case 4:
      color = (float4)(((const ushort *)src)[0], ((const ushort *)src)[1], ((const ushort *)src)[2], ((const ushort *)src)[3]) / 65535.0f;
      break;
    }
    break;
  case CLK_UNORM_SHORT_565: 
    {
      ushort denorm_us = *((const ushort *)src);
      color = (float4)(((denorm_us >> 11) & 0x1f) / 31.0f, ((denorm_us >> 5) & 0x3f) / 63.0f, (denorm_us & 0x1f) / 31.0f, 0.0f);
    }
    break;
  case CLK_UNORM_SHORT_555:
    {
      ushort denorm_us = *((const ushort *)src);
      color = (float4)(((denorm_us >> 10) & 0x1f) / 31.0f, ((denorm_us >> 5) & 0x1f) / 31.0f, (denorm_us & 0x1f) / 31.0f, 0.0f);
    }
    break;
  case CLK_UNORM_INT_101010:
    {
      uint denorm_ui = *((const uint *)src);
      color = (float4)(((denorm_ui >> 20) & 0x3ff) / 1023.0f, ((denorm_ui >> 10) & 0x3ff) / 1023.0f, (denorm_ui & 0x3ff) / 1023.0f, 0.0f);
    }
    break;
  case CLK_FLOAT:
    switch(dev_image->num_channels) {
    case 1:
      color = (float4)(((const float *)src)[0], 0.0f, 0.0f, 0.0f);
      break;
    case 2:
      color = (float4)(((const float *)src)[0], ((const float *)src)[1], 0.0f, 0.0f);
      break;
    case 3:
      color = (float4)(((const float *)src)[0], ((const float *)src)[1], ((const float *)src)[2], 0.0f);
      break;
    case 4:
      color = (float4)(((const float *)src)[0], ((const float *)src)[1], ((const float *)src)[2], ((const float *)src)[3]);
      break;
    }
    break;
  default:
    color = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
  }

  return map_channels_read(dev_image, color);
}
}]>;

def write_pixel : OCLCodeBlock<[{
__opencrun_overload
void write_pixel(device_image_t *dev_image, const int4 coord, const int4 color) {
  void *dst = (void *)((uintptr_t)dev_image->data +
                       dev_image->element_size * coord.x +
                       dev_image->row_pitch * coord.y +
                       dev_image->slice_pitch * coord.z);

  switch(dev_image->image_channel_data_type) {
  case CLK_SIGNED_INT8:
    switch(dev_image->num_channels) {
    case 1:
      *((char *)dst) = convert_char_sat(color.x);
      break;
    case 2:
      *((char2 *)dst) = convert_char2_sat((int2)(color.x, color.y));
      break;
    case 3:
      *((char3 *)dst) = convert_char3_sat((int3)(color.x, color.y, color.z));
      break;
    case 4:
      *((char4 *)dst) = convert_char4_sat((int4)(color.x, color.y, color.z, color.w));
    }
	break;
  case CLK_SIGNED_INT16:
    switch(dev_image->num_channels) {
    case 1:
      *((short *)dst) = convert_short_sat(color.x);
      break;
    case 2:
      *((short2 *)dst) = convert_short2_sat((int2)(color.x, color.y));
      break;
    case 3:
      *((short3 *)dst) = convert_short3_sat((int3)(color.x, color.y, color.z));
      break;
    case 4:
      *((short4 *)dst) = convert_short4_sat((int4)(color.x, color.y, color.z, color.w));
    }
	break;
  case CLK_SIGNED_INT32:
    switch(dev_image->num_channels) {
    case 1:
      *((int *)dst) = color.x;
      break;
    case 2:
      *((int2 *)dst) = (int2)(color.x, color.y);
      break;
    case 3:
      *((int3 *)dst) = (int3)(color.x, color.y, color.z);
      break;
    case 4:
      *((int4 *)dst) = color;
    }
	break;
  }
}

__opencrun_overload
void write_pixel(device_image_t *dev_image, const int4 coord, const uint4 color) {
  void *dst = (void *)((uintptr_t)dev_image->data +
                       dev_image->element_size * coord.x +
                       dev_image->row_pitch * coord.y +
                       dev_image->slice_pitch * coord.z);

  switch(dev_image->image_channel_data_type) {
  case CLK_UNSIGNED_INT8:
    switch(dev_image->num_channels) {
    case 1:
      *((uchar *)dst) = convert_uchar_sat(color.x);
      break;
    case 2:
      *((uchar2 *)dst) = convert_uchar2_sat((uint2)(color.x, color.y));
      break;
    case 3:
      *((uchar3 *)dst) = convert_uchar3_sat((uint3)(color.x, color.y, color.z));
      break;
    case 4:
      *((uchar4 *)dst) = convert_uchar4_sat((uint4)(color.x, color.y, color.z, color.w));
    }
	break;
  case CLK_UNSIGNED_INT16:
    switch(dev_image->num_channels) {
    case 1:
      *((ushort *)dst) = convert_ushort_sat(color.x);
      break;
    case 2:
      *((ushort2 *)dst) = convert_ushort2_sat((uint2)(color.x, color.y));
      break;
    case 3:
      *((ushort3 *)dst) = convert_ushort3_sat((uint3)(color.x, color.y, color.z));
      break;
    case 4:
      *((ushort4 *)dst) = convert_ushort4_sat((uint4)(color.x, color.y, color.z, color.w));
    }
	break;
  case CLK_UNSIGNED_INT32:
    switch(dev_image->num_channels) {
    case 1:
      *((uint *)dst) = color.x;
      break;
    case 2:
      *((uint2 *)dst) = (uint2)(color.x, color.y);
      break;
    case 3:
      *((uint3 *)dst) = (uint3)(color.x, color.y, color.z);
      break;
    case 4:
      *((uint4 *)dst) = color;
    }
	break;
  }
}

__opencrun_overload
void write_pixel(device_image_t *dev_image, const int4 coord, const float4 color) {
  void *dst = (void *)((uintptr_t)dev_image->data +
                       dev_image->element_size * coord.x +
                       dev_image->row_pitch * coord.y +
                       dev_image->slice_pitch * coord.z);

  switch(dev_image->image_channel_data_type) {
  case CLK_UNORM_SHORT_565:
    *((ushort *)dst) = (min(convert_uchar_sat_rte(color.x * 31.0f), (uchar)0x1f) << 11) |
                       (min(convert_uchar_sat_rte(color.y * 63.0f), (uchar)0x3f) << 6) |
                       (min(convert_uchar_sat_rte(color.z * 31.0f), (uchar)0x1f));
    break;
  case CLK_UNORM_SHORT_555:
    *((ushort *)dst) = (min(convert_uchar_sat_rte(color.x * 31.0f), (uchar)0x1f) << 10) |
                       (min(convert_uchar_sat_rte(color.y * 31.0f), (uchar)0x1f) << 5) |
                       (min(convert_uchar_sat_rte(color.z * 31.0f), (uchar)0x1f));
    break;
  case CLK_UNORM_INT_101010:
    *((uint *)dst) = (min(convert_ushort_sat_rte(color.x * 1023.0f), (ushort)0x3ff) << 20) |
                     (min(convert_ushort_sat_rte(color.y * 1023.0f), (ushort)0x3ff) << 10) |
                     (min(convert_ushort_sat_rte(color.z * 1023.0f), (ushort)0x3ff));
    break;
  case CLK_SNORM_INT8:
    switch(dev_image->num_channels) {
    case 1:
      *((char *)dst) = convert_char_sat_rte(color.x * 127.0f);
      break;
    case 2:
      *((char2 *)dst) = convert_char2_sat_rte((float2)(color.x, color.y) * 127.0f);
      break;
    case 3:
      *((char3 *)dst) = convert_char3_sat_rte((float3)(color.x, color.y, color.z) * 127.0f);
      break;
    case 4:
      *((char4 *)dst) = convert_char4_sat_rte((float4)(color.x, color.y, color.z, color.w) * 127.0f);
      break;
    }
    break;
  case CLK_UNORM_INT8:
    switch(dev_image->num_channels) {
    case 1:
      *((uchar *)dst) = convert_uchar_sat_rte(color.x * 255.0f);
      break;
    case 2:
      *((uchar2 *)dst) = convert_uchar2_sat_rte((float2)(color.x, color.y) * 255.0f);
      break;
    case 3:
      *((uchar3 *)dst) = convert_uchar3_sat_rte((float3)(color.x, color.y, color.z) * 255.0f);
      break;
    case 4:
      *((uchar4 *)dst) = convert_uchar4_sat_rte((float4)(color.x, color.y, color.z, color.w) * 255.0f);
      break;
    }
    break;
  case CLK_SNORM_INT16:
    switch(dev_image->num_channels) {
    case 1:
      *((short *)dst) = convert_short_sat_rte(color.x * 32767.0f);
      break;
    case 2:
      *((short2 *)dst) = convert_short2_sat_rte((float2)(color.x, color.y) * 32767.0f);
      break;
    case 3:
      *((short3 *)dst) = convert_short3_sat_rte((float3)(color.x, color.y, color.z) * 32767.0f);
      break;
    case 4:
      *((short4 *)dst) = convert_short4_sat_rte((float4)(color.x, color.y, color.z, color.w) * 32767.0f);
      break;
    }
    break;
  case CLK_UNORM_INT16:
    switch(dev_image->num_channels) {
    case 1:
      *((ushort *)dst) = convert_ushort_sat_rte(color.x * 65535.0f);
      break;
    case 2:
      *((ushort2 *)dst) = convert_ushort2_sat_rte((float2)(color.x, color.y) * 65535.0f);
      break;
    case 3:
      *((ushort3 *)dst) = convert_ushort3_sat_rte((float3)(color.x, color.y, color.z) * 65535.0f);
      break;
    case 4:
      *((ushort4 *)dst) = convert_ushort4_sat_rte((float4)(color.x, color.y, color.z, color.w) * 65535.0f);
      break;
    }
    break;
  case CLK_FLOAT:
    switch(dev_image->num_channels) {
    case 1:
      *((float *)dst) = color.x;
      break;
    case 2:
      *((float2 *)dst) = (float2)(color.x, color.y);
      break;
    case 3:
      *((float3 *)dst) = (float3)(color.x, color.y, color.z);
      break;
    case 4:
      *((float4 *)dst) = color;
      break; 
    }
  }
}
}]>;

//===----------------------------------------------------------------------===//
// Image builtin implementations
//===----------------------------------------------------------------------===//

let Requirements = [device_image,
                    device_sampler,
                    address_mode,
                    map_channels_read,
                    map_to_float4,
                    read_pixel,
                    apply_filter_nearest,
                    apply_filter_linear] in {

def : OCLBuiltinImpl<blt_read_imagei, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  device_sampler_t dev_sampler = *((device_sampler_t *)&param2);
  int4 coord_4 = apply_filter_nearest(dev_image, dev_sampler, map_to_float4(param3));

  return read_pixeli(dev_image, coord_4);
}]>, "VnIMg">;

def : OCLBuiltinImpl<blt_read_imagei, TemplateStrategy<[{
  device_sampler_t dev_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
  return read_imagei(
          param1,
          *((sampler_t *)&dev_sampler),
          param2);
}]>, "VnIn">;

def : OCLBuiltinImpl<blt_read_imagei, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  device_sampler_t dev_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
  int4 coord_4 = apply_filter_nearest(dev_image, dev_sampler, map_to_float4(param2));

  return read_pixeli(dev_image, coord_4);
}]>, "VnDbICn">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_read_imageui, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  device_sampler_t dev_sampler = *((device_sampler_t *)&param2);
  int4 coord_4 = apply_filter_nearest(dev_image, dev_sampler, map_to_float4(param3));

  return read_pixelui(dev_image, coord_4);
}]>, "VUnIMg">;

def : OCLBuiltinImpl<blt_read_imageui, TemplateStrategy<[{
  device_sampler_t dev_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
  return read_imageui(
          param1,
          *((sampler_t *)&dev_sampler),
          param2);
}]>, "VUnIn">;

def : OCLBuiltinImpl<blt_read_imageui, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  device_sampler_t dev_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
  int4 coord_4 = apply_filter_nearest(dev_image, dev_sampler, map_to_float4(param2));

  return read_pixelui(dev_image, coord_4);
}]>, "VUnDbICn">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_read_imagef, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  device_sampler_t dev_sampler = *((device_sampler_t *)&param2);
  int4 coord_4 = apply_filter_nearest(dev_image, dev_sampler, map_to_float4(param3));

  return read_pixelf(dev_image, coord_4);
}]>, "VfIMn">;

def : OCLBuiltinImpl<blt_read_imagef, TemplateStrategy<[{
  device_sampler_t dev_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
  return read_imagef(
          param1,
          *((sampler_t *)&dev_sampler),
          param2);
}]>, "VfIn">;

def : OCLBuiltinImpl<blt_read_imagef, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  device_sampler_t dev_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
  int4 coord_4 = apply_filter_nearest(dev_image, dev_sampler, map_to_float4(param2));

  return read_pixelf(dev_image, coord_4);
}]>, "VfDbICn">;

}

let Requirements = [device_image,
                    map_channels_write,
                    map_to_int4,
                    write_pixel] in {

def : OCLBuiltinImpl<blt_write_imagei, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  int4 color_out = map_channels_write(dev_image, param3);

  write_pixel(dev_image, map_to_int4(param2), color_out);
}]>, "vD12InVn">;

def : OCLBuiltinImpl<blt_write_imagei, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  int4 color_out = map_channels_write(dev_image, param3);
  
  write_pixel(dev_image, map_to_int4(param2), color_out);
}]>, "vD3IVnVn">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_write_imageui, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  uint4 color_out = map_channels_write(dev_image, param3);

  write_pixel(dev_image, map_to_int4(param2), color_out);
}]>, "vD12InVUn">;

def : OCLBuiltinImpl<blt_write_imageui, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  uint4 color_out = map_channels_write(dev_image, param3);

  write_pixel(dev_image, map_to_int4(param2), color_out);
}]>, "vD3IVnVUn">;

//----------------------------------------------------------------------------//

def : OCLBuiltinImpl<blt_write_imagef, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  float4 color_out = map_channels_write(dev_image, param3);

  write_pixel(dev_image, map_to_int4(param2), color_out);
}]>, "vD12InVf">;

def : OCLBuiltinImpl<blt_write_imagef, TemplateStrategy<[{
  device_image_t *dev_image = *((device_image_t**)&param1);
  float4 color_out = map_channels_write(dev_image, param3);

  write_pixel(dev_image, map_to_int4(param2), color_out);
}]>, "vD3IVnVf">;

}

let Requirements = [device_image] in {

def : OCLBuiltinImpl<blt_get_image_width, TemplateStrategy<[{
  return (*(device_image_t **)&param1)->width;
}]>>;

def : OCLBuiltinImpl<blt_get_image_height, TemplateStrategy<[{
  return (*(device_image_t **)&param1)->height;
}]>>;

def : OCLBuiltinImpl<blt_get_image_depth, TemplateStrategy<[{
  return (*(device_image_t **)&param1)->depth;
}]>>;

def : OCLBuiltinImpl<blt_get_image_channel_data_type, TemplateStrategy<[{
  return (*(device_image_t **)&param1)->image_channel_data_type;
}]>>;

def : OCLBuiltinImpl<blt_get_image_channel_order, TemplateStrategy<[{
  return (*(device_image_t **)&param1)->image_channel_order;
}]>>;

def : OCLBuiltinImpl<blt_get_image_dim, TemplateStrategy<[{
  return (int2)(
    (*(device_image_t **)&param1)->width,
    (*(device_image_t **)&param1)->height
  );  
}]>, "VnD2I">;

def : OCLBuiltinImpl<blt_get_image_dim, TemplateStrategy<[{
  return (int4)(
    (*(device_image_t **)&param1)->width,
    (*(device_image_t **)&param1)->height,
    (*(device_image_t **)&param1)->depth,
    0
  );
}]>, "VnD3I">;

def : OCLBuiltinImpl<blt_get_image_array_size, TemplateStrategy<[{
  return (*(device_image_t **)&param1)->array_size;
}]>>;

}
