
#include "opencrun/Core/Kernel.h"
#include "opencrun/Core/Context.h"
#include "opencrun/Core/Device.h"
#include "opencrun/Passes/AllPasses.h"

#include "llvm/IR/Constants.h"
#include "llvm/PassManager.h"
#include "llvm/ADT/StringSwitch.h"

using namespace opencrun;

#define RETURN_WITH_ERROR(ERR, MSG)   \
  {                                   \
  GetContext().ReportDiagnostic(MSG); \
  return ERR;                         \
  }

KernelDescriptor::KernelDescriptor(llvm::StringRef N, Program &P,
                                   KernelInfoContainer &&I)
 : Name(N), Infos(std::move(I)), Prog(&P), LocalArgsSz(0) {
  for (const auto &V : Infos)
    V.first->RegisterKernel(*this);
}

KernelDescriptor::~KernelDescriptor() {
  Prog->DetachKernel(*this);
  for (const auto &V : Infos)
    V.first->UnregisterKernel(*this);
}

bool KernelDescriptor::hasRequiredWorkGroupSizes(const Device *Dev) const {
  // Retrive kernel metadata and check if the "reqd_work_group_size"
  // node is present.
  ModuleInfo Info(*getFunction(Dev)->getParent());
  return Info.getKernelInfo(Name).hasRequiredWorkGroupSizes() ?
    true : false;
}

bool KernelDescriptor::
getRequiredWorkGroupSizes(llvm::SmallVectorImpl<size_t> &Sizes,
                          const Device *Dev) const {
  if (!hasRequiredWorkGroupSizes(Dev))
    return false;

  // Retrive kernel metadata and extract values from the "reqd_work_group_size"
  // node.
  ModuleInfo Info(*getFunction(Dev)->getParent());
  KernelArgInfo WGSzInfo = Info.getKernelInfo(Name).getRequiredWorkGroupSizes();
  unsigned NumWGSz = WGSzInfo.getNumArguments();

  Sizes.clear();
  Sizes.resize(NumWGSz);

  for (unsigned I = 0, E = NumWGSz; I != E; ++I) {
    size_t Sz = WGSzInfo.getArgumentAs<llvm::ConstantInt>(I)->getZExtValue();
    Sizes[I] = Sz;
  }

  return true;
}

bool KernelDescriptor::
getMaxWorkGroupSize(size_t &Size, const Device *Dev) const {
  const Footprint &FP = Dev->getKernelFootprint(*this);

  Size = FP.GetMaxWorkGroupSize(Dev->GetPrivateMemorySize());

  size_t DevMaxSize = Dev->GetMaxWorkGroupSize();
  if(Size > DevMaxSize)
    Size = DevMaxSize;

  return true;
}

bool KernelDescriptor::
getMinLocalMemoryUsage(size_t &Size, const Device *Dev) const {
  const Footprint &FP = Dev->getKernelFootprint(*this);

  Size = FP.GetLocalMemoryUsage();

  return true;
}

bool KernelDescriptor::
getMinPrivateMemoryUsage(size_t &Size, const Device *Dev) const {
  const Footprint &FP = Dev->getKernelFootprint(*this);

  Size = FP.GetPrivateMemoryUsage();

  return true;
}

