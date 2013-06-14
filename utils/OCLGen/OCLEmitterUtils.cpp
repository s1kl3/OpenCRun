#include "OCLEmitterUtils.h"
#include "OCLType.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/ErrorHandling.h"

using namespace opencrun;

static const char *ExtensionName(unsigned E) {
  switch (E) {
  case Ext_cl_khr_fp16: return "cl_khr_fp16";
  case Ext_cl_khr_fp64: return "cl_khr_fp64";
  default: break;
  }
  return 0;
}

const char *opencrun::AddressSpaceName(OCLPointerType::AddressSpace AS) {
  switch (AS) {
  case OCLPointerType::AS_Private: return "__private";
  case OCLPointerType::AS_Global: return "__global";
  case OCLPointerType::AS_Local: return "__local";
  case OCLPointerType::AS_Constant: return "__constant";
  default: break;
  }

  llvm_unreachable("Invalid address space!");
  return 0;
}

void opencrun::EmitOCLTypeSignature(llvm::raw_ostream &OS, const OCLType &T,
                                    std::string Name) {
  if (llvm::isa<OCLGroupType>(&T)) 
    llvm_unreachable("Illegal basic type!");

  if (const OCLPointerType *P = llvm::dyn_cast<OCLPointerType>(&T)) {
    // Modifiers
    if (P->hasModifier(OCLPointerType::M_Const))
      OS << "const ";

    // Base type
    EmitOCLTypeSignature(OS, P->getBaseType());
    OS << " ";

    // Address Space
    // OS << AddressSpaceName(P->getAddressSpace()) << " ";
    OS << "__opencrun_as(" << P->getAddressSpace() << ") ";

    // Star
    OS << "*";
  } else {
    // Typen name
    OS << T;
  }

  if (Name.length())
    OS << " " << Name;
}

void opencrun::ComputeRequiredExt(const BuiltinSignature &Sign, 
                                  llvm::BitVector &Req) {
  Req.reset();
  for (unsigned i = 0, e = Sign.size(); i != e; ++i) {
    const OCLBasicType *B = Sign[i];

    const OCLScalarType *ST = 0;

    // FIXME: handle also pointers
    if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(B))
      ST = &V->getBaseType();
    else if (const OCLScalarType *S = llvm::dyn_cast<OCLScalarType>(B))
      ST = S;

    if (ST)
      Req |= ST->getRequiredTypeExt();
  }
}

void opencrun::EmitRequiredExtBegin(llvm::raw_ostream &OS, 
                                    const llvm::BitVector &Req) {
  llvm::SmallVector<const char *, 4> Names;
  for (unsigned i = Ext_InitValue; i != Ext_MaxValue; ++i)
    if (Req[i]) Names.push_back(ExtensionName(i));

  if (Names.size()) {
    OS << "#if";
    for (unsigned i = 0, e = Names.size(); i != e; ++i) {
      OS << " defined(__opencrun_support_" << Names[i]<< ")";
      if (i + 1 != e) OS << " &&";
    }
    OS << "\n";
    for (unsigned i = 0, e = Names.size(); i != e; ++i)
      OS << "#pragma OPENCL EXTENSION " << Names[i] << " : enable\n";
  }
}

void opencrun::EmitRequiredExtEnd(llvm::raw_ostream &OS, 
                                  const llvm::BitVector &Req) {
  if (!Req.any()) return;
  for (unsigned i = Ext_InitValue; i != Ext_MaxValue; ++i)
    if (Req[i])
      OS << "#pragma OPENCL EXTENSION " << ExtensionName(i) 
         << " : disable\n";
  OS << "#endif\n";
}

void opencrun::EmitBuiltinGroupBegin(llvm::raw_ostream &OS, 
                                     llvm::StringRef Group) {
  if (!Group.size()) return;
  OS << "#ifdef OPENCRUN_BUILTIN_" << Group << "\n\n";
}

void opencrun::EmitBuiltinGroupEnd(llvm::raw_ostream &OS, 
                                   llvm::StringRef Group) {
  if (!Group.size()) return;
  OS << "#endif // OPENCRUN_BUILTIN_" << Group << "\n";
}

bool opencrun::IsScalarAlternative(BuiltinSignature &Sign) {
  bool IsScalar = true;

  for (unsigned I = 0, E = Sign.size(); I != E && IsScalar; ++I) {
    if (llvm::isa<const OCLScalarType>(Sign[I]))
      continue;
    else if(llvm::isa<const OCLPointerType>(Sign[I])) {
      const OCLPointerType *P = llvm::cast<const OCLPointerType>(Sign[I]);

      if (!llvm::isa<const OCLScalarType>(P->getBaseType()))
        IsScalar = false;

    } else {
      IsScalar = false;
    }
  }

  return IsScalar;
}

