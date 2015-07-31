
#include "AsyncCopyThread.h"

using namespace opencrun::cpu;

void AsyncCopyThread::Run() {
  unsigned char *dst = thrd_data->dst;
  const unsigned char *src = thrd_data->src;
  size_t num_gentypes = thrd_data->num_gentypes;
  size_t sz_gentype = thrd_data->sz_gentype;
  size_t dst_stride = thrd_data->dst_stride;
  size_t src_stride = thrd_data->src_stride;

  if (src_stride) {
    // In this case the thread performs a strided copy 
    // from a global memory location to a local one.
    for (size_t I = 0; I < num_gentypes; ++I) {
      for (size_t J = 0; J < sz_gentype; ++J, ++src) {
        dst[I * sz_gentype + J] = *src;
        if ((J == (sz_gentype - 1)) && (I < (num_gentypes - 1)))
          src += src_stride * sz_gentype;
      }
    }  
  } else if (dst_stride) {
    // In this case the thread performs a strided copy
    // from a local memory location to a global one.
    for (size_t I = 0; I < num_gentypes; ++I) {
      for (size_t J = 0; J < sz_gentype; ++J, ++dst) {
        *dst = src[I * sz_gentype + J];
        if ((J == (sz_gentype - 1)) && (I < (num_gentypes - 1)))
          dst += dst_stride * sz_gentype;
      }
    }
  } else {
    // Otherwise normal copy is performed.
    size_t num_bytes = num_gentypes * sz_gentype;
    std::memcpy(dst, src, num_bytes);
  }
}
