
#ifndef OPENCRUN_CORE_MEMORYOBJ_H
#define OPENCRUN_CORE_MEMORYOBJ_H

#include "CL/opencl.h"

#include "opencrun/Util/MTRefCounted.h"

#include "llvm/Support/Mutex.h"

#include <map>

struct _cl_mem { };

namespace opencrun {

class Context;

class MemoryObj : public _cl_mem, public MTRefCountedBaseVPTR<MemoryObj> {
public:
  enum Type {
    HostBuffer,
    HostAccessibleBuffer,
    DeviceBuffer,
    VirtualBuffer,
    LastBuffer
  };

	enum HostPtrUsageMode {
		UseHostPtr			= CL_MEM_USE_HOST_PTR,
		AllocHostPtr		= CL_MEM_ALLOC_HOST_PTR,
		CopyHostPtr			= CL_MEM_COPY_HOST_PTR,
		NoHostPtrUsage	= 0
	};
	
  enum AccessProtection {
    ReadWrite         = CL_MEM_READ_WRITE,
    WriteOnly         = CL_MEM_WRITE_ONLY,
    ReadOnly          = CL_MEM_READ_ONLY,
    InvalidProtection = 0
  };
	
	enum HostAccessProtection {
		HostWriteOnly			= CL_MEM_HOST_WRITE_ONLY,
		HostReadOnly			= CL_MEM_HOST_READ_ONLY,
		HostNoAccess			= CL_MEM_HOST_NO_ACCESS,
		HostNoProtection	= 0
	};
	
public:
  static bool classof(const _cl_mem *MemObj) { return true; }

public:
	typedef struct {
		size_t Offset;
		size_t Size;
		unsigned long MapFlags;
	} MappingInfo;
	
	typedef std::multimap<void *, MappingInfo> MappingsContainer;
	
public:
	typedef MappingsContainer::iterator maps_iterator;
	typedef MappingsContainer::const_iterator const_maps_iterator;
	
protected:
  MemoryObj(Type MemTy,
            Context &Ctx,
            size_t Size,
						HostPtrUsageMode HostPtrMode,
            AccessProtection AccessProt,
						HostAccessProtection HostAccessProt) : MemTy(MemTy),
																									 Ctx(&Ctx),
																									 Size(Size),
																									 HostPtrMode(HostPtrMode),
																									 AccessProt(AccessProt),
																									 HostAccessProt(HostAccessProt)	{ }

public:
  virtual ~MemoryObj();

public:
  size_t GetSize() const { return Size; }
  Type GetType() const { return MemTy; }
  Context &GetContext() const { return *Ctx; }
	
	HostPtrUsageMode GetHostPtrUsageMode() const { return HostPtrMode; }
	AccessProtection GetAccessProtection() const { return AccessProt; }
	HostAccessProtection GetHostAccessProtection() const { return HostAccessProt; }
	
	unsigned long GetMemFlags() const { 
		return static_cast<unsigned long>(HostPtrMode | 
																			AccessProt | 
																			HostAccessProt); 
	}
	
public:
	bool AddNewMapping(void *MapBuf, 
										 size_t Offset, 
										 size_t Size, 
										 unsigned long MapFlags);
	
	bool RemoveMapping(void *MapBuf);
	
	bool IsValidMappingPtr(void *MapBuf);
	
	MappingInfo *GetMappingInfo(void *MapBuf);
  
  size_t GetMappedCount() const { return Maps.size(); }
	
private:
  Type MemTy;

  llvm::IntrusiveRefCntPtr<Context> Ctx;
  size_t Size;
	
	HostPtrUsageMode HostPtrMode;
  AccessProtection AccessProt;
	HostAccessProtection HostAccessProt;
	
	llvm::sys::Mutex ThisLock;
	MappingsContainer Maps;
};

class Buffer : public MemoryObj {
public:
  static bool classof(const MemoryObj *MemObj) {
    return MemObj->GetType() < MemoryObj::LastBuffer;
  }

protected:
  Buffer(Type MemTy,
         Context &Ctx,
         size_t Size,
				 MemoryObj::HostPtrUsageMode HostPtrMode,
         MemoryObj::AccessProtection AccessProt,
				 MemoryObj::HostAccessProtection HostAccessProt) :
  MemoryObj(MemTy, Ctx, Size, HostPtrMode, AccessProt, HostAccessProt) { }
};

class HostBuffer : public Buffer {
public:
	static bool classof(const MemoryObj *MemObj) {
		return MemObj->GetType() == MemoryObj::HostBuffer;
	}
	
private:
  HostBuffer(Context &Ctx,
             size_t Size,
             void *Storage,
						 MemoryObj::HostPtrUsageMode HostPtrMode,
             MemoryObj::AccessProtection AccessProt,
						 MemoryObj::HostAccessProtection HostAccessProt)
    : Buffer(MemoryObj::HostBuffer, Ctx, Size, HostPtrMode, AccessProt, HostAccessProt) { }

