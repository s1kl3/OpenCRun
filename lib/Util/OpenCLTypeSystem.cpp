#include "opencrun/Util/OpenCLTypeSystem.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecordLayout.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

using namespace opencrun;
using namespace opencrun::opencl;

namespace {

enum MDNodeID {
  ID_Type,
  ID_QualType,
  ID_RecordTypeBody
};

}

#ifdef LLVM_GT_5
AddressSpace opencrun::opencl::convertAddressSpace(clang::LangAS AS) {
  switch (AS) {
  case clang::LangAS::Default: case clang::LangAS::opencl_private: return AS_Private;
#else
AddressSpace opencrun::opencl::convertAddressSpace(unsigned AS) {
  switch (AS) {
  case 0: return AS_Private;
#endif
  case clang::LangAS::opencl_global: return AS_Global;
  case clang::LangAS::opencl_constant: return AS_Constant;
  case clang::LangAS::opencl_local: return AS_Local;
  default: break;
  }
  llvm_unreachable("Illegal address space!");
}

ImageAccess
opencrun::opencl::convertImageAccess(const clang::OpenCLAccessAttr *CLA) {
  if (CLA->isReadOnly())
    return IA_ReadOnly;

  if (CLA->isWriteOnly())
    return IA_WriteOnly;

  llvm_unreachable("Illegal image access qualifier!");
}

Type::Type(llvm::MDNode *MD) : MD(MD) {
  assert(!MD || isRawType() ||
         (isQualType() && getUnqualifiedType().isRawType()));
}

Type::PrimitiveClass Type::getPrimitiveClass() const {
  if (isQualType())
    return getUnqualifiedType().getPrimitiveClass();

  assert(isPrimitive());

  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(2));

  assert(Cst->getZExtValue() < PrimitiveEnd);
  return PrimitiveClass(Cst->getZExtValue());
}

Type::ImageClass Type::getImageClass() const {
  if (isQualType())
    return getUnqualifiedType().getImageClass();

  assert(isImage());
  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(2));

  assert(Cst->getZExtValue() < ImageEnd);
  return ImageClass(Cst->getZExtValue());
}

Type Type::getElementType() const {
  if (isQualType())
    return getUnqualifiedType().getElementType();

  assert(isArray() || isVector() || isPointer());

  return llvm::cast<llvm::MDNode>(MD->getOperand(2));
}

unsigned Type::getNumElements() const {
  if (isQualType())
    return getUnqualifiedType().getNumElements();

  assert(isArray() || isVector());

  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(3));
  return Cst->getZExtValue();
}

bool Type::isRecordTypeAnonymous() const {
  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(3));
  return Cst->getZExtValue();
}

llvm::StringRef Type::getRecordTypeName() const {
  if (isQualType())
    return getUnqualifiedType().getRecordTypeName();

  assert(isStruct() || isUnion());
  assert(!isRecordTypeAnonymous());

  return llvm::cast<llvm::MDString>(MD->getOperand(2))->getString();
}

RecordTypeBody Type::getRecordTypeBody() const {
  if (isQualType())
    return getUnqualifiedType().getRecordTypeBody();

  assert(isStruct() || isUnion());

  return llvm::cast<llvm::MDNode>(MD->getOperand(4));
}

Qualifiers Type::getQualifiers() const {
  if (!isQualType()) return Qualifiers(0);

  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(1));
  return Qualifiers(Cst->getZExtValue());
}

Type::Kind Type::getKind() const {
  if (isQualType())
    return getUnqualifiedType().getKind();

  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(1));

  assert(Cst->getZExtValue() <= TK_Union);
  return Kind(Cst->getZExtValue());
}

bool Type::isQualType() const {
  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(0));

  return Cst->getZExtValue() == ID_QualType;
}

bool Type::isRawType() const {
  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(0));

  return Cst->getZExtValue() == ID_Type;
}

Type Type::getUnqualifiedType() const {
  if (isQualType())
    return llvm::cast<llvm::MDNode>(MD->getOperand(2));
  return MD;
}

RecordTypeBody::RecordTypeBody(llvm::MDNode *MD) : MD(MD) {
  assert(!MD || isRecordBody());
}

