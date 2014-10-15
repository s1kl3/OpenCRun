
#include "opencrun/Core/Context.h"
#include "opencrun/Core/CommandQueue.h"
#include "opencrun/Core/Device.h"
#include "opencrun/System/Env.h"

#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/Support/raw_ostream.h"

using namespace opencrun;

#define RETURN_WITH_ERROR(VAR, ERRCODE, MSG) \
  {                                          \
  ReportDiagnostic(MSG);                     \
  if(VAR)                                    \
    *VAR = ERRCODE;                          \
  return NULL;                               \
  }

Context::Context(Platform &Plat,
                 llvm::SmallVector<Device *, 2> &Devs,
                 ContextErrCallbackClojure &ErrNotify) :
  Devices(Devs),
  UserDiag(ErrNotify) {
  if(sys::HasEnv("OPENCRUN_INTERNAL_DIAGNOSTIC")) {
    clang::TextDiagnosticPrinter *DiagClient;
    
    DiagOptions.ShowColors = true;

    DiagClient = new clang::TextDiagnosticPrinter(llvm::errs(), &DiagOptions);
    DiagClient->setPrefix("opencrun");

    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagIDs;

    DiagIDs = new clang::DiagnosticIDs();
    Diag.reset(new clang::DiagnosticsEngine(DiagIDs, &DiagOptions, DiagClient));
  }

  // Implicit retain on each associated sub-device.
  for(device_iterator I = device_begin(), E = device_end(); I != E; ++I)
    if((*I)->IsSubDevice())
      (*I)->Retain();
}

Context::~Context() {
  // Implicit release on each associated sub-device.
  for(device_iterator I = device_begin(), E = device_end(); I != E; ++I)
    if((*I)->IsSubDevice())
      (*I)->Release();
}

CommandQueue *Context::GetQueueForDevice(Device &Dev,
                                         bool OutOfOrder,
                                         bool EnableProfile,
                                         cl_int *ErrCode) {
  if(!std::count(Devices.begin(), Devices.end(), &Dev))
    RETURN_WITH_ERROR(ErrCode,
                      CL_INVALID_DEVICE,
                      "device not associated with this context");

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  if(OutOfOrder)
    return new OutOfOrderQueue(*this, Dev, EnableProfile);
  else
    return new InOrderQueue(*this, Dev, EnableProfile);
}

HostBuffer *Context::CreateHostBuffer(Buffer *Parent,
                                      size_t Offset,
                                      size_t Size,
                                      void *HostPtr,
                                      MemoryObj::AccessProtection AccessProt,
                                      MemoryObj::HostAccessProtection HostAccessProt,
                                      cl_int *ErrCode) {
  HostBuffer *Buf = new HostBuffer(*this,
                                   Parent,
                                   Offset,
                                   Size, 
                                   HostPtr, 
                                   MemoryObj::UseHostPtr, 
                                   AccessProt, 
                                   HostAccessProt);
  bool Rollback = false;

  for(device_iterator I = Devices.begin(),
                      E = Devices.end();
                      I != E && !Rollback;
                      ++I)
    if(!(*I)->CreateHostBuffer(*Buf))
      Rollback = true;

  if(Rollback) {
    for(device_iterator I = Devices.begin(), E = Devices.end(); I != E; ++I)
      (*I)->DestroyMemoryObj(*Buf);

    delete Buf;

    RETURN_WITH_ERROR(ErrCode,
                      CL_OUT_OF_RESOURCES,
                      "failed allocating resources for device buffer");
  }
  
  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return Buf;
}

HostAccessibleBuffer *Context::CreateHostAccessibleBuffer(
                                 Buffer *Parent,
                                 size_t Offset,
                                 size_t Size,
                                 void *Src,
                                 MemoryObj::AccessProtection AccessProt,
                                 MemoryObj::HostAccessProtection HostAccessProt,
                                 cl_int *ErrCode) {
  MemoryObj::HostPtrUsageMode HostPtrMode = MemoryObj::AllocHostPtr;
  if(Src)
    HostPtrMode = static_cast<MemoryObj::HostPtrUsageMode>(
                    static_cast<int>(HostPtrMode) | MemoryObj::CopyHostPtr
                  );
    
  HostAccessibleBuffer *Buf = new HostAccessibleBuffer(*this,
                                                       Parent,
                                                       Offset,
                                                       Size, 
                                                       Src, 
                                                       HostPtrMode, 
                                                       AccessProt, 
                                                       HostAccessProt);
  bool Rollback = false;

  for(device_iterator I = Devices.begin(),
                      E = Devices.end();
                      I != E && !Rollback;
                      ++I)
    if(!(*I)->CreateHostAccessibleBuffer(*Buf))
      Rollback = true;

  if(Rollback) {
    for(device_iterator I = Devices.begin(), E = Devices.end(); I != E; ++I)
      (*I)->DestroyMemoryObj(*Buf);

    delete Buf;

    RETURN_WITH_ERROR(ErrCode,
                      CL_OUT_OF_RESOURCES,
                      "failed allocating resources for device buffer");

  }
  
  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return Buf;
}

