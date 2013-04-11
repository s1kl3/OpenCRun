
//
// The Geometric library.
//

__attribute__((overloadable,pure))
float4 __builtin_ocl_cross(float4 p0, float4 p1) {
  return (float4)(p0.y * p1.z - p0.z * p1.y,
                  p0.z * p1.x - p0.x * p1.z,
                  p0.x * p1.y - p0.y * p1.x,
                  0.0f);  
}

__attribute__((overloadable,pure))
float3 __builtin_ocl_cross(float3 p0, float3 p1) {
  return (float3)(p0.y * p1.z - p0.z * p1.y,
                  p0.z * p1.x - p0.x * p1.z,     
                  p0.x * p1.y - p0.y * p1.x);
}

__attribute__((overloadable,pure))
float __builtin_ocl_dot(float p0, float p1) {
  return p0 * p1;
}

__attribute__((overloadable,pure))
float __builtin_ocl_dot(float2 p0, float2 p1) {
  float2 tmp = p0 * p1;
  return tmp.x + tmp.y;
}

__attribute__((overloadable,pure))
float __builtin_ocl_dot(float3 p0, float3 p1) {
  float3 tmp = p0 * p1; 
  return tmp.x + tmp.y + tmp.z;
}

__attribute__((overloadable,pure))
float __builtin_ocl_dot(float4 p0, float4 p1) {
  float4 tmp = p0 * p1;
  return tmp.x + tmp.y + tmp.z + tmp.w;
}

__attribute__((overloadable,pure))
float __builtin_ocl_length(float p) {
  return __builtin_sqrt(__builtin_ocl_dot(p, p));
}

__attribute__((overloadable,pure))
float __builtin_ocl_length(float2 p) {
  return __builtin_sqrt(__builtin_ocl_dot(p, p));
}

__attribute__((overloadable,pure))
float __builtin_ocl_length(float3 p) {
  return __builtin_sqrt(__builtin_ocl_dot(p, p));
}

__attribute__((overloadable,pure))
float __builtin_ocl_length(float4 p) {
  return __builtin_sqrt(__builtin_ocl_dot(p, p));
}

__attribute__((overloadable,pure))
float __builtin_ocl_distance(float p0, float p1) {
  return __builtin_ocl_length(p0 - p1);
}

__attribute__((overloadable,pure))
float __builtin_ocl_distance(float2 p0, float2 p1) {
  return __builtin_ocl_length(p0 - p1);
}

__attribute__((overloadable,pure))
float __builtin_ocl_distance(float3 p0, float3 p1) {
  return __builtin_ocl_length(p0 - p1);
}

__attribute__((overloadable,pure))
float __builtin_ocl_distance(float4 p0, float4 p1) {
  return __builtin_ocl_length(p0 - p1);
}

__attribute__((overloadable,pure))
float __builtin_ocl_normalize(float p) {
  return (1.0f / __builtin_ocl_length(p)) * p;
}

__attribute__((overloadable,pure))
float2 __builtin_ocl_normalize(float2 p) {
  return (1.0f / __builtin_ocl_length(p)) * p;
}

__attribute__((overloadable,pure))
float3 __builtin_ocl_normalize(float3 p) {
  return (1.0f / __builtin_ocl_length(p)) * p;
}

__attribute__((overloadable,pure))
float4 __builtin_ocl_normalize(float4 p) {
  return (1.0f / __builtin_ocl_length(p)) * p;
}

__attribute__((overloadable,pure))
float __builtin_ocl_fast_length(float p) {
  return __builtin_ocl_half_sqrt(__builtin_ocl_dot(p, p));
}

__attribute__((overloadable,pure))
float __builtin_ocl_fast_length(float2 p) {
  return __builtin_ocl_half_sqrt(__builtin_ocl_dot(p, p));
}

__attribute__((overloadable,pure))
float __builtin_ocl_fast_length(float3 p) {
  return __builtin_ocl_half_sqrt(__builtin_ocl_dot(p, p));
}

__attribute__((overloadable,pure))
float __builtin_ocl_fast_length(float4 p) {
  return __builtin_ocl_half_sqrt(__builtin_ocl_dot(p, p));
}

__attribute__((overloadable,pure))
float __builtin_ocl_fast_distance(float p0, float p1) {
  return __builtin_ocl_fast_length(p0 - p1);
}

__attribute__((overloadable,pure))
float __builtin_ocl_fast_distance(float2 p0, float2 p1) {
  return __builtin_ocl_fast_length(p0 - p1);
}

__attribute__((overloadable,pure))
float __builtin_ocl_fast_distance(float3 p0, float3 p1) {
  return __builtin_ocl_fast_length(p0 - p1);
}

__attribute__((overloadable,pure))
float __builtin_ocl_fast_distance(float4 p0, float4 p1) {
  return __builtin_ocl_fast_length(p0 - p1);
}

__attribute__((overloadable,pure))
float __builtin_ocl_fast_normalize(float p) {
  return __builtin_ocl_half_rsqrt(__builtin_ocl_dot(p, p)) * p;
}

__attribute__((overloadable,pure))
float2 __builtin_ocl_fast_normalize(float2 p) {
  return __builtin_ocl_half_rsqrt(__builtin_ocl_dot(p, p)) * p;
}

__attribute__((overloadable,pure))
float3 __builtin_ocl_fast_normalize(float3 p) {
  return __builtin_ocl_half_rsqrt(__builtin_ocl_dot(p, p)) * p;
}

__attribute__((overloadable,pure))
float4 __builtin_ocl_fast_normalize(float4 p) {
  return __builtin_ocl_half_rsqrt(__builtin_ocl_dot(p, p)) * p;
}
