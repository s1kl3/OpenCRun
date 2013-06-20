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

void EmitOCLTypedefs(llvm::raw_ostream &OS, 
                     const OCLStrategy &Strategy,
                     BuiltinSignature &Sign) {

  for (OCLStrategy::iterator TI = Strategy.begin(),
                             TE = Strategy.end();
                             TI != TE;
                             ++TI) {
    if (llvm::isa<OCLEmitTypedefActual>(*TI)) {
      const OCLBasicType *TypeFrom = (*TI)->getParam().get(Sign);
      std::string TypeTo = (*TI)->getName();
  
      OS << "\n";
      OS.indent(2) << "typedef ";
      EmitOCLTypeSignature(OS, *TypeFrom);
      OS << " " << TypeTo << ";\n";

    } else if (llvm::isa<OCLEmitTypedefUnsigned>(*TI)) {
      std::string TypeTo = (*TI)->getName();
      const OCLBasicType *TypeFrom = (*TI)->getParam().get(Sign);
      bool IsUnsigned = false;

      if (const OCLIntegerType *Int = llvm::dyn_cast<OCLIntegerType>(TypeFrom)) {
        if (Int->isUnsigned())
          IsUnsigned = true;
      } else if (const OCLPointerType *Ptr = llvm::dyn_cast<OCLPointerType>(TypeFrom)) {
        if (const OCLIntegerType *Int = llvm::dyn_cast<OCLIntegerType>(&Ptr->getBaseType())) {
          if (Int->isUnsigned())
            IsUnsigned = true;
        } else
          llvm::PrintFatalError("Invalid source type for typedef!");

      } else
        llvm::PrintFatalError("Invalid source type for typedef!");

      if (IsUnsigned) {
        OS << "\n";
        OS.indent(2) << "typedef ";
      } else {
        OS << "\n";
        OS.indent(2) << "typedef unsigned ";
      }

      EmitOCLTypeSignature(OS, *TypeFrom);

      OS << " " << TypeTo << ";\n";
    } else
      llvm::PrintFatalError("Invalid typedef!");
  }

}

