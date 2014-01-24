#include "OCLBuiltin.h"
#include "OCLEmitter.h"
#include "OCLEmitterUtils.h"
#include "OCLType.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace opencrun;

namespace {

OCLTypesContainer OCLTypes;
OCLBuiltinsContainer OCLBuiltins;
OCLBuiltinImplsContainer OCLBuiltinImpls;

}

typedef std::vector<std::string> IndexesContainer;

static void EmitOCLTypeValue(llvm::raw_ostream &OS, const OCLBasicType &T,
                             const OCLTypeValueDecl &D) {

  bool IsMin = llvm::isa<OCLMinValueDecl>(&D);

  if (const OCLIntegerType *I = llvm::dyn_cast<OCLIntegerType>(&T)) {
    bool IsUnsigned = I->isUnsigned();
    unsigned BitWidth = I->getBitWidth();

    llvm::APInt Value;
    if (IsMin && IsUnsigned)
      Value = llvm::APInt::getNullValue(BitWidth);
    else if (IsUnsigned)
      Value = llvm::APInt::getMaxValue(BitWidth);
    else if (IsMin)
      Value = llvm::APInt::getSignedMinValue(BitWidth);
    else
      Value = llvm::APInt::getSignedMaxValue(BitWidth);
    
    llvm::SmallString<20> Buffer; 
    Value.toString(Buffer, 16, !IsUnsigned, true);

    OS << Buffer;
  }
}

static void EmitOCLDecl(llvm::raw_ostream &OS, const OCLDecl *D,
                        const BuiltinSign &Sign, bool End) {
  if (const OCLTypedefDecl *T = llvm::dyn_cast<OCLTypedefDecl>(D)) {
    if (End) return;

    const OCLBasicType *Ty = T->getParam().get(Sign);
    OS.indent(2) << "typedef ";

    if (llvm::isa<OCLTypedefUnsignedDecl>(T)) {
      if (const OCLIntegerType *I = llvm::dyn_cast<OCLIntegerType>(Ty)) {
        if (I->isSigned()) OS << "unsigned ";
      } else
        llvm::PrintFatalError("Illegal source type for OCLTypedef: " + 
                              Ty->getName());
    }

    EmitOCLTypeSignature(OS, *Ty);
    OS << " " << T->getName() << ";\n";

  } else if (const OCLTypeValueDecl *T = llvm::dyn_cast<OCLTypeValueDecl>(D)) {
    if (End) return;

    const OCLBasicType *Ty = T->getParam().get(Sign);
    OS.indent(2) << "const ";
    EmitOCLTypeSignature(OS, *Ty, T->getID());
    OS << " = ";
    EmitOCLTypeValue(OS, *Ty, *T);
    OS << ";\n";
  } else if (const OCLBuiltinNameDecl *T = 
               llvm::dyn_cast<OCLBuiltinNameDecl>(D)) {
    OCLBuiltinDecorator BD(T->getBuiltin());
    
    OS.indent(2);
    if (End) {
      OS << "#undef " << T->getName() << "\n";
    } else {
      OS << "#define " << T->getName() << " " 
         << BD.getInternalName(&Sign) << "\n";
    }
  } else if (const OCLLibMDecl *T = llvm::dyn_cast<OCLLibMDecl>(D)) {
    OS.indent(2);

    if (End) {
      OS << "#undef __libm" << "\n"; 
      OS << "#undef __libm_const" << "\n"; 
      OS << "#undef __libm_CONST_" << "\n"; 
      OS << "#undef __libm_CONST" << "\n"; 
    } else {
      const OCLRealType *Ty = llvm::cast<OCLRealType>(T->getParam().get(Sign));
      bool IsFloat = Ty->getBitWidth() == 32;
      OS << "#define __libm(x) __builtin_ ## x ";
      if (IsFloat) OS << "## f";
      OS << "\n";
      OS << "#define __libm_const(x) x";
      if (IsFloat) OS << "## f";
      OS << "\n";
      OS << "#define __libm_CONST_(x) x";
      if (IsFloat) OS << "## _F";
      OS << "\n";
      OS << "#define __libm_CONST(x) x";
      if (IsFloat) OS << "## F";
      OS << "\n";
    }
  } else
    llvm::PrintFatalError("Illegal OCLDecl!");
}