uint64_t RecordTypeBody::getSizeInBits() const {
  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(1));
  return Cst->getZExtValue();
}

uint64_t RecordTypeBody::getAlignment() const {
  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(2));
  return Cst->getZExtValue();
}

bool RecordTypeBody::isPacked() const {
  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(3));
  return Cst->getZExtValue();
}

unsigned RecordTypeBody::getNumFields() const {
  return (MD->getNumOperands() - 4) / 2;
}

Type RecordTypeBody::getField(unsigned I) const {
  assert(I < getNumFields());
  return llvm::cast<llvm::MDNode>(MD->getOperand(I * 2 + 4));
}

uint64_t RecordTypeBody::getFieldOffset(unsigned I) const {
  assert(I < getNumFields());
  auto *Cst =
    llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(I * 2 + 5));
  return Cst->getZExtValue();
}

bool RecordTypeBody::isRecordBody() const {
  auto *Cst = llvm::mdconst::extract<llvm::ConstantInt>(MD->getOperand(0));
  return Cst->getZExtValue() == ID_RecordTypeBody;
}

static llvm::Metadata *getIntegerMD(llvm::Type *Ty, uint64_t Value) {
  return llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(Ty, Value));
}

Type TypeGenerator::get(clang::ASTContext &ASTCtx, clang::QualType Ty,
                        const clang::OpenCLAccessAttr *CLA) {
  llvm::LLVMContext &Ctx = Mod.getContext();

  llvm::StringRef Name;
  Ty = Ty.getDesugaredType(ASTCtx);

  if (Ty->isBuiltinType()) {
    Type RetTy;
    switch (Ty->getAs<clang::BuiltinType>()->getKind()) {
    case clang::BuiltinType::Bool:
      RetTy = getPrimitiveType(Type::Bool);
      break;
    case clang::BuiltinType::Char_S:
    case clang::BuiltinType::Char_U:
    case clang::BuiltinType::SChar:
      RetTy = getPrimitiveType(Type::Char);
      break;
    case clang::BuiltinType::Short:
      RetTy = getPrimitiveType(Type::Short);
      break;
    case clang::BuiltinType::Int:
      RetTy = getPrimitiveType(Type::Int);
      break;
    case clang::BuiltinType::Long:
      RetTy = getPrimitiveType(Type::Long);
      break;
    case clang::BuiltinType::UChar:
      RetTy = getPrimitiveType(Type::UChar);
      break;
    case clang::BuiltinType::UShort:
      RetTy = getPrimitiveType(Type::UShort);
      break;
    case clang::BuiltinType::UInt:
      RetTy = getPrimitiveType(Type::UInt);
      break;
    case clang::BuiltinType::ULong:
      RetTy = getPrimitiveType(Type::ULong);
      break;
    case clang::BuiltinType::Half:
      RetTy = getPrimitiveType(Type::Half);
      break;
    case clang::BuiltinType::Float:
      RetTy = getPrimitiveType(Type::Float);
      break;
    case clang::BuiltinType::Double:
      RetTy = getPrimitiveType(Type::Double);
      break;
    case clang::BuiltinType::OCLEvent:
      RetTy = getPrimitiveType(Type::Event);
      break;

    case clang::BuiltinType::OCLImage1dRO:
    case clang::BuiltinType::OCLImage1dWO:
      RetTy = getImageType(Type::Image1d);
      break;
    case clang::BuiltinType::OCLImage1dArrayRO:
    case clang::BuiltinType::OCLImage1dArrayWO:
      RetTy = getImageType(Type::Image1dArray);
      break;
    case clang::BuiltinType::OCLImage1dBufferRO:
    case clang::BuiltinType::OCLImage1dBufferWO:
      RetTy = getImageType(Type::Image1dBuffer);
      break;
    case clang::BuiltinType::OCLImage2dRO:
    case clang::BuiltinType::OCLImage2dWO:
      RetTy = getImageType(Type::Image2d);
      break;
    case clang::BuiltinType::OCLImage2dArrayRO:
    case clang::BuiltinType::OCLImage2dArrayWO:
      RetTy = getImageType(Type::Image2dArray);
      break;
    case clang::BuiltinType::OCLImage3dRO:
    case clang::BuiltinType::OCLImage3dWO:
      RetTy = getImageType(Type::Image3d);
      break;
    case clang::BuiltinType::OCLSampler:
      RetTy = getImageType(Type::Sampler);
      break;
    default:
      llvm_unreachable("Unexpected clang type!");
    }
    return addQualifiers(Ty.getQualifiers(), CLA, RetTy);
  }

  if (Ty->isConstantArrayType()) {
    auto *CATy = llvm::cast<clang::ConstantArrayType>(Ty->getAsArrayTypeUnsafe());
    clang::QualType Elem = CATy->getElementType();
    unsigned NumElems = CATy->getSize().getZExtValue();
    return addQualifiers(Ty.getQualifiers(), 0,
                         getArrayType(get(ASTCtx, Elem), NumElems));
  }

  if (Ty->isPointerType()) {
    clang::QualType Pointee = Ty->getAs<clang::PointerType>()->getPointeeType();

    return addQualifiers(Ty.getQualifiers(), CLA,
                         getPointerType(get(ASTCtx, Pointee)));
  }

  if (Ty->isExtVectorType()) {
    const clang::ExtVectorType *VTy = Ty->getAs<clang::ExtVectorType>();
    clang::QualType Elem = VTy->getElementType();
    unsigned NumElems = VTy->getNumElements();
    return addQualifiers(Ty.getQualifiers(), 0,
                         getVectorType(get(ASTCtx, Elem), NumElems));
  }

  if (Ty->isRecordType()) {
    const clang::RecordDecl *RD = Ty->getAs<clang::RecordType>()->getDecl();
    assert(RD->isUnion() || RD->isStruct());

    RecordTypesContainer::const_iterator I = RTCache.find(RD);
    if (I != RTCache.end())
      return addQualifiers(Ty.getQualifiers(), CLA, I->second);

    bool IsUnion = RD->isUnion();
    bool IsAnonymous = RD->isAnonymousStructOrUnion();
    llvm::StringRef Name = RD->getName();

    Type R;

    if (!IsAnonymous) {
      // Named struct or union can have recursive dependencies, but have a
      // unique definition. Thus we can create the type immediatly and safely
      // replace the body reference later.
      R = getRecordType(Name, IsUnion, IsAnonymous);
      RTCache[RD] = R;
    }

    const clang::ASTRecordLayout &Layout = ASTCtx.getASTRecordLayout(RD);

    uint64_t Size = ASTCtx.toBits(Layout.getSize());
    uint64_t Alignment = ASTCtx.toBits(Layout.getAlignment());
    bool Packed = RD->hasAttr<clang::PackedAttr>();

    llvm::Type *I64Ty = llvm::Type::getInt64Ty(Ctx);
    llvm::Type *I1Ty = llvm::Type::getInt1Ty(Ctx);

    llvm::SmallVector<llvm::Metadata *, 32> BodyVals;
    BodyVals.push_back(getIntegerMD(I64Ty, ID_RecordTypeBody));
    BodyVals.push_back(getIntegerMD(I64Ty, Size));
    BodyVals.push_back(getIntegerMD(I64Ty, Alignment));
    BodyVals.push_back(getIntegerMD(I1Ty, Packed));

    uint64_t FieldIdx = 0;
    for (clang::RecordDecl::field_iterator I = RD->field_begin(),
         E = RD->field_end(); I != E; ++I, ++FieldIdx) {
      BodyVals.push_back(get(ASTCtx, I->getType()).getMDNode());
      uint64_t Offset = Layout.getFieldOffset(FieldIdx);
      BodyVals.push_back(getIntegerMD(I64Ty, Offset));
    }

    RecordTypeBody Body = llvm::MDNode::get(Ctx, BodyVals);

    if (IsAnonymous) {
      // Anonymous struct or union cannot have recursive dependencies. However
      // different anonymous struct or union can be equivalent, thus we have to
      // unify the type descriptor.
      R = getRecordTypeWithBody(Name, IsUnion, true, Body);
    } else {
      // Replace the body reference in the current MDNode.
      R.getMDNode()->replaceOperandWith(4, Body.getMDNode());
    }

    return addQualifiers(Ty.getQualifiers(), CLA, R);
  }

  llvm_unreachable("Unexpected clang type!");
}