void EmitOCLRecursiveSplit(llvm::raw_ostream &OS,
                           const OCLBuiltin &BuiltIn, 
                           const RecursiveSplit &Split,
                           BuiltinSignature &Sign) {

  if (IsScalarAlternative(Sign)) {
    std::string ScalarImpl = Split.getScalarImpl();
    EmitOCLTypedefs(OS, Split, Sign);
    OS << ScalarImpl << "\n";
  } else {
    const OCLVectorType *RetTy;
    RetTy = llvm::dyn_cast<OCLVectorType>(Sign[0]);
    if (!RetTy)
      llvm::PrintFatalError("Not a vector return type for recursive split!");

    unsigned N = RetTy->getWidth();

    BuiltinSignature Sign_1, Sign_2;
    IndexesContainer Indexes_1, Indexes_2;

    Sign_1.reserve(Sign.size());
    Sign_2.reserve(Sign.size());
    Indexes_1.reserve(Sign.size());
    Indexes_2.reserve(Sign.size());

    for (unsigned i = 0, e = Sign.size(); i != e; ++i) {
      if (const OCLScalarType *Ty = llvm::dyn_cast<OCLScalarType>(Sign[i])) {
        Sign_1.push_back(Ty);
        Sign_2.push_back(Ty);
        Indexes_1.push_back("");
        Indexes_2.push_back("");
      } else if (const OCLVectorType *Ty = llvm::dyn_cast<OCLVectorType>(Sign[i])) {
        if (Ty->getWidth() == N) {
          unsigned N_1 = N/2;
          unsigned N_2 = N - N_1;

          if ((N_1 == 1) && (N_2 == 1)) {
            Sign_1.push_back(&Ty->getBaseType());
            Sign_2.push_back(&Ty->getBaseType());
            Indexes_1.push_back(".s0");
            Indexes_2.push_back(".s1");
          } else if ((N_1 == 1) && (N_2 == 2)) {
            Sign_1.push_back(&Ty->getBaseType());
            Indexes_1.push_back(".s0");
            Sign_2.push_back(OCLTypesTable::getVectorType(Ty->getBaseType(), 2));
            Indexes_2.push_back(".s12");
          } else if ((N_1 == 2) && (N_2 == 2)) {
            Sign_1.push_back(OCLTypesTable::getVectorType(Ty->getBaseType(), 2));
            Indexes_1.push_back(".s01");
            Sign_2.push_back(OCLTypesTable::getVectorType(Ty->getBaseType(), 2));
            Indexes_2.push_back(".s23");
          } else if ((N_1 == 4) && (N_2 == 4)) {
            Sign_1.push_back(OCLTypesTable::getVectorType(Ty->getBaseType(), 4));
            Indexes_1.push_back(".s0123");
            Sign_2.push_back(OCLTypesTable::getVectorType(Ty->getBaseType(), 4));
            Indexes_2.push_back(".s4567");
          } else if ((N_1 == 8) && (N_2 == 8)) {
            Sign_1.push_back(OCLTypesTable::getVectorType(Ty->getBaseType(), 8));
            Indexes_1.push_back(".s01234567");
            Sign_2.push_back(OCLTypesTable::getVectorType(Ty->getBaseType(), 8));
            Indexes_2.push_back(".s89abcdef");
          } else
            llvm::PrintFatalError("Unsupported split size for paramters!");

        } else 
          llvm::PrintFatalError("Wrong parameter type size!");

      } else
        llvm::PrintFatalError("Unsupported parameter type!");

    } 

    // First branch
    OS << "\n";
    OS.indent(2) << *Sign_1[0] << " Tmp_1 = (" << *Sign_1[0] << ") __builtin_ocl_"
      << BuiltIn.getName() << "(\n";
    for (unsigned PI = 1, PE = Sign_1.size(); PI != PE; ++PI) {
      OS.indent(8) << "param" << PI;
      OS << Indexes_1[PI];
      if (PI < PE - 1)
        OS << ", \n";
      else if (PI == PE - 1) {
        OS << "\n";
        OS.indent(2) << ");\n\n";
      }
    }

    // Second branch
    OS.indent(2) << *Sign_2[0] << " Tmp_2 = (" << *Sign_2[0] << ") __builtin_ocl_"
      << BuiltIn.getName() << "(\n";
    for (unsigned PI = 1, PE = Sign_2.size(); PI != PE; ++PI) {
      OS.indent(8) << "param" << PI;
      OS << Indexes_2[PI];
      if (PI < PE - 1)
        OS << ", \n";
      else if (PI == PE - 1) {
        OS << "\n";
        OS.indent(2) << ");\n\n";
      }
    }

    OS.indent(2) << *Sign[0] << " RetValue = (" << *Sign[0] 
      << ") (Tmp_1, Tmp_2);\n\n";

    OS.indent(2) << "return RetValue;\n\n";
  }

}

void EmitOCLDirectSplit(llvm::raw_ostream &OS,
                        const OCLBuiltin &BuiltIn, 
                        const DirectSplit &Split,
                        BuiltinSignature &Sign) {

  if (IsScalarAlternative(Sign)) {
    std::string ScalarImpl = Split.getScalarImpl();
    EmitOCLTypedefs(OS, Split, Sign);
    OS << ScalarImpl << "\n";
  } else {
    const OCLVectorType *RetTy;
    RetTy = llvm::dyn_cast<OCLVectorType>(Sign[0]);
    if (!RetTy)
      llvm::PrintFatalError("Not a vector return type for direct split!");

    unsigned N = RetTy->getWidth();                       

    OS << "\n";
    OS.indent(2) << *Sign[0] << " RetValue = (" << *Sign[0] <<") (\n"; 
    for (unsigned i = 0; i < N; ++i) {
      OS.indent(4) << "__builtin_ocl_" << BuiltIn.getName() << "(\n";

      for (unsigned PI = 1, PE = Sign.size(); PI != PE; ++PI) {
        if (llvm::isa<OCLScalarType>(Sign[PI])) {
          OS.indent(8) << "param" << PI;
        } else if (llvm::isa<OCLVectorType>(Sign[PI])) {
          OS.indent(8) << "param" << PI << ".s";
          OS.write_hex(i);
        } else if (llvm::isa<OCLPointerType>(Sign[PI])) {
          const OCLPointerType *PtrTy = llvm::cast<OCLPointerType>(Sign[PI]);
          
          if (llvm::isa<OCLScalarType>(PtrTy->getBaseType()))
            OS.indent(8) << "param" << PI;
          else if (const OCLVectorType *BaseTy = llvm::dyn_cast<OCLVectorType>(&PtrTy->getBaseType())) {
            
            OS.indent(8) << "(";
            
            // Modifiers
            unsigned Modifiers = PtrTy->getModifierFlags();
            if (Modifiers & OCLPointerType::M_Const)
              OS << "const ";
            
            // Base type
            OS << BaseTy->getBaseType() << " ";

            // Address Space
            OS << AddressSpaceName(PtrTy->getAddressSpace());

            OS << " *) param" << PI;
            if (i > 0)
              OS << " + " << i;

          } else
            llvm::PrintFatalError("Pointee type not supported!");

        } else
          llvm::PrintFatalError("Parameter type not supported!");

        if (PI < PE -1)
          OS << ", \n";
        else if (PI == PE - 1) {
          OS << "\n";
          OS.indent(4) << ")";
        }
      }

      if (i < N - 1)
        OS << ", \n";
      else if (i == N - 1) {
        OS << "\n";
        OS.indent(2) << ");\n\n";
      }

    }

    OS.indent(2) << "return RetValue;\n\n";

  }

}