static void EmitOCLDecls(llvm::raw_ostream &OS, const BuiltinSign &Sign,
                         const OCLStrategy &S, bool End) {
  for (OCLStrategy::decl_iterator DI = S.begin(), DE = S.end(); 
       DI != DE; ++DI) {
    EmitOCLDecl(OS, *DI, Sign, End);
  }
}

static bool IsScalarSignature(const BuiltinSign &Sign) {
  for (unsigned I = 0, E = Sign.size(); I != E; ++I) {
    const OCLType *T = Sign[I];
    if (llvm::isa<const OCLPointerType>(T)) {
      const OCLPointerType *P = 0;
      do {
        P = llvm::cast<OCLPointerType>(T);
        T = &P->getBaseType();
      } while (llvm::isa<OCLPointerType>(T));
    }
    if (llvm::isa<const OCLVectorType>(T)) return false;
  }

  return true;
}

static void EmitSubvector(llvm::raw_ostream &OS, 
                          std::pair<unsigned, unsigned> Range) {
  if (Range.first < Range.second) {
    OS << ".s";
    for (unsigned ri = Range.first; ri != Range.second; ++ri) OS.write_hex(ri);
  }
}

static void EmitImplementationBody(llvm::raw_ostream &OS,
                                   const OCLBuiltin &B,
                                   const BuiltinSign &Sign,
                                   const OCLRecursiveSplit &S) {
  if (IsScalarSignature(Sign)) {
    EmitOCLDecls(OS, Sign, S, false);
    OS << S.getScalarImpl() << "\n";
    EmitOCLDecls(OS, Sign, S, true);
    return;
  }

  BuiltinSign LeftSign, RightSign;
  std::vector<std::pair<unsigned, unsigned> > LeftRanges, RightRanges;
  for (unsigned i = 0, e = Sign.size(); i != e; ++i) {
    const OCLBasicType *B = Sign[i];
    if (llvm::isa<OCLScalarType>(B)) {
      LeftSign.push_back(B);
      RightSign.push_back(B);
      LeftRanges.push_back(std::make_pair(0,0));
      RightRanges.push_back(std::make_pair(0,0));
    } else if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(B)) {
      unsigned N = V->getWidth();
      unsigned N1 = N/2;
      unsigned N2 = N - N/2;
      const OCLBasicType *LTy, *RTy;
      if (N1  == 1) LTy = &V->getBaseType();
      else LTy = OCLTypesTable::getVectorType(V->getBaseType(), N1);
      if (N2  == 1) RTy = &V->getBaseType();
      else RTy = OCLTypesTable::getVectorType(V->getBaseType(), N2);
      LeftSign.push_back(LTy);
      RightSign.push_back(RTy);
      LeftRanges.push_back(std::make_pair(0,N1));
      RightRanges.push_back(std::make_pair(N1,N));
    } else
      llvm::PrintFatalError("Type not supported by RecursiveSplit strategy!");
  }

  // TODO: check if splitted signatures exists.

  OCLBuiltinDecorator BD(B);

  // Left branch
  OS.indent(2) << *LeftSign[0] << " Tmp1 = (" << *LeftSign[0] << ") "
               << BD.getInternalName(&LeftSign) << "(\n";
  for (unsigned i = 1, e = LeftSign.size(); i != e; ++i) {
    OS.indent(6) << "param" << i;
    EmitSubvector(OS, LeftRanges[i]);
    if (i + 1 != e)
      OS << ", \n";
    else {
      OS << "\n";
      OS.indent(4) << ");\n\n";
    }
  }

  // Right branch
  OS.indent(2) << *RightSign[0] << " Tmp2 = (" << *RightSign[0] << ") "
               << BD.getInternalName(&RightSign) << "(\n";
  for (unsigned i = 1, e = RightSign.size(); i != e; ++i) {
    OS.indent(6) << "param" << i;
    EmitSubvector(OS, RightRanges[i]);
    if (i + 1 != e)
      OS << ", \n";
    else {
      OS << "\n";
      OS.indent(4) << ");\n\n";
    }
  }

  const OCLReduction *R = S.getReduction();

  if (!R) {
    const OCLVectorType *RetTy = llvm::dyn_cast<OCLVectorType>(Sign[0]);
    if (!RetTy)
      llvm::PrintFatalError("RecursiveSplit: '" + B.getName() + 
                            "' has not a vector return type");

    OS.indent(2) << "return (" << *Sign[0] << ")(Tmp1, Tmp2);\n";
    return;
  }
  if (llvm::isa<OCLInfixBinAssocReduction>(R)) {
    const OCLInfixBinAssocReduction *BR =
      llvm::cast<OCLInfixBinAssocReduction>(R);
    OS.indent(2) << "return Tmp1 " << BR->getOperator() << " Tmp2;\n";
  } else
    llvm::PrintFatalError("Reduction not supported.");
}

