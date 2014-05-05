
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

Kernel::LifetimeHandler::LifetimeHandler(Kernel &KernHandle)
 : KernHandle(KernHandle) {
  KernHandle.GetProgram().RegisterKernel(KernHandle);
  KernHandle.RegisterToDevices();
}

Kernel::LifetimeHandler::~LifetimeHandler() {
  KernHandle.UnregisterFromDevices();
  KernHandle.GetProgram().UnregisterKernel(KernHandle);
}

Kernel::Kernel(const Kernel &K)
 : Prog(K.Prog), Codes(K.Codes), Lifetime(K.Lifetime) {
  for (unsigned I = 0, E = K.Arguments.size(); I != E; ++I) {
    llvm::KernelArg *Arg = K.Arguments[I];
    switch (Arg->GetType()) {
    case KernelArg::BufferArg:
      Arg = new BufferKernelArg(llvm::cast<BufferKernelArg>(*Arg));
      break;
    case KernelArg::ImageArg:
      Arg = new ImageKernelArg(llvm::cast<ImageKernelArg>(*Arg));
      break;
    case KernelArg::SamplerArg:
      Arg = new SamplerKernelArg(llvm::cast<SamplerKernelArg>(*Arg));
      break;
    case KernelArg::ByValueArg:
      Arg = new ByValueKernelArg(llvm::cast<ByValueKernelArg>(*Arg));
      break;
    }
    Arguments.push_back(Arg);
  }
} 

Kernel::~Kernel() {
  llvm::DeleteContainerPointers(Arguments);
}

llvm::StringRef Kernel::GetArgName(unsigned I) const {
  return GetInfo().getArgsName().getArgumentAs<llvm::MDString>(I)->getString();
}

void Kernel::RegisterToDevices() {
  for (CodesContainer::const_iterator I = Codes.begin(),
       E = Codes.end(); I != E; ++I)
    I->first->RegisterKernel(*this);
}

void Kernel::UnregisterFromDevices() {
  for (CodesContainer::const_iterator I = Codes.begin(),
       E = Codes.end(); I != E; ++I)
    I->first->UnregisterKernel(*this);
}

cl_kernel_arg_address_qualifier Kernel::GetArgAddressQualifier(unsigned I) const {
  opencl::AddressSpace AS;

  AS = static_cast<opencl::AddressSpace>(
      GetInfo().getArgsAddrSpace().getArgumentAs<llvm::ConstantInt>(I)->getZExtValue());

  switch(AS) {
  case opencl::AS_Global:
    return CL_KERNEL_ARG_ADDRESS_GLOBAL;
  case opencl::AS_Constant:
    return CL_KERNEL_ARG_ADDRESS_CONSTANT;
  case opencl::AS_Local:
    return CL_KERNEL_ARG_ADDRESS_LOCAL;
  case opencl::AS_Private:
  case opencl::AS_Invalid:
  default:
    return CL_KERNEL_ARG_ADDRESS_PRIVATE;
  }
}

cl_kernel_arg_access_qualifier Kernel::GetArgAccessQualifier(unsigned I) const {
  cl_kernel_arg_access_qualifier ArgAccQls;
  llvm::StringRef ArgAccStr;

  ArgAccStr = GetInfo().getArgsAccessQual().getArgumentAs<llvm::MDString>(I)->getString();
  ArgAccQls = llvm::StringSwitch<cl_kernel_arg_access_qualifier>(ArgAccStr)
    .Case("read_only", CL_KERNEL_ARG_ACCESS_READ_ONLY)
    .Case("write_only", CL_KERNEL_ARG_ACCESS_WRITE_ONLY)
    .Case("none", CL_KERNEL_ARG_ACCESS_NONE)
    .Default(CL_KERNEL_ARG_ACCESS_NONE);

  return ArgAccQls;
}

llvm::StringRef Kernel::GetArgTypeName(unsigned I) const {
  return GetInfo().getArgsType().getArgumentAs<llvm::MDString>(I)->getString();
}

cl_kernel_arg_type_qualifier Kernel::GetArgTypeQualifier(unsigned I) const {
  cl_kernel_arg_type_qualifier TyQls = CL_KERNEL_ARG_TYPE_NONE;
  llvm::StringRef TyQlsStr;
  
  TyQlsStr = GetInfo().getArgsTypeQual().getArgumentAs<llvm::MDString>(I)->getString();

  if(TyQlsStr.find("const") != llvm::StringRef::npos)
    TyQls |= CL_KERNEL_ARG_TYPE_CONST;
  else if(TyQlsStr.find("restrict") != llvm::StringRef::npos)
    TyQls |= CL_KERNEL_ARG_TYPE_RESTRICT;
  else if(TyQlsStr.find("volatile") != llvm::StringRef::npos)
    TyQls |= CL_KERNEL_ARG_TYPE_VOLATILE;

  return TyQls;
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

KernelInfo Kernel::GetInfo() const {
  // All stored functions share the same signature, use the first.
  CodesContainer::const_iterator J = Codes.begin();
  assert(J != Codes.end());
  llvm::Function &Kern = *J->second;
  llvm::Module &Mod = *Kern.getParent();

  return ModuleInfo(Mod).getKernelInfo(Kern.getName());
}