Type TypeGenerator::getPrimitiveType(Type::PrimitiveClass C) {
  llvm::LLVMContext &Ctx = Mod.getContext();
  llvm::Type *I64Ty = llvm::Type::getInt64Ty(Ctx);

  llvm::Metadata *MDs[3] = {
    getIntegerMD(I64Ty, ID_Type),
    getIntegerMD(I64Ty, Type::TK_Primitive),
    getIntegerMD(I64Ty, C)
  };

  return llvm::MDNode::get(Ctx, MDs);
}

Type TypeGenerator::getImageType(Type::ImageClass C) {
  llvm::LLVMContext &Ctx = Mod.getContext();
  llvm::Type *I64Ty = llvm::Type::getInt64Ty(Ctx);

  llvm::Metadata *MDs[3] = {
    getIntegerMD(I64Ty, ID_Type),
    getIntegerMD(I64Ty, Type::TK_Image),
    getIntegerMD(I64Ty, C)
  };

  return llvm::MDNode::get(Ctx, MDs);
}

Type TypeGenerator::getVectorType(Type Elem, unsigned NumElems) {
  assert(Elem.isPrimitive() && !Elem.isQualType());

  llvm::LLVMContext &Ctx = Mod.getContext();
  llvm::Type *I64Ty = llvm::Type::getInt64Ty(Ctx);

  llvm::Metadata *MDs[4] = {
    getIntegerMD(I64Ty, ID_Type),
    getIntegerMD(I64Ty, Type::TK_Vector),
    Elem.getMDNode(),
    getIntegerMD(I64Ty, NumElems)
  };

  return llvm::MDNode::get(Ctx, MDs);
}