enum DirectSplitKind {
  DSK_None,
  DSK_Vector,
  DSK_PtrVector
};

static void EmitImplementationBody(llvm::raw_ostream &OS,
                                   const OCLBuiltin &B,
                                   const BuiltinSign &Sign,
                                   const OCLDirectSplit &S) {
  if (IsScalarSignature(Sign)) {
    EmitOCLDecls(OS, Sign, S, false);
    OS << S.getScalarImpl() << "\n";
    EmitOCLDecls(OS, Sign, S, true);
    return;
  }

  const OCLVectorType *RetTy = llvm::dyn_cast<OCLVectorType>(Sign[0]);
  if (!RetTy)
    llvm::PrintFatalError("DirectSplit: '" + B.getName() + 
                          "' has not a vector return type");

  unsigned N = RetTy->getWidth();

  // Compute signature of the scalar builtin
  BuiltinSign ScalarSign;
  ScalarSign.reserve(Sign.size());
  std::vector<unsigned> SKs(Sign.size(), DSK_None);
  for (unsigned i = 0, e = Sign.size(); i != e; ++i) {
    const OCLBasicType *Ty = Sign[i];
    if (llvm::isa<OCLScalarType>(Ty)) {
      ScalarSign.push_back(Ty);
    } else if (llvm::isa<OCLVectorType>(Ty)) {
      const OCLVectorType *V = llvm::cast<OCLVectorType>(Ty);
      assert(V->getWidth() == N);
      ScalarSign.push_back(&V->getBaseType());
      SKs[i] = DSK_Vector;
    } else {
      assert(llvm::isa<OCLPointerType>(Ty));
      const OCLPointerType *PtrTy = llvm::cast<OCLPointerType>(Ty);
      const OCLType *PointeeTy = &PtrTy->getBaseType();
      
      if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(PointeeTy)) {
        OCLPtrStructure PtrS;
        PtrS.push_back(OCLPtrDesc(PtrTy->getAddressSpace(),
                                  PtrTy->getModifierFlags()));
        const OCLPointerType *CastTy =
          &OCLTypesTable::getPointerType(V->getBaseType(), PtrS);

        ScalarSign.push_back(CastTy);
        SKs[i] = DSK_PtrVector;
      } else
        llvm::PrintFatalError("Type not supported by DirectSplit strategy!");
    }
  }

  OCLBuiltinDecorator BD(B);

  OS.indent(2) << "return (" << *RetTy << ")(\n";

  for (unsigned i = 0; i != N; ++i) {
    OS.indent(6) << BD.getInternalName(&ScalarSign) << "(\n";
    for (unsigned PI = 1, PE = ScalarSign.size(); PI != PE; ++PI) {
      OS.indent(8);
      switch (SKs[PI]) {
      case DSK_None:
        OS << "param" << PI;
        break;
      case DSK_Vector:
        OS << "param" << PI << ".s";
        OS.write_hex(i);
        break;
      case DSK_PtrVector:
        OS << "(";
        EmitOCLTypeSignature(OS, *ScalarSign[PI]);
        OS << ") param" << PI;
        if (i > 0) OS << " + " << i;
        break;
      default:
        llvm_unreachable(0);
      }
      if (PI + 1 != PE) OS << ",";
      OS << "\n";
    }
    OS.indent(6) << ")";

    if (i + 1 != N) OS << ",";
    OS << "\n";
  }

  OS.indent(4) << ");\n";
}

