
#include "opencrun/Core/Sampler.h"
#include "opencrun/Core/Context.h"

using namespace opencrun;

//
// SamplerBuilder implementation.
//

#define RETURN_WITH_ERROR(VAR) \
  {                            \
  if(VAR)                      \
    *VAR = this->ErrCode;      \
  return NULL;                 \
  }

SamplerBuilder &SamplerBuilder::SetNormalizedCoords(bool Enabled) {
  NormalizedCoords = Enabled;

  return *this;
}

SamplerBuilder &SamplerBuilder::SetAddressNone(bool Enabled) {
  if(Enabled)
    return SetAddressingMode(Sampler::AddressNone);

  return *this;
}

SamplerBuilder &SamplerBuilder::SetAddressClampToEdge(bool Enabled) {
  if(Enabled)
    return SetAddressingMode(Sampler::AddressClampToEdge);

  return *this;
}

SamplerBuilder &SamplerBuilder::SetAddressClamp(bool Enabled) {
  if(Enabled)
    return SetAddressingMode(Sampler::AddressClamp);

  return *this;
}

SamplerBuilder &SamplerBuilder::SetAddressRepeat(bool Enabled) {
  if(Enabled)
    return SetAddressingMode(Sampler::AddressRepeat);

  return *this;
}

SamplerBuilder &SamplerBuilder::SetAddressMirroredRepeat(bool Enabled) {
  if(Enabled)
    return SetAddressingMode(Sampler::AddressMirroredRepeat);

  return *this;
}

SamplerBuilder &SamplerBuilder::SetFilterNearest(bool Enabled) {
  if(Enabled)
    return SetFilterMode(Sampler::FilterNearest);
  
  return *this;
}

SamplerBuilder &SamplerBuilder::SetFilterLinear(bool Enabled) {
  if(Enabled)
    return SetFilterMode(Sampler::FilterLinear);
  
  return *this;
}

SamplerBuilder &SamplerBuilder::SetAddressingMode(Sampler::AddressingMode AddrMode) {
  if(this->AddrMode != Sampler::NoAddressing)
    return NotifyError(CL_INVALID_VALUE, "multiple addressing modes not allowed");

  this->AddrMode = AddrMode; 

  return *this;
}

SamplerBuilder &SamplerBuilder::SetFilterMode(Sampler::FilterMode FltrMode) {
  if(this->FltrMode != Sampler::NoFilter)
    return NotifyError(CL_INVALID_VALUE, "multiple filter mode not allowed");

  this->FltrMode = FltrMode;

  return *this;
}

SamplerBuilder &SamplerBuilder::NotifyError(cl_int ErrCode, const char *Msg) {
  Ctx.ReportDiagnostic(Msg);
  this->ErrCode = ErrCode;

  return *this;
}

Sampler *SamplerBuilder::Create(cl_int *ErrCode) {
  if(this->ErrCode != CL_SUCCESS)
    RETURN_WITH_ERROR(ErrCode);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return new Sampler(Ctx, NormalizedCoords, AddrMode, FltrMode);
}