void EmitOCLMergeSplit(llvm::raw_ostream &OS,
                       const OCLBuiltin &BuiltIn, 
                       const MergeSplit &Split,
                       BuiltinSignature &Sign) {

  if (IsScalarAlternative(Sign)) {
    std::string ScalarImpl = Split.getScalarImpl();
    EmitOCLTypedefs(OS, Split, Sign);
    OS << ScalarImpl << "\n";
  } else {
    if (!llvm::isa<OCLScalarType>(Sign[0]))
      llvm::PrintFatalError("Not a scalar return type for merge split!");

    unsigned N;
    for (unsigned SI = 1; SI < Sign.size(); ++SI) {
      if (const OCLVectorType *VTy = llvm::dyn_cast<OCLVectorType>(Sign[SI])) {
        N = VTy->getWidth();
        break;
      }
    }

    OS << "\n";
    OS.indent(2) << "return\n";
    for (unsigned i = 0; i < N; ++i) {
      OS.indent(4) << "__builtin_ocl_" << BuiltIn.getName() << "(\n";

      for (unsigned PI = 1, PE = Sign.size(); PI != PE; ++PI) {
        if (llvm::isa<OCLScalarType>(Sign[PI])) {
          OS.indent(8) << "param" << PI;
        } else if (llvm::isa<OCLVectorType>(Sign[PI])) {
          OS.indent(8) << "param" << PI << ".s";
          OS.write_hex(i);
        } else if (llvm::isa<OCLPointerType>(Sign[PI])) {
          const OCLPointerType *PtrTy = llvm::cast<OCLPointerType>(Sign[PI]);
          
          if (llvm::isa<OCLScalarType>(PtrTy->getBaseType()))
            OS.indent(8) << "param" << PI;
          else if (const OCLVectorType *BaseTy = llvm::dyn_cast<OCLVectorType>(&PtrTy->getBaseType())) {
            
            OS.indent(8) << "(";
            
            // Modifiers
            unsigned Modifiers = PtrTy->getModifierFlags();
            if (Modifiers & OCLPointerType::M_Const)
              OS << "const ";
            
            // Base type
            OS << BaseTy->getBaseType() << " ";

            // Address Space
            OS << AddressSpaceName(PtrTy->getAddressSpace());

            OS << " *) param" << PI;
            if (i > 0)
              OS << " + " << i;

          } else
            llvm::PrintFatalError("Pointee type not supported!");

        } else
          llvm::PrintFatalError("Parameter type not supported!");

        if (PI < PE -1)
          OS << ", \n";
        else if (PI == PE - 1) {
          OS << "\n";
          OS.indent(4) << ")";
        }
      }

      if (i < N - 1)
        OS << " " << Split.getMergeOp() << " \n";
      else if (i == N - 1)
        OS << ";\n\n";

    }

  }
  
}