Kernel::Kernel(const Kernel &K) : Desc(K.Desc) {
  Arguments.reserve(K.Arguments.size());
  for (unsigned I = 0, E = K.Arguments.size(); I != E; ++I) {
    KernelArg *Arg = K.Arguments[I];
    switch (Arg->GetType()) {
    default: llvm_unreachable(0);
    case KernelArg::BufferArg:
      Arg = new BufferKernelArg(llvm::cast<BufferKernelArg>(*Arg));
      break;
    case KernelArg::LocalBufferArg:
      Arg = new LocalBufferKernelArg(llvm::cast<LocalBufferKernelArg>(*Arg));
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
  //llvm::DeleteContainerPointers(Arguments);
}

llvm::StringRef Kernel::GetArgName(unsigned I) const {
  KernelArgInfo ArgsName = Desc->getKernelInfo().getArgsName();
  return ArgsName.getArgumentAs<llvm::MDString>(I)->getString();
}

unsigned Kernel::GetArgCount() const {
  return Desc->getKernelInfo().getSignature().getNumArguments();
}

bool Kernel::AreAllArgsSpecified() const {
  llvm::Function *Kern = Desc->getKernelInfo().getFunction();
  llvm::FunctionType *FTy = Kern->getFunctionType();

  llvm::Argument &LastArg = *--(Kern->arg_end());
  if (LastArg.getName().equals(Kern->getName().str() + ".locals"))
    return GetArgCount() == (FTy->getNumParams() - 1) ? true : false;

  return GetArgCount() == FTy->getNumParams() ? true : false;
}

cl_kernel_arg_address_qualifier Kernel::GetArgAddressQualifier(unsigned I) const {
  KernelArgInfo ArgsAS = Desc->getKernelInfo().getArgsAddrSpace();

  unsigned AS = ArgsAS.getArgumentAs<llvm::ConstantInt>(I)->getZExtValue();

  switch(AS) {
  case opencl::AS_Global:
    return CL_KERNEL_ARG_ADDRESS_GLOBAL;
  case opencl::AS_Constant:
    return CL_KERNEL_ARG_ADDRESS_CONSTANT;
  case opencl::AS_Local:
    return CL_KERNEL_ARG_ADDRESS_LOCAL;
  case opencl::AS_Private:
  case opencl::AS_Invalid:
    return CL_KERNEL_ARG_ADDRESS_PRIVATE;
  default:
    llvm_unreachable("Unexpected address space!");
  }
}

cl_kernel_arg_access_qualifier Kernel::GetArgAccessQualifier(unsigned I) const {
  KernelArgInfo ArgsAccessQual = Desc->getKernelInfo().getArgsAccessQual();

  llvm::StringRef ArgAccStr =
    ArgsAccessQual.getArgumentAs<llvm::MDString>(I)->getString();

  return llvm::StringSwitch<cl_kernel_arg_access_qualifier>(ArgAccStr)
          .Case("read_only", CL_KERNEL_ARG_ACCESS_READ_ONLY)
          .Case("write_only", CL_KERNEL_ARG_ACCESS_WRITE_ONLY)
          .Case("none", CL_KERNEL_ARG_ACCESS_NONE)
          .Default(CL_KERNEL_ARG_ACCESS_NONE);
}

llvm::StringRef Kernel::GetArgTypeName(unsigned I) const {
  KernelArgInfo ArgsType = Desc->getKernelInfo().getArgsType();
  return ArgsType.getArgumentAs<llvm::MDString>(I)->getString();
}

cl_kernel_arg_type_qualifier Kernel::GetArgTypeQualifier(unsigned I) const {
  KernelArgInfo ArgsTypeQual = Desc->getKernelInfo().getArgsTypeQual();

  llvm::StringRef TyQlsStr =
    ArgsTypeQual.getArgumentAs<llvm::MDString>(I)->getString();

  cl_kernel_arg_type_qualifier TyQls = CL_KERNEL_ARG_TYPE_NONE;

  if (TyQlsStr.find("const") != llvm::StringRef::npos)
    TyQls |= CL_KERNEL_ARG_TYPE_CONST;
  if (TyQlsStr.find("restrict") != llvm::StringRef::npos)
    TyQls |= CL_KERNEL_ARG_TYPE_RESTRICT;
  if (TyQlsStr.find("volatile") != llvm::StringRef::npos)
    TyQls |= CL_KERNEL_ARG_TYPE_VOLATILE;

  return TyQls;
}

static bool isBufferArg(opencl::Type Ty) {
  if (!Ty.isPointer())
    return false;

  auto Q = Ty.getElementType().getQualifiers();
  return Q.getAddressSpace() != opencl::AS_Local;
}

static bool isLocalBufferArg(opencl::Type Ty) {
  if (!Ty.isPointer())
    return false;

  auto Q = Ty.getElementType().getQualifiers();
  return Q.getAddressSpace() == opencl::AS_Local;
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

  opencl::Type ArgTy = Desc->getKernelInfo().getSignature().getArgument(I);

  if (isBufferArg(ArgTy))
    return SetBufferArg(I, Size, Arg);

  if (isLocalBufferArg(ArgTy))
    return SetLocalBufferArg(I, Size, Arg);

  if (isImageArg(ArgTy))
    return SetImageArg(I, Size, Arg);

  if (isSamplerArg(ArgTy))
    return SetSamplerArg(I, Size, Arg);

  if (isByValueArg(ArgTy))
    return SetByValueArg(I, Size, Arg);

  llvm_unreachable("Unknown argument type");
}


cl_int Kernel::SetBufferArg(unsigned I, size_t Size, const void *Arg) {
  Buffer *Buf;

  Context &Ctx = GetContext();

  KernelSignature Sign = Desc->getKernelInfo().getSignature();

  opencl::Type PointeeTy = Sign.getArgument(I).getElementType();

  opencl::AddressSpace AddrSpace = PointeeTy.getQualifiers().getAddressSpace();

  switch(AddrSpace) {
  case opencl::AS_Global:
  case opencl::AS_Constant:
    if (Size != sizeof(cl_mem))
      RETURN_WITH_ERROR(CL_INVALID_ARG_SIZE,
                        "kernel argument size does not match");

    std::memcpy(&Buf, Arg, Size);

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

cl_int Kernel::SetLocalBufferArg(unsigned I, size_t Size, const void *Arg) {
  if (Arg)
    RETURN_WITH_ERROR(CL_INVALID_ARG_VALUE, "cannot set a local pointer");

  if (!Size)
    RETURN_WITH_ERROR(CL_INVALID_ARG_SIZE, "local buffer size unspecified");

  ThisLock.acquire();
  Arguments[I] = new LocalBufferKernelArg(I, Size);
  ThisLock.release();

  return CL_SUCCESS;
}

cl_int Kernel::SetImageArg(unsigned I, size_t Size, const void *Arg) {
  Image *Img;

  Context &Ctx = GetContext();

  opencl::Type ImgTy = Desc->getKernelInfo().getSignature().getArgument(I);

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
  // The expected data type for the parameter is extracted from the
  // FunctionType of the kernel function. All required error checking
  // has been already performed by the SetArg function.
  llvm::Function *Kern = Desc->getKernelInfo().getFunction();
  llvm::FunctionType *FTy = Kern->getFunctionType();
  const llvm::DataLayout *DL = Kern->getParent()->getDataLayout();
  
  uint64_t ArgSz;
  llvm::Type *ParamTy = FTy->getParamType(I);
  // For 3-component vector data types, the size of the data type is
  // 4 * sizeof(component). This means that a 3-component vector data
  // type will be aligned to a 4 * sizeof(component) boundary.
  if (ParamTy->isVectorTy() && (ParamTy->getVectorNumElements() == 3))
    ArgSz = DL->getTypeStoreSize(ParamTy->getVectorElementType()) * 4;
  else
    ArgSz = DL->getTypeStoreSize(FTy->getParamType(I));

  if (Size != ArgSz)
    RETURN_WITH_ERROR(CL_INVALID_ARG_SIZE,
                      "kernel argument size does not match");

  ThisLock.acquire();
  Arguments[I] = new ByValueKernelArg(I, Arg, Size);
  ThisLock.release();

  return CL_SUCCESS;
}
