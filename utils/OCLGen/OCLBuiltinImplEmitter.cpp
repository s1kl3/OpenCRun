#include "OCLBuiltinImplEmitter.h"
#include "OCLEmitterUtils.h"
#include "OCLBuiltin.h"
#include "OCLType.h"

#include "llvm/TableGen/Error.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace opencrun;

namespace {

OCLTypesContainer OCLTypes;
OCLBuiltinsContainer OCLBuiltins;
OCLBuiltinImplsContainer OCLBuiltinImpls;

}

typedef std::vector<std::string> IndexesContainer;

static void EmitOCLDecls(llvm::raw_ostream &OS, 
                         const BuiltinSignature &Sign,
                         const OCLStrategy &S) {
  for (OCLStrategy::decl_iterator DI = S.begin(), DE = S.end(); 
       DI != DE; ++DI) {
    if (const OCLTypedefDecl *T = llvm::dyn_cast<OCLTypedefDecl>(*DI)) {
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

    } else
      llvm::PrintFatalError("Illegal OCLDecl!");
  }
}

static bool IsScalarSignature(const BuiltinSignature &Sign) {
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
                                   const BuiltinSignature &Sign,
                                   const OCLRecursiveSplit &S) {
  if (IsScalarSignature(Sign)) {
    EmitOCLDecls(OS, Sign, S);
    OS << S.getScalarImpl() << "\n";
    return;
  }

  BuiltinSignature LeftSign, RightSign;
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

  // Left branch
  OS.indent(2) << *LeftSign[0] << " Tmp1 = (" << *LeftSign[0] << ") "
               << "__builtin_ocl_" << B.getName() << "(\n";
  for (unsigned i = 1, e = LeftSign.size(); i != e; ++i) {
    OS.indent(8) << "param" << i;
    EmitSubvector(OS, LeftRanges[i]);
    if (i + 1 != e)
      OS << ", \n";
    else {
      OS << "\n";
      OS.indent(2) << ");\n\n";
    }
  }

  // Right branch
  OS.indent(2) << *RightSign[0] << " Tmp2 = (" << *RightSign[0] << ") "
               << "__builtin_ocl_" << B.getName() << "(\n";
  for (unsigned i = 1, e = RightSign.size(); i != e; ++i) {
    OS.indent(8) << "param" << i;
    EmitSubvector(OS, RightRanges[i]);
    if (i + 1 != e)
      OS << ", \n";
    else {
      OS << "\n";
      OS.indent(2) << ");\n\n";
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

static void EmitImplementationBody(llvm::raw_ostream &OS,
                                   const OCLBuiltin &B,
                                   const BuiltinSignature &Sign,
                                   const OCLDirectSplit &S) {
  if (IsScalarSignature(Sign)) {
    EmitOCLDecls(OS, Sign, S);
    OS << S.getScalarImpl() << "\n";
    return;
  }

  const OCLVectorType *RetTy = llvm::dyn_cast<OCLVectorType>(Sign[0]);
  if (!RetTy)
    llvm::PrintFatalError("DirectSplit: '" + B.getName() + 
                          "' has not a vector return type");

  unsigned N = RetTy->getWidth();

  OS.indent(2) << "return (" << *RetTy << ")(\n";

  for (unsigned i = 0; i != N; ++i) {
    OS.indent(6) << "__builtin_ocl_" << B.getName() << "(\n";
    for (unsigned PI = 1, PE = Sign.size(); PI != PE; ++PI) {
      const OCLBasicType *ArgTy = Sign[PI];
      if (llvm::isa<OCLScalarType>(ArgTy)) {
        OS.indent(8) << "param" << PI;
      } else if (llvm::isa<OCLVectorType>(ArgTy)) {
        OS.indent(8) << "param" << PI << ".s";
        OS.write_hex(i);
      } else {
        assert(llvm::isa<OCLPointerType>(ArgTy));
        const OCLPointerType *PtrTy = llvm::cast<OCLPointerType>(ArgTy);
        const OCLType &PTy = PtrTy->getBaseType();

        if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(&PTy)) {
          OS.indent(8) << "(";

          OCLPtrStructure PtrS;
          PtrS.push_back(OCLPtrDesc(PtrTy->getAddressSpace(), 
                                    PtrTy->getModifierFlags()));
          const OCLType &CastTy = 
            *OCLTypesTable::getPointerType(V->getBaseType(), PtrS);
          EmitOCLTypeSignature(OS, CastTy, "");

          OS << ") param" << PI;
          if (i > 0) OS << " + " << i;
        } else if (llvm::isa<OCLPointerType>(&PTy)) {
          OS.indent(8) << "param" << PI;
        } else
          llvm::PrintFatalError("Type not supported by DirectSplit strategy!");
      }
      if (PI + 1 != PE) OS << ",\n";
    }
    OS.indent(6) << ")";

    if (i + 1 != N) OS << ",\n";
  }

  OS.indent(4) << ");\n";
}

static void EmitImplementationBody(llvm::raw_ostream &OS,
                                   const OCLBuiltin &B, 
                                   const BuiltinSignature &Sign,
                                   const OCLTemplateStrategy &S) {
  EmitOCLDecls(OS, Sign, S);
  OS << S.getTemplateImpl() << "\n";
}

static void EmitImplementation(llvm::raw_ostream &OS,
                               const OCLBuiltin &B,
                               const BuiltinSignature &Sign,
                               const OCLStrategy &S) {
  OS << "__opencrun_overload\n";
  EmitOCLTypeSignature(OS, *Sign[0]);
  OS << " ";
  OS << "__builtin_ocl_" << B.getName();
  OS << "(";
  std::string ParamName = "param";
  for (unsigned i = 1, e = Sign.size(); i != e; ++i) {
    EmitOCLTypeSignature(OS, *Sign[i], ParamName + llvm::Twine(i).str());
    if (i + 1 != e) OS << ", ";
  }
  OS << ") {\n";

  if (const OCLRecursiveSplit *RS = llvm::dyn_cast<OCLRecursiveSplit>(&S))
    EmitImplementationBody(OS, B, Sign, *RS);
  else if (const OCLDirectSplit *DS = llvm::dyn_cast<OCLDirectSplit>(&S))
    EmitImplementationBody(OS, B, Sign, *DS);
  else if (const OCLTemplateStrategy *TS = 
             llvm::dyn_cast<OCLTemplateStrategy>(&S))
    EmitImplementationBody(OS, B, Sign, *TS);
  else
    llvm::PrintFatalError("Illegal strategy!");

  OS << "}\n";
}

bool opencrun::EmitOCLBuiltinImpl(llvm::raw_ostream &OS, 
                                  llvm::RecordKeeper &R) {
  LoadOCLTypes(R, OCLTypes);
  LoadOCLBuiltins(R, OCLBuiltins);
  LoadOCLBuiltinImpls(R, OCLBuiltinImpls);

  typedef std::map<BuiltinSignature, const OCLBuiltinImpl *> SignatureImplMap;
  typedef std::map<const OCLBuiltin *, SignatureImplMap> BuiltinImplMap;

  BuiltinImplMap Impls;
  for (unsigned i = 0, e = OCLBuiltinImpls.size(); i != e; ++i) {
    const OCLBuiltinImpl &Impl = *OCLBuiltinImpls[i];
    const OCLBuiltin &B = Impl.getBuiltin();
    OCLBuiltin::iterator VI = B.find(Impl.getVariantName());

    std::list<const OCLBuiltinVariant *> CurVariants;
    if (VI != B.end()) {
      CurVariants.push_back(VI->second);
    } else if (Impl.getVariantName() == "") {
      for (OCLBuiltin::iterator VI = B.begin(), VE = B.end(); VI != VE; ++VI)
        CurVariants.push_back(VI->second);
    } else {
      llvm::PrintWarning("Unknown variant '" + Impl.getVariantName() + "' "
                         "for builtin '" + B.getName() + "'");
      continue;
    }

    BuiltinSignatureList l;
    for (std::list<const OCLBuiltinVariant*>::iterator
         I = CurVariants.begin(), E = CurVariants.end(); I != E; ++I)
      ExpandSignature(**I, l);

    for (BuiltinSignatureList::iterator I = l.begin(), 
         E = l.end(); I != E; ++I) {
      if (!Impls[&B].count(*I) || Impls[&B][*I]->getVariantName() == "")
        Impls[&B][*I] = &Impl;
    }
  }

  emitSourceFileHeader("OCL Builtin implementations", OS);

  llvm::StringRef GlobalGroup = "";
  for (BuiltinImplMap::const_iterator BI = Impls.begin(), 
       BE = Impls.end(); BI != BE; ++BI) {
    llvm::StringRef Group = BI->first->getGroup();
    if (Group != GlobalGroup) {
      EmitBuiltinGroupEnd(OS, GlobalGroup);
      GlobalGroup = Group;
      OS << "\n";
      EmitBuiltinGroupBegin(OS, GlobalGroup);
    }

    llvm::BitVector GroupPreds;
    for (SignatureImplMap::const_iterator SI = BI->second.begin(),
         SE = BI->second.end(); SI != SE; ++SI) {
      llvm::BitVector Preds;
      ComputePredicates(SI->first, Preds);
      if (GroupPreds != Preds) {
        EmitPredicatesEnd(OS, GroupPreds);
        GroupPreds = Preds;
        OS << "\n";
        EmitPredicatesBegin(OS, GroupPreds);
      }

      EmitImplementation(OS, *BI->first, SI->first, SI->second->getStrategy());
    }
    EmitPredicatesEnd(OS, GroupPreds);
    OS << "\n";
  }
  EmitBuiltinGroupEnd(OS, GlobalGroup);
  
  return false;
}