  HostBuffer(const HostBuffer &That); // Do not implement.
  void operator=(const HostBuffer &That); // Do not implement.

public:
	void *GetStorageData() const { return Storage; }

private:
	void *Storage;
	
  friend class Context;
};

class HostAccessibleBuffer : public Buffer {
public:
	static bool classof(const MemoryObj *MemObj) {
		return MemObj->GetType() == MemoryObj::HostAccessibleBuffer;
	}
	
private:
  HostAccessibleBuffer(Context &Ctx,
                       size_t Size,
                       void *Src,
											 MemoryObj::HostPtrUsageMode HostPtrMode,
                       MemoryObj::AccessProtection AccessProt,
											 MemoryObj::HostAccessProtection HostAccessProt)
    : Buffer(MemoryObj::HostAccessibleBuffer, Ctx, Size, HostPtrMode, AccessProt, HostAccessProt),
			Src(Src) { }

  HostAccessibleBuffer(const HostAccessibleBuffer &That); // Do not implement.
  void operator=(const HostAccessibleBuffer &That); // Do not implement.

public:
	const void *GetInitializationData() const { return Src; }
	bool HasInitializationData() const { return Src; }
	
private:
	void *Src;
	
  friend class Context;
};

class DeviceBuffer : public Buffer {
public:
	static bool classof(const MemoryObj *MemObj) {
		return MemObj->GetType() == MemoryObj::DeviceBuffer;
	}
	
private:
  DeviceBuffer(Context &Ctx,
               size_t Size,
               void *Src,
							 MemoryObj::HostPtrUsageMode HostPtrMode,
               MemoryObj::AccessProtection AccessProt,
							 MemoryObj::HostAccessProtection HostAccessProt)
    : Buffer(MemoryObj::DeviceBuffer, Ctx, Size, HostPtrMode, AccessProt, HostAccessProt),
      Src(Src) { }

  DeviceBuffer(const DeviceBuffer &That); // Do not implement.
  void operator=(const DeviceBuffer &That); // Do not implement.

public:
  const void *GetInitializationData() const { return Src; }
  bool HasInitializationData() const { return Src; }

private:
  void *Src;

  friend class Context;
};

class VirtualBuffer : public Buffer {
public:
	static bool classof(const MemoryObj *MemObj) {
		return MemObj->GetType() == MemoryObj::VirtualBuffer;
	}
	
private:
  VirtualBuffer(Context &Ctx,
                size_t Size,
                MemoryObj::AccessProtection AccessProt,
								MemoryObj::HostAccessProtection HostAccessProt)
    : Buffer(MemoryObj::VirtualBuffer, Ctx, Size, MemoryObj::NoHostPtrUsage, AccessProt, HostAccessProt) { }

  VirtualBuffer(const VirtualBuffer &That); // Do not implement.
  void operator=(const VirtualBuffer &That); // Do not implement.

private:
  friend class Context;
};

class BufferBuilder {
public:
  BufferBuilder(Context &Ctx, size_t Size);

public:
  BufferBuilder &SetUseHostMemory(bool Enabled, void *Storage);
  BufferBuilder &SetAllocHostMemory(bool Enabled);
  BufferBuilder &SetCopyHostMemory(bool Enabled, void *Src);
  BufferBuilder &SetReadWrite(bool Enabled);
  BufferBuilder &SetWriteOnly(bool Enabled);
  BufferBuilder &SetReadOnly(bool Enabled);
	BufferBuilder &SetHostWriteOnly(bool Enabled);
	BufferBuilder &SetHostReadOnly(bool Enabled);
	BufferBuilder &SetHostNoAccess(bool Enabled);
	
  Buffer *Create(cl_int *ErrCode = NULL);

private:
  BufferBuilder &NotifyError(cl_int ErrCode, const char *Msg = "");

private:
  Context &Ctx;
  size_t Size;

  void *HostPtr;

	MemoryObj::HostPtrUsageMode HostPtrMode;
  MemoryObj::AccessProtection AccessProt;
	MemoryObj::HostAccessProtection HostAccessProt;

  cl_int ErrCode;
};

} // End namespace opencrun.

#endif // OPENCRUN_CORE_MEMORYOBJ_H