DeviceBuffer *Context::CreateDeviceBuffer(
                         Buffer *Parent,
                         size_t Offset,
                         size_t Size,
                         void *Src,
                         MemoryObj::AccessProtection AccessProt,
                         MemoryObj::HostAccessProtection HostAccessProt,
                         cl_int *ErrCode) {
  MemoryObj::HostPtrUsageMode HostPtrMode = MemoryObj::NoHostPtrUsage;
  if(Src)
    HostPtrMode = static_cast<MemoryObj::HostPtrUsageMode>(
                    static_cast<int>(HostPtrMode) | MemoryObj::CopyHostPtr
                  );
    
  DeviceBuffer *Buf = new DeviceBuffer(*this,
                                       Parent,
                                       Offset,
                                       Size, 
                                       Src, 
                                       HostPtrMode,
                                       AccessProt, 
                                       HostAccessProt);
  bool Rollback = false;

  for(device_iterator I = Devices.begin(),
                      E = Devices.end();
                      I != E && !Rollback;
                      ++I)
    if(!(*I)->CreateDeviceBuffer(*Buf))
      Rollback = true;

  if(Rollback) {
    for(device_iterator I = Devices.begin(), E = Devices.end(); I != E; ++I)
      (*I)->DestroyMemoryObj(*Buf);

    delete Buf;

    RETURN_WITH_ERROR(ErrCode,
                      CL_OUT_OF_RESOURCES,
                      "failed allocating resources for device buffer");

  }
  
  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return Buf;
}

Buffer *Context::CreateVirtualBuffer(size_t Size,
                                     MemoryObj::AccessProtection AccessProt,
                                     MemoryObj::HostAccessProtection HostAccessProt,
                                     cl_int *ErrCode) {
  if(!Size)
    RETURN_WITH_ERROR(ErrCode, CL_INVALID_BUFFER_SIZE, "buffer size is zero");

  VirtualBuffer *Buf = new VirtualBuffer(*this, Size, AccessProt, HostAccessProt);

  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return Buf;
}

HostImage *Context::CreateHostImage(
    size_t Size,
    void *Storage,
    Image::TargetDevices &TargetDevs,
    Image::ChannelOrder ChOrder,
    Image::ChannelType ChDataType,
    size_t ElementSize,
    Image::ImgType ImgTy,
    size_t Width,
    size_t Height,
    size_t Depth,
    size_t ArraySize,
    size_t RowPitch,
    size_t SlicePitch,
    unsigned NumMipLevels,
    unsigned NumSamples,
    Buffer *Buf,
    MemoryObj::AccessProtection AccessProt,
    MemoryObj::HostAccessProtection HostAccessProt,
    cl_int *ErrCode) {
  HostImage *Img = new HostImage(*this,
                                 Size,
                                 Storage,
                                 TargetDevs,
                                 ChOrder, ChDataType,
                                 ElementSize,
                                 ImgTy,
                                 Width, Height, Depth,
                                 ArraySize,
                                 RowPitch, SlicePitch,
                                 NumMipLevels, NumSamples,
                                 Buf,
                                 MemoryObj::UseHostPtr, AccessProt, HostAccessProt);

  bool Rollback = false;
  
  for(Image::targetdev_iterator I = TargetDevs.begin(),
                                E = TargetDevs.end();
                                I != E && !Rollback;
                                ++I)
    if(!(*I)->CreateHostImage(*Img))
      Rollback = true;
      
  if(Rollback) {
    for(Image::targetdev_iterator I = TargetDevs.begin(),
                                  E = TargetDevs.end();
                                  I != E;
                                  ++I)
      (*I)->DestroyMemoryObj(*Img);

    delete Img;

    RETURN_WITH_ERROR(ErrCode,
                      CL_OUT_OF_RESOURCES,
                      "failed allocating resources for device image");

  }
  
  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return Img;																			
}