static void EmitImplementationBody(llvm::raw_ostream &OS,
                                   const OCLBuiltin &B, 
                                   const BuiltinSign &Sign,
                                   const OCLTemplateStrategy &S) {
  EmitOCLDecls(OS, Sign, S, false);
  OS << S.getTemplateImpl() << "\n";
  EmitOCLDecls(OS, Sign, S, true);
}

static void EmitImplementationBody(llvm::raw_ostream &OS,
                                   const OCLBuiltin &B,
                                   const BuiltinSign &Sign,
                                   const OCLStrategy &S) {
  if (const OCLRecursiveSplit *RS = llvm::dyn_cast<OCLRecursiveSplit>(&S))
    EmitImplementationBody(OS, B, Sign, *RS);
  else if (const OCLDirectSplit *DS = llvm::dyn_cast<OCLDirectSplit>(&S))
    EmitImplementationBody(OS, B, Sign, *DS);
  else if (const OCLTemplateStrategy *TS = 
             llvm::dyn_cast<OCLTemplateStrategy>(&S))
    EmitImplementationBody(OS, B, Sign, *TS);
  else
    llvm::PrintFatalError("Illegal strategy!");
}

typedef std::map<BuiltinSign, const OCLBuiltinImpl *, 
                 BuiltinSignCompare> SignatureImplMap;
typedef std::map<const OCLBuiltin *, SignatureImplMap> BuiltinImplMap;

typedef std::map<const OCLBuiltin *, BuiltinSignList> BuiltinSignMap;

typedef std::set<const OCLRequirement *> RequirementSet;

static void EmitOCLRequirement(llvm::raw_ostream &OS, const OCLRequirement *R) {
  if (const OCLIncludeRequirement *I = 
        llvm::dyn_cast<OCLIncludeRequirement>(R))
    OS << "#include \"" << I->getFileName() << "\"\n";
  else if (const OCLCodeBlockRequirement *C = 
            llvm::dyn_cast<OCLCodeBlockRequirement>(R))
    OS << C->getCodeBlock();
  else
    llvm_unreachable(0);
 
  OS << "\n"; 
}

static void EmitRequirements(llvm::raw_ostream &OS, const OCLBuiltinImpl &BI, 
                             RequirementSet &EmittedReqs) {
  for (OCLBuiltinImpl::req_iterator
       I = BI.req_begin(), E = BI.req_end(); I != E; ++I) {
    const OCLRequirement *R = *I;
    if (!EmittedReqs.count(R)) {
      EmittedReqs.insert(R);
      EmitOCLRequirement(OS, R);
    }
  }
}

static void EmitOCLGenericBuiltinImpls(llvm::raw_ostream &OS,
                                       const OCLGenericBuiltin &B,
                                       const SignatureImplMap &M,
                                       RequirementSet &EmittedReqs) {
  OCLBuiltinDecorator BD(B);

  PredicatesGuardEmitter Preds(OS);
  for (SignatureImplMap::const_iterator
       SI = M.begin(), SE = M.end(); SI != SE; ++SI) {
    const BuiltinSign &Sign = SI->first;
    const OCLStrategy &Strategy = SI->second->getStrategy();

    EmitRequirements(OS, *SI->second, EmittedReqs);

    Preds.Push(ComputePredicates(Sign));

    if (M.size() > 1)
      OS << "__opencrun_overload\n";

    EmitOCLTypeSignature(OS, *Sign[0]);
    OS << " ";
    OS << BD.getInternalName();
    OS << "(";
    std::string ParamName = "param";
    for (unsigned i = 1, e = Sign.size(); i != e; ++i) {
      EmitOCLTypeSignature(OS, *Sign[i], ParamName + llvm::Twine(i).str());
      if (i + 1 != e) OS << ", ";
    }
    OS << ") {\n";

    EmitImplementationBody(OS, B, Sign, Strategy);

    OS << "}\n\n";
  }
  Preds.Finalize();
}