Type TypeGenerator::getRecordType(llvm::StringRef Name, bool IsUnion,
                                  bool IsAnonymous) {
  llvm::LLVMContext &Ctx = Mod.getContext();
  llvm::Type *I64Ty = llvm::Type::getInt64Ty(Ctx);
  llvm::Type *I1Ty = llvm::Type::getInt1Ty(Ctx);

  llvm::Metadata *MDs[5] = {
    getIntegerMD(I64Ty, ID_Type),
    getIntegerMD(I64Ty, IsUnion ? Type::TK_Union : Type::TK_Struct),
    llvm::MDString::get(Ctx, Name),
    getIntegerMD(I1Ty, IsAnonymous),
    getIntegerMD(I64Ty, -1) // Placeholder for body
  };

  Type Res = llvm::MDNode::get(Ctx, MDs);
  Res.getMDNode()->replaceOperandWith(4, Res.getMDNode());
  return Res;
}

Type TypeGenerator::getRecordTypeWithBody(llvm::StringRef Name, bool IsUnion,
                                          bool IsAnonymous, RecordTypeBody B) {
  llvm::LLVMContext &Ctx = Mod.getContext();
  llvm::Type *I64Ty = llvm::Type::getInt64Ty(Ctx);
  llvm::Type *I1Ty = llvm::Type::getInt1Ty(Ctx);

  llvm::Metadata *MDs[5] = {
    getIntegerMD(I64Ty, ID_Type),
    getIntegerMD(I64Ty, IsUnion ? Type::TK_Union : Type::TK_Struct),
    llvm::MDString::get(Ctx, Name),
    getIntegerMD(I1Ty, IsAnonymous),
    B.getMDNode()
  };

  return llvm::MDNode::get(Ctx, MDs);
}

Type TypeGenerator::getPointerType(Type Pointee) {
  llvm::LLVMContext &Ctx = Mod.getContext();
  llvm::Type *I64Ty = llvm::Type::getInt64Ty(Ctx);

  llvm::Metadata *MDs[3] = {
    getIntegerMD(I64Ty, ID_Type),
    getIntegerMD(I64Ty, Type::TK_Pointer),
    Pointee.getMDNode()
  };

  return llvm::MDNode::get(Ctx, MDs);
}

