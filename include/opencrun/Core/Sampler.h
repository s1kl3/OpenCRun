
#ifndef OPENCRUN_CORE_SAMPLER_H
#define OPENCRUN_CORE_SAMPLER_H

#include "CL/opencl.h"

#include "opencrun/Util/MTRefCounted.h"

struct _cl_sampler { };

namespace opencrun {

class Context;

class Sampler : public _cl_sampler, public MTRefCountedBase<Sampler> {
public:
  enum AddressingMode {
    AddressNone             = CL_ADDRESS_NONE,
    AddressClampToEdge      = CL_ADDRESS_CLAMP_TO_EDGE,
    AddressClamp            = CL_ADDRESS_CLAMP,
    AddressRepeat           = CL_ADDRESS_REPEAT,
    AddressMirroredRepeat   = CL_ADDRESS_MIRRORED_REPEAT,
    NoAddressing            = 0
  };

  enum FilterMode {
    FilterNearest   = CL_FILTER_NEAREST,
    FilterLinear    = CL_FILTER_LINEAR,
    NoFilter        = 0
  };

public:
  static bool classof(const _cl_sampler *Smplr) { return true; }

private:
  Sampler(Context &Ctx,
          bool NormalizedCoords,
          AddressingMode AddrMode,
          FilterMode FltrMode)
    : Ctx(&Ctx),
      NormalizedCoords(NormalizedCoords),
      AddrMode(AddrMode),
      FltrMode(FltrMode) { }

public:
  Context &GetContext() const { return *Ctx; }
  bool GetNormalizedCoords() const { return NormalizedCoords; }
  AddressingMode GetAddressingMode() const { return AddrMode; }
  FilterMode GetFilterMode() const { return FltrMode; }

private:
  llvm::IntrusiveRefCntPtr<Context> Ctx;

  bool NormalizedCoords;
  AddressingMode AddrMode;
  FilterMode FltrMode;

  friend class SamplerBuilder;
};

class SamplerBuilder {
public:
  SamplerBuilder(Context &Ctx) : Ctx(Ctx),
                                 ErrCode(CL_SUCCESS),
                                 NormalizedCoords(false),
                                 AddrMode(Sampler::NoAddressing),
                                 FltrMode(Sampler::NoFilter) { }

public:
  SamplerBuilder &SetNormalizedCoords(bool Enabled);
  SamplerBuilder &SetAddressNone(bool Enabled);
  SamplerBuilder &SetAddressClampToEdge(bool Enabled);
  SamplerBuilder &SetAddressClamp(bool Enabled);
  SamplerBuilder &SetAddressRepeat(bool Enabled);
  SamplerBuilder &SetAddressMirroredRepeat(bool Enabled);
  SamplerBuilder &SetFilterNearest(bool Enabled);
  SamplerBuilder &SetFilterLinear(bool Enabled);

  Sampler *Create(cl_int *ErrCode = NULL);

private:
  SamplerBuilder &SetAddressingMode(Sampler::AddressingMode AddrMode);
  SamplerBuilder &SetFilterMode(Sampler::FilterMode FltrMode);
  SamplerBuilder &NotifyError(cl_int ErrCode, const char *Msg = "");

private:
  Context &Ctx;
  cl_int ErrCode;
  
  bool NormalizedCoords;
  Sampler::AddressingMode AddrMode;
  Sampler::FilterMode FltrMode;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_SAMPLER_H