void EmitOCLTemplateStrategy(llvm::raw_ostream &OS,
                             const OCLBuiltin &BuiltIn, 
                             const TemplateStrategy &Template,
                             BuiltinSignature &Sign) {

    std::string ScalarImpl = Template.getScalarImpl();
    EmitOCLTypedefs(OS, Template, Sign);
    OS << ScalarImpl << "\n";

}

void EmitOCLBuiltinImplementation(llvm::raw_ostream &OS, const OCLBuiltinImpl &B) {

  const OCLBuiltin &BuiltIn = B.getBuiltin();
  const std::string VariantId = B.getVariantId();
  const OCLStrategy &Strategy = B.getStrategy();

  EmitBuiltinGroupBegin(OS, BuiltIn.getGroup());
  
  OS << "// Group: " << BuiltIn.getGroup() << "\n";
  OS << "// Builtin: " << BuiltIn.getName() << "\n";

  if (!VariantId.empty()) // VariantId non vuota
    OS << "// Variant: " << VariantId << "\n";

  OS << "\n";

  std::list<BuiltinSignature> Alts;

  for (OCLBuiltin::iterator BI = BuiltIn.begin(), BE = BuiltIn.end(); BI != BE; ++BI) {
    std::string BV_Id = BI->getVariantId();
    
    if (VariantId.empty() || (BV_Id.compare(VariantId) == 0)) {
      Alts.clear();
      ExpandSignature(*BI, Alts);

      SortBuiltinSignatureList(Alts);

      llvm::BitVector GroupPreds;
      
      for (std::list<BuiltinSignature>::iterator I = Alts.begin(), 
                                                 E = Alts.end(); 
                                                 I != E; 
                                                 ++I) {
        BuiltinSignature &Sign = *I;

        llvm::BitVector Preds;
        ComputePredicates(Sign, Preds);
        if (Preds != GroupPreds) {
          EmitPredicatesEnd(OS, GroupPreds);
          GroupPreds = Preds;
          EmitPredicatesBegin(OS, GroupPreds);
          OS << "\n";
        }
       
        OS << "__opencrun_overload\n";
        EmitOCLTypeSignature(OS, *Sign[0]);
        OS << " ";
        OS << "__builtin_ocl_" << BuiltIn.getName();
        OS << "(";
        
        std::string ParamName = "param";
        for (unsigned SI = 1, SE = Sign.size(); SI != SE; ++SI) {
          EmitOCLTypeSignature(OS, *Sign[SI], ParamName + llvm::Twine(SI).str());
          if (SI + 1 != SE) OS << ", ";
        }
        
        OS << ") ";
        OS << "{\n";

        if (const RecursiveSplit *RSplit = llvm::dyn_cast<const RecursiveSplit>(&Strategy))
          EmitOCLRecursiveSplit(OS, BuiltIn, *RSplit, Sign);
        else if (const DirectSplit *DSplit = llvm::dyn_cast<const DirectSplit>(&Strategy))
          EmitOCLDirectSplit(OS, BuiltIn, *DSplit, Sign);
        else if (const MergeSplit *MSplit = llvm::dyn_cast<const MergeSplit>(&Strategy))
          EmitOCLMergeSplit(OS, BuiltIn, *MSplit, Sign);
        else if (const TemplateStrategy *Tmplt = llvm::dyn_cast<const TemplateStrategy>(&Strategy))
          EmitOCLTemplateStrategy(OS, BuiltIn, *Tmplt, Sign);
        else
          llvm::PrintFatalError("Invalid strategy!");
  
        OS << "}\n\n";
      }

      EmitPredicatesEnd(OS, GroupPreds);
      OS << "\n";
    }
  }

  EmitBuiltinGroupEnd(OS, BuiltIn.getGroup());

}

bool opencrun::EmitOCLBuiltinImpl(llvm::raw_ostream &OS, 
                                  llvm::RecordKeeper &R) {
  LoadOCLTypes(R, OCLTypes);
  LoadOCLBuiltins(R, OCLBuiltins);
  LoadOCLBuiltinImpls(R, OCLBuiltinImpls);

  emitSourceFileHeader("OCL Builtin implementations", OS);

  for (unsigned I = 0, E = OCLBuiltinImpls.size(); I != E; ++I)
    EmitOCLBuiltinImplementation(OS, *OCLBuiltinImpls[I]);  
  
  return false;
}