Type TypeGenerator::getArrayType(Type Elem, unsigned NumElems) {
  llvm::LLVMContext &Ctx = Mod.getContext();
  llvm::Type *I64Ty = llvm::Type::getInt64Ty(Ctx);

  llvm::Metadata *MDs[4] = {
    getIntegerMD(I64Ty, ID_Type),
    getIntegerMD(I64Ty, Type::TK_Array),
    Elem.getMDNode(),
    getIntegerMD(I64Ty, NumElems)
  };

  return llvm::MDNode::get(Ctx, MDs);
}

Type TypeGenerator::getQualType(uint32_t Mask, Type T) {
  assert(!T.isQualType());

  llvm::LLVMContext &Ctx = Mod.getContext();
  llvm::Type *I64Ty = llvm::Type::getInt64Ty(Ctx);
  llvm::Type *I32Ty = llvm::Type::getInt32Ty(Ctx);

  llvm::Metadata *MDs[3] = {
    getIntegerMD(I64Ty, ID_QualType),
    getIntegerMD(I32Ty, Mask),
    T.getMDNode()
  };

  return llvm::MDNode::get(Ctx, MDs);
}
 
Type TypeGenerator::addQualifiers(clang::Qualifiers Q,
                                  const clang::OpenCLAccessAttr *CLA,
                                  Type T) {
  Qualifiers Quals = T.getQualifiers();
  T = T.getUnqualifiedType();

  if (Q.hasConst()) Quals.addConst();
  if (Q.hasVolatile()) Quals.addVolatile();
  if (Q.hasRestrict()) Quals.addRestrict();
  Quals.addAddressSpace(convertAddressSpace(Q.getAddressSpace()));
  if (CLA) Quals.addImageAccess(convertImageAccess(CLA));

  return Quals.empty() ? T : getQualType(Quals.getQualifiersMask(), T);
}

bool TypeComparator::match(Type T1, Type T2, std::set<MatchedTypes> &Visited) {
  if (T1 == T2)
    return true;

  if (&T1.MD->getContext() == &T2.MD->getContext())
    return false;

  if (T1.getQualifiers() != T2.getQualifiers())
    return false;

  T1 = T1.getUnqualifiedType();
  T2 = T2.getUnqualifiedType();

  MatchedTypes MP(T1.getMDNode(), T2.getMDNode());

  if (Cache.count(MP))
    return true;

  if (T1.getKind() != T2.getKind())
    return false;

  switch (T1.getKind()) {
  default: break;
  case Type::TK_Primitive:
    return T1.getPrimitiveClass() == T2.getPrimitiveClass();
  case Type::TK_Image:
    return T1.getImageClass() == T2.getImageClass();
  case Type::TK_Vector:
  case Type::TK_Array:
    return T1.getNumElements() == T2.getNumElements() &&
           match(T1.getElementType(), T2.getElementType(), Visited);
  case Type::TK_Pointer:
    return match(T1.getElementType(), T2.getElementType(), Visited);
  }

  assert(T1.isStruct() || T1.isUnion());

  // Check for record type declaration already visited
  if (Visited.count(MP))
    return true;

  if (T1.isRecordTypeAnonymous() != T2.isRecordTypeAnonymous())
    return false;

  if (!T1.isRecordTypeAnonymous() &&
      T1.getRecordTypeName() != T2.getRecordTypeName())
    return false;

  RecordTypeBody B1 = T1.getRecordTypeBody();
  RecordTypeBody B2 = T2.getRecordTypeBody();

  if (B1.isPacked() != B2.isPacked() ||
      B1.getAlignment() != B2.getAlignment() ||
      B1.getNumFields() != B2.getNumFields())
    return false;

  Visited.insert(MP);
  bool AllFieldsMatch = true;
  for (unsigned I = 0, E = B1.getNumFields(); I != E; ++I) {
    if (B1.getFieldOffset(I) != B2.getFieldOffset(I) ||
        !match(B1.getField(I), B2.getField(I), Visited)) {
      AllFieldsMatch = false;
      break;
    }
  }
  Visited.erase(MP);

  if (AllFieldsMatch)
    Cache.insert(MP);

  return AllFieldsMatch;
}