HostAccessibleImage *Context::CreateHostAccessibleImage(
    size_t Size,
    void *Src,
    Image::TargetDevices &TargetDevs,
    Image::ChannelOrder ChOrder,
    Image::ChannelType ChDataType,
    size_t ElementSize,
    Image::ImgType ImgTy,
    size_t Width,
    size_t Height,
    size_t Depth,
    size_t ArraySize,
    size_t RowPitch,
    size_t SlicePitch,
    unsigned NumMipLevels,
    unsigned NumSamples,
    Buffer *Buf,
    MemoryObj::AccessProtection AccessProt,
    MemoryObj::HostAccessProtection HostAccessProt,
    cl_int *ErrCode) {
  MemoryObj::HostPtrUsageMode HostPtrMode = MemoryObj::AllocHostPtr;
  if(Src)
    HostPtrMode = static_cast<MemoryObj::HostPtrUsageMode>(
                    static_cast<int>(HostPtrMode) | MemoryObj::CopyHostPtr
                  );
    
  HostAccessibleImage *Img = new HostAccessibleImage(
                                  *this,
                                  Size,
                                  Src,
                                  TargetDevs,
                                  ChOrder, ChDataType,
                                  ElementSize,
                                  ImgTy,
                                  Width, Height, Depth,
                                  ArraySize,
                                  RowPitch, SlicePitch,
                                  NumMipLevels, NumSamples,
                                  Buf,
                                  HostPtrMode, AccessProt, HostAccessProt
                                );
    
  bool Rollback = false;

  for(Image::targetdev_iterator I = TargetDevs.begin(),
                                E = TargetDevs.end();
                                I != E && !Rollback;
                                ++I)
    if(!(*I)->CreateHostAccessibleImage(*Img))
      Rollback = true;

  if(Rollback) {
    for(Image::targetdev_iterator I = TargetDevs.begin(),
                                  E = TargetDevs.end();
                                  I != E;
                                  ++I)
      (*I)->DestroyMemoryObj(*Img);

    delete Img;

    RETURN_WITH_ERROR(ErrCode,
                      CL_OUT_OF_RESOURCES,
                      "failed allocating resources for device image");

  }
  
  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return Img;
}

DeviceImage *Context::CreateDeviceImage(
    size_t Size,
    void *Src,
    Image::TargetDevices &TargetDevs,
    Image::ChannelOrder ChOrder,
    Image::ChannelType ChDataType,
    size_t ElementSize,
    Image::ImgType ImgTy,
    size_t Width,
    size_t Height,
    size_t Depth,
    size_t ArraySize,
    size_t RowPitch,
    size_t SlicePitch,
    unsigned NumMipLevels,
    unsigned NumSamples,
    Buffer *Buf,
    MemoryObj::AccessProtection AccessProt,
    MemoryObj::HostAccessProtection HostAccessProt,
    cl_int *ErrCode) {
  MemoryObj::HostPtrUsageMode HostPtrMode = MemoryObj::NoHostPtrUsage;
  if(Src)
    HostPtrMode = static_cast<MemoryObj::HostPtrUsageMode>(
                    static_cast<int>(HostPtrMode) | MemoryObj::CopyHostPtr
                  );
    
  DeviceImage *Img = new DeviceImage(*this, 
                                     Size, 
                                     Src,
                                     TargetDevs,
                                     ChOrder, ChDataType,
                                     ElementSize,
                                     ImgTy,
                                     Width, Height, Depth,
                                     ArraySize,
                                     RowPitch, SlicePitch,
                                     NumMipLevels, NumSamples,
                                     Buf,
                                     HostPtrMode, AccessProt, HostAccessProt);
                                     
  bool Rollback = false;

  for(Image::targetdev_iterator I = TargetDevs.begin(),
                                E = TargetDevs.end();
                                I != E && !Rollback;
                                ++I)
    if(!(*I)->CreateDeviceImage(*Img))
      Rollback = true;

  if(Rollback) {
    for(Image::targetdev_iterator I = TargetDevs.begin(),
                                E = TargetDevs.end();
                                I != E;
                                ++I)
      (*I)->DestroyMemoryObj(*Img);

    delete Img;

    RETURN_WITH_ERROR(ErrCode,
                      CL_OUT_OF_RESOURCES,
                      "failed allocating resources for device image");

  }
  
  if(ErrCode)
    *ErrCode = CL_SUCCESS;

  return Img;
}

void Context::DestroyMemoryObj(MemoryObj &MemObj) {
  for(device_iterator I = Devices.begin(), E = Devices.end(); I != E; ++I)
    if(!llvm::isa<VirtualBuffer>(MemObj))
      (*I)->DestroyMemoryObj(MemObj);
}

void Context::ReportDiagnostic(llvm::StringRef Msg) {
  if(Diag) {
    llvm::sys::ScopedLock Lock(DiagLock);
    auto Diags = Diag->getDiagnosticIDs();
    Diag->Report(Diags->getCustomDiagID(clang::DiagnosticIDs::Error, Msg));
  }

  UserDiag.Notify(Msg);
}

void Context::ReportDiagnostic(clang::TextDiagnosticBuffer &DiagInfo) {
  if(Diag) {
    llvm::sys::ScopedLock Lock(DiagLock);

    DiagInfo.FlushDiagnostics(*Diag);
  }
}
