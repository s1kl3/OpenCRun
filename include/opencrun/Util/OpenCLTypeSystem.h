#ifndef OPENCRUN_UTIL_OPENCLTYPESYSTEM_H
#define OPENCRUN_UTIL_OPENCLTYPESYSTEM_H

#include "clang/AST/Type.h"
#include "llvm/ADT/StringRef.h"

#include <map>
#include <set>

namespace clang {
class OpenCLAccessAttr;
class RecordTypeDecl;
}

namespace llvm {
class MDNode;
class Module;
}

namespace opencrun {
namespace opencl {

enum AddressSpace {
  AS_Private,
  AS_Global,
  AS_Constant,
  AS_Local,

  AS_Invalid
};

#ifdef LLVM_GT_5
AddressSpace convertAddressSpace(clang::LangAS AS);
#else
AddressSpace convertAddressSpace(unsigned AS);
#endif

enum ImageAccess {
  IA_ReadOnly = 1,
  IA_WriteOnly,

  IA_Invalid = 0
};

ImageAccess convertImageAccess(const clang::OpenCLAccessAttr *CLA);

class Qualifiers {
public:
  explicit Qualifiers(uint32_t mask) : Mask(mask) {}
  Qualifiers(const Qualifiers &Q) : Mask(Q.Mask) {}

private:
  enum {
    Const = 1 << 0,
    Volatile = 1 << 1,
    Restrict = 1 << 2,
    ImageAccessShAmt = 3,
    ImageAccessMask = 3 << ImageAccessShAmt,
    AddressSpaceShAmt = 5,
    AddressSpaceMask = 3 << AddressSpaceShAmt
  };

public:
  bool isConst() const { return Mask & Const; }
  bool isVolatile() const { return Mask & Volatile; }
  bool isRestrict() const { return Mask & Restrict; }
  AddressSpace getAddressSpace() const {
    return AddressSpace((Mask & AddressSpaceMask) >> AddressSpaceShAmt);
  }
  ImageAccess getImageAccess() const {
    return ImageAccess((Mask & ImageAccessMask) >> ImageAccessShAmt);
  }

  void addConst() { Mask |= Const; }
  void addVolatile() { Mask |= Volatile; }
  void addRestrict() { Mask |= Restrict; }
  void addAddressSpace(AddressSpace AS) {
    Mask = (Mask & ~AddressSpaceMask) | AS << AddressSpaceShAmt;
  }
  void addImageAccess(ImageAccess IA) {
    Mask = (Mask & ~ImageAccessMask) | IA << ImageAccessShAmt;
  }

  void removeConst() { Mask &= ~Const; }
  void removeVolatile() { Mask &= ~Volatile; }
  void removeRestrict() { Mask &= ~Restrict; }
  void removeAddressSpace() { addAddressSpace(AS_Private); }
  void removeImageAccess() { addImageAccess(IA_Invalid); }

  uint32_t getQualifiersMask() const { return Mask; }

  bool operator==(const Qualifiers &Q) const { return Mask == Q.Mask; }
  bool operator!=(const Qualifiers &Q) const { return Mask != Q.Mask; }

  bool empty() const { return Mask == 0; }

private:
  uint32_t Mask;
};

class RecordTypeBody;

class Type {
public:
  enum Kind {
    TK_Primitive,
    TK_Vector,
    TK_Image,
    TK_Array,
    TK_Pointer,
    TK_Struct,
    TK_Union
  };

  enum PrimitiveClass {
    Bool,
    Char, UChar,
    Short, UShort,
    Int, UInt,
    Long, ULong,
    Half, Float, Double,
    Event,

    PrimitiveEnd,
    PrimitiveBegin = Bool,
    VectorBaseBegin = Char,
    VectorBaseEnd = Event
  };

  enum ImageClass {
    Image1d, Image1dArray, Image1dBuffer,
    Image2d, Image2dArray,
    Image3d,
    Sampler,

    ImageEnd,
    ImageBegin = Image1d
  };

public:
  Type(llvm::MDNode *MD = 0);
  Type(const Type &T) : MD(T.MD) {}
  Type &operator=(const Type &T) {
    MD = T.MD; return *this;
  }

  bool isPrimitive() const { return getKind() == TK_Primitive; }
  bool isVector() const { return getKind() == TK_Vector; }
  bool isImage() const { return getKind() == TK_Image; }
  bool isArray() const { return getKind() == TK_Array; }
  bool isPointer() const { return getKind() == TK_Pointer; }
  bool isStruct() const { return getKind() == TK_Struct; }
  bool isUnion() const { return getKind() == TK_Union; }

  PrimitiveClass getPrimitiveClass() const;
  ImageClass getImageClass() const;
  Type getElementType() const;
  unsigned getNumElements() const;

  bool isRecordTypeAnonymous() const;
  llvm::StringRef getRecordTypeName() const;
  RecordTypeBody getRecordTypeBody() const;

  Qualifiers getQualifiers() const;
  Type getUnqualifiedType() const;

  llvm::MDNode *getMDNode() const { return MD; }

  bool operator==(const Type &T) const { return MD == T.MD; }
  bool operator!=(const Type &T) const { return MD != T.MD; }

private:
  bool isQualType() const;
  bool isRawType() const;
  Kind getKind() const;

private:
  llvm::MDNode *MD;

  friend class TypeGenerator;
  friend class TypeComparator;
};

class RecordTypeBody {
public:
  RecordTypeBody(llvm::MDNode *BodyMD = 0);
  RecordTypeBody(const RecordTypeBody &B) : MD(B.MD) {}
  RecordTypeBody &operator=(const RecordTypeBody &B) {
    MD = B.MD; return *this;
  }

  uint64_t getSizeInBits() const;
  uint64_t getAlignment() const;
  bool isPacked() const;
  unsigned getNumFields() const;
  Type getField(unsigned I) const;
  uint64_t getFieldOffset(unsigned I) const;

  llvm::MDNode *getMDNode() const { return MD; }

private:
  bool isRecordBody() const;

private:
  llvm::MDNode *MD;
};

class TypeGenerator {
public:
  TypeGenerator(llvm::Module &M) : Mod(M) {}

  Type get(clang::ASTContext &ASTCtx, clang::QualType Ty,
           const clang::OpenCLAccessAttr *CLA = 0);

private:
  typedef std::map<const clang::RecordDecl*, Type> RecordTypesContainer;

private:
  Type getPrimitiveType(Type::PrimitiveClass C);
  Type getImageType(Type::ImageClass C);
  Type getVectorType(Type Elem, unsigned Size);
  Type getRecordType(llvm::StringRef Name, bool IsUnion, bool IsAnonymous);
  Type getRecordTypeWithBody(llvm::StringRef Name, bool IsUnion,
                             bool IsAnonymous, RecordTypeBody Body);
  Type getPointerType(Type Pointee);
  Type getArrayType(Type Elem, unsigned Size);
  Type getQualType(uint32_t Quals, Type Ty);

  Type addQualifiers(clang::Qualifiers,
                     const clang::OpenCLAccessAttr *CLA,
                     Type Ty);

private:
  RecordTypesContainer RTCache;
  llvm::Module &Mod;
};

class TypeComparator {
public:
  bool equals(Type T1, Type T2) {
    std::set<MatchedTypes> Visited;
    return match(T1, T2, Visited);
  }

  void reset() { Cache.clear(); }

private:
  typedef std::pair<llvm::MDNode*, llvm::MDNode*> MatchedTypes;

private:
  bool match(Type T1, Type T2, std::set<MatchedTypes> &Visited);

private:
  std::set<MatchedTypes> Cache;
};

}
}

#endif