static void EmitOCLCastBuiltinImpls(llvm::raw_ostream &OS,
                                    const OCLCastBuiltin &B,
                                    const SignatureImplMap &M,
                                    RequirementSet &EmittedReqs) {
  OCLBuiltinDecorator BD(B);

  typedef MapKeyIterator<BuiltinSign, const OCLBuiltinImpl *> ImplMapKeyIter;
  std::list<std::pair<unsigned, ImplMapKeyIter> > Ranges;
  ComputeSignsRanges(ImplMapKeyIter(M.begin()), ImplMapKeyIter(M.end()),
                     Ranges);

  PredicatesGuardEmitter Preds(OS);
  for (SignatureImplMap::const_iterator
       SI = M.begin(), SE = M.end(); SI != SE; ++SI) {
    const BuiltinSign &Sign = SI->first;
    const OCLStrategy &Strategy = SI->second->getStrategy();

    EmitRequirements(OS, *SI->second, EmittedReqs);

    if (Ranges.front().second == ImplMapKeyIter(SI)) Ranges.pop_front();

    Preds.Push(ComputePredicates(Sign));

    if (Ranges.front().first > 1)
      OS << "__opencrun_overload\n";

    EmitOCLTypeSignature(OS, *Sign[0]);
    OS << " ";
    OS << BD.getInternalName(&Sign);
    OS << "(";
    std::string ParamName = "param";
    for (unsigned i = 1, e = Sign.size(); i != e; ++i) {
      EmitOCLTypeSignature(OS, *Sign[i], ParamName + llvm::Twine(i).str());
      if (i + 1 != e) OS << ", ";
    }
    OS << ") {\n";

    EmitImplementationBody(OS, B, Sign, Strategy);

    OS << "}\n\n";
  }
  Preds.Finalize();
}

bool opencrun::EmitOCLBuiltinImpls(llvm::raw_ostream &OS, 
                                   llvm::RecordKeeper &R) {
  LoadOCLTypes(R, OCLTypes);
  LoadOCLBuiltins(R, OCLBuiltins);
  LoadOCLBuiltinImpls(R, OCLBuiltinImpls);

  BuiltinImplMap Impls;
  BuiltinSignMap Signs;
  for (unsigned i = 0, e = OCLBuiltinImpls.size(); i != e; ++i) {
    const OCLBuiltinImpl &Impl = *OCLBuiltinImpls[i];
    const OCLBuiltin &B = Impl.getBuiltin();
    OCLBuiltin::var_iterator VI = B.find(Impl.getVariantName());

    std::list<const OCLBuiltinVariant *> CurVariants;
    if (VI != B.end()) {
      CurVariants.push_back(VI->second);
    } else if (Impl.getVariantName() == "") {
      for (OCLBuiltin::var_iterator I = B.begin(), E = B.end(); I != E; ++I)
        CurVariants.push_back(I->second);
    } else {
      llvm::PrintWarning("Unknown variant '" + Impl.getVariantName() + "' "
                         "for builtin '" + B.getName() + "'");
      continue;
    }

    BuiltinSignList l;
    for (std::list<const OCLBuiltinVariant*>::iterator
         I = CurVariants.begin(), E = CurVariants.end(); I != E; ++I)
      ExpandSigns(**I, l);

    for (BuiltinSignList::iterator I = l.begin(), 
         E = l.end(); I != E; ++I) {
      if (!Impls[&B].count(*I) || Impl.isTarget() || 
          Impls[&B][*I]->getVariantName() == "")
        Impls[&B][*I] = &Impl;
    }
  }

  emitSourceFileHeader("OCL Builtin implementations", OS);

  RequirementSet EmittedReqs;
  GroupGuardEmitter Groups(OS);

  for (unsigned i = 0, e = OCLBuiltins.size(); i != e; ++i) {
    const OCLBuiltin &B = *OCLBuiltins[i];

    BuiltinImplMap::iterator BI = Impls.find(&B);
    if (BI == Impls.end()) continue;

    if (Groups.Push(B.getGroup()))
      EmittedReqs.clear();

    if (const OCLGenericBuiltin *GB = llvm::dyn_cast<OCLGenericBuiltin>(&B))
      EmitOCLGenericBuiltinImpls(OS, *GB, BI->second, EmittedReqs);
    else if (const OCLCastBuiltin *CB = llvm::dyn_cast<OCLCastBuiltin>(&B))
      EmitOCLCastBuiltinImpls(OS, *CB, BI->second, EmittedReqs);
    else
      llvm::PrintFatalError("Invalid builtin!");
  }
  Groups.Finalize();
  
  return false;
}
