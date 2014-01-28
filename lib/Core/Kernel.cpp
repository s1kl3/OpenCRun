
#include "opencrun/Core/Kernel.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/Device.h"
#include "opencrun/Passes/AllPasses.h"

#include "llvm/IR/Constants.h"
#include "llvm/PassManager.h"
#include "llvm/ADT/StringSwitch.h"

using namespace opencrun;

#define RETURN_WITH_ERROR(ERR, MSG)  \
  {                                  \
  Context &Ctx = Prog->GetContext(); \
  Ctx.ReportDiagnostic(MSG);         \
  return ERR;                        \
  }

Kernel::~Kernel() {
  Prog->UnregisterKernel(*this);

  for(CodesContainer::iterator I = Codes.begin(),
                               E = Codes.end();
                               I != E;
                               ++I) {
    Device &Dev = *I->first;

    Dev.UnregisterKernel(*this);
  }

  llvm::DeleteContainerPointers(Arguments);
}

static bool isBufferArg(opencl::Type Ty) {
  return Ty.isPointer();
}

static bool isByValueArg(opencl::Type Ty) {
  return !Ty.isPointer() && !Ty.isImage();
}

static bool isImageArg(opencl::Type Ty) {
  return Ty.isImage() && Ty.getImageClass() != opencl::Type::Sampler;
}

static bool isSamplerArg(opencl::Type Ty) {
  return Ty.isImage() && Ty.getImageClass() == opencl::Type::Sampler;
}

cl_int Kernel::SetArg(unsigned I, size_t Size, const void *Arg) {
  if (I >= GetArgCount())
    RETURN_WITH_ERROR(CL_INVALID_ARG_INDEX,
                      "argument number exceed kernel argument count");

  opencl::Type ArgTy = GetSignature().getArgument(I);

  if (isBufferArg(ArgTy))
    return SetBufferArg(I, Size, Arg);

  if (isImageArg(ArgTy))
    return SetImageArg(I, Size, Arg);

  if (isSamplerArg(ArgTy))
    return SetSamplerArg(I, Size, Arg);

  if (isByValueArg(ArgTy))
    return SetByValueArg(I, Size, Arg);

  llvm_unreachable("Unknown argument type");
}

bool Kernel::GetMaxWorkGroupSize(size_t &Size, Device *Dev) {
  const Footprint &FP = Dev->ComputeKernelFootprint(*this);

  Size = FP.GetMaxWorkGroupSize(Dev->GetPrivateMemorySize());

  size_t DevMaxSize = Dev->GetMaxWorkGroupSize();
  if(Size > DevMaxSize)
    Size = DevMaxSize;

  return true;
}

bool Kernel::GetMinLocalMemoryUsage(size_t &Size, Device *Dev) {
  const Footprint &FP = Dev->ComputeKernelFootprint(*this);

  Size = FP.GetLocalMemoryUsage();

  return true;
}

bool Kernel::GetMinPrivateMemoryUsage(size_t &Size, Device *Dev) {
  const Footprint &FP = Dev->ComputeKernelFootprint(*this);

  Size = FP.GetPrivateMemoryUsage();

  return true;
}

cl_int Kernel::SetBufferArg(unsigned I, size_t Size, const void *Arg) {
  Buffer *Buf;

  Context &Ctx = GetContext();

  opencl::Type PointeeTy = GetSignature().getArgument(I).getElementType();

  opencl::AddressSpace AddrSpace = PointeeTy.getQualifiers().getAddressSpace();

  switch(AddrSpace) {
  case opencl::AS_Global:
  case opencl::AS_Constant:
    if (Size != sizeof(cl_mem))
      RETURN_WITH_ERROR(CL_INVALID_ARG_SIZE,
                        "kernel argument size does not match");

    std::memcpy(&Buf, Arg, Size);

    break;

  case opencl::AS_Local:
    if (Arg)
      RETURN_WITH_ERROR(CL_INVALID_ARG_VALUE, "cannot set a local pointer");

    if (!Size)
      RETURN_WITH_ERROR(CL_INVALID_ARG_SIZE, "local buffer size unspecified");

    Buf = Ctx.CreateVirtualBuffer(Size, 
                                  MemoryObj::ReadWrite, 
                                  MemoryObj::HostNoProtection);

    break;

  default:
    llvm_unreachable("Invalid address space");
  }

  if (Buf) {
    if (Buf->GetContext() != Ctx)
      RETURN_WITH_ERROR(CL_INVALID_MEM_OBJECT,
                        "buffer and kernel contexts do not match");
  }

  ThisLock.acquire();
  Arguments[I] = new BufferKernelArg(I, Buf, AddrSpace);
  ThisLock.release();

  return CL_SUCCESS;
}

cl_int Kernel::SetImageArg(unsigned I, size_t Size, const void *Arg) {
  Image *Img;

  Context &Ctx = GetContext();

  opencl::Type ImgTy = GetSignature().getArgument(I);

  opencl::ImageAccess ImgAccess = ImgTy.getQualifiers().getImageAccess(); 
  
  // Function arguments of type image2d_t, image3d_t, image2d_array_t, 
  // image1d_t, image1d_buffer_t, and image1d_array_t refer to image 
  // memory objects allocated in the __global address space.
  if(Size != sizeof(cl_mem))
    RETURN_WITH_ERROR(CL_INVALID_ARG_SIZE,
                      "kernel argument size does not match");

  std::memcpy(&Img, Arg, Size);

  if(Img) {
    if(Img->GetContext() != Ctx)
      RETURN_WITH_ERROR(CL_INVALID_MEM_OBJECT,
                        "image and kernel contexts do not match");
  }

  ThisLock.acquire();
  Arguments[I] = new ImageKernelArg(I, Img, ImgAccess);
  ThisLock.release();

  return CL_SUCCESS;
}

cl_int Kernel::SetSamplerArg(unsigned I, size_t Size, const void *Arg) {
  Sampler *Smplr;

  Context &Ctx = GetContext();

  // The sampler type cannot be used with the __local and __global 
  // address space qualifiers.
  if(Size != sizeof(cl_sampler))
    RETURN_WITH_ERROR(CL_INVALID_ARG_SIZE,
                      "kernel argument size does not match");
  
  std::memcpy(&Smplr, Arg, Size);

  if(Smplr) {
    if(Smplr->GetContext() != Ctx)
      RETURN_WITH_ERROR(CL_INVALID_SAMPLER,
                        "sampler and kernel contexts do not match");
  }

  ThisLock.acquire();
  Arguments[I] = new SamplerKernelArg(I, Smplr);
  ThisLock.release();

  return CL_SUCCESS;
}

cl_int Kernel::SetByValueArg(unsigned I, size_t Size, const void *Arg) {
  // TODO: compute argument type size in order to match the parameter.
  ThisLock.acquire();
  Arguments[I] = new ByValueKernelArg(I, Arg, Size);
  ThisLock.release();

  return CL_SUCCESS;
}

KernelSignature Kernel::GetSignature() const {
  // All stored functions share the same signature, use the first.
  CodesContainer::const_iterator J = Codes.begin();
  assert(J != Codes.end());
  llvm::Function &Kern = *J->second;
  llvm::Module &Mod = *Kern.getParent();

  return ModuleInfo(Mod).getKernelInfo(Kern.getName()).getSignature();
}
