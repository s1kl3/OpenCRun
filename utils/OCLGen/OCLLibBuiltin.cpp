
#include "OCLLibBuiltin.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"

using namespace opencrun;

namespace {

//
// OCLTypeTable implementation.
//

class OCLTypeTable {
public:
  typedef std::map<llvm::Record *, OCLType *> TypesContainer;

public:
  ~OCLTypeTable() { DeleteContainerSeconds(Types); }

public:
  OCLType *GetType(llvm::Record &R) {
    if(!Types.count(&R))
      BuildType(R);

    return Types[&R];
  }

  OCLType *GetType(llvm::StringRef Name) {
    TypesContainer::iterator I, E;

    for(I = Types.begin(), E = Types.end(); I != E; ++I) {
      OCLType *Type = I->second;
      if(Type->GetName() == Name)
        return Type;
    }

    llvm::PrintFatalError("Type not found: " + Name.str());
  }

private:
  void BuildType(llvm::Record &R) {
    OCLType *Type;

    // OpenCL C scalar types.
    if(R.isSubClassOf("OCLScalarType"))
      Type = new OCLScalarType(R.getName());

    // OpenCL C vector types.
    else if(R.isSubClassOf("OCLVectType")) {
      OCLType *RawBaseType = GetType(*R.getValueAsDef("BaseType"));
      int64_t Width = R.getValueAsInt("Width");

      OCLScalarType *BaseType = llvm::dyn_cast<OCLScalarType>(RawBaseType);
      if(!BaseType)
        llvm::PrintFatalError("Cannot construct a vector for "
                              "a non-scalar type: " + RawBaseType->GetName());

      Type = new OCLVectType(*BaseType, Width);
    }

    // Generic types.
    else if(R.isSubClassOf("OCLGenType"))
      Type = new OCLGenType(R.getValueAsInt("N"));

    // Unknown types.
    else
      llvm::PrintFatalError("Cannot construct a type for " + R.getName());

    Types[&R] = Type;
  }

private:
  TypesContainer Types;
};

class OCLAttributeTable {
public:
  typedef std::map<llvm::Record *, OCLAttribute *> AttributeContainer;

public:
  ~OCLAttributeTable() { DeleteContainerSeconds(Attrs); }

public:
  OCLAttribute *GetAttribute(llvm::Record &R) {
    if(!Attrs.count(&R))
      Attrs[&R] = new OCLAttribute(R.getName());

    return Attrs[&R];
  }

private:
  AttributeContainer Attrs;
};

OCLTypeTable TypeTable;
OCLAttributeTable AttributeTable;

} // End anonymous namespace.

//
// OCLScalarType implementation.
//

std::string OCLScalarType::BuildName(llvm::StringRef Name) {
  if(!Name.startswith("ocl_"))
    llvm::PrintFatalError("Scalar type names must begin with ocl_: " +
                          Name.str());

  llvm::StringRef StrippedName = Name.substr(4);

  if(StrippedName.empty())
    llvm::PrintFatalError("Scalar type names must have a non-empty suffix: " +
                          Name.str());

  // TODO: perform more accurate checks.

  return StrippedName.str();
}

//
// OCLGenType implementation.
//

std::string OCLGenType::BuildName(int64_t N) {
  return "gentype" + llvm::utostr(N);
}

//
// OCLVectType implementation.
//

std::string OCLVectType::BuildName(OCLScalarType &BaseType, int64_t Width) {
  return BaseType.GetName() + llvm::utostr(Width);
}

//
// OCLLibBuiltin implementation.
//

OCLLibBuiltin::OCLLibBuiltin(llvm::Record &R) {  
  if(R.isSubClassOf("CommonBuiltin"))
    BuiltinTy = CommonBuiltin;
  else if(R.isSubClassOf("IntegerBuiltin"))
    BuiltinTy = IntegerBuiltin;
  else if(R.isSubClassOf("MathBuiltin"))
    BuiltinTy = MathBuiltin;
  else if(R.isSubClassOf("Relational_1_Builtin"))
    BuiltinTy = Relational_1_Builtin;
  else if(R.isSubClassOf("Relational_2_Builtin"))
    BuiltinTy = Relational_2_Builtin;
  
  // Parse name.
  llvm::StringRef RawName = R.getName();
  if(RawName.startswith("blt_common_"))
    Name.assign(RawName.begin() + 11, RawName.end());
  else if(RawName.startswith("blt_integer_"))
    Name.assign(RawName.begin() + 12, RawName.end());
  else if(RawName.startswith("blt_math_"))
    Name.assign(RawName.begin() + 9, RawName.end());
  else if(RawName.startswith("blt_relational_"))
    Name.assign(RawName.begin() + 15, RawName.end());
  else
    llvm::PrintFatalError("Builtin name must starts with blt_<libname>: " +
                          RawName.str());

  // Parse return types for each built-in form.
  std::vector<llvm::Record *> Returns = R.getValueAsListOfDefs("RetTypes");

  for(unsigned I = 0, E = Returns.size(); I != E; ++I)
    ReturnTys.push_back(TypeTable.GetType(*Returns[I]));

  // Parse parameter list for each built-in form.
  llvm::ListInit *ParamTypes = R.getValueAsListInit("ParamTypes");
  
  for(unsigned I = 0; I < GetAlternativesCount(); ++I) {
    llvm::Init *CurElem = ParamTypes->getElement(I);
    llvm::ListInit *CurParams = llvm::dyn_cast<llvm::ListInit>(CurElem);

    if(!CurParams)
      llvm::PrintFatalError("Expected parameter types list for built-in alternative. Found: "
                            + CurElem->getAsString());

    ParamTysEntry CurParamTys;
    for(unsigned J = 0, F = CurParams->size(); J != F; ++J) {
      llvm::Record *CurParamTy = CurParams->getElementAsRecord(J);
      CurParamTys.push_back(TypeTable.GetType(*CurParamTy));
    }
    ParamTys.push_back(CurParamTys);
  }

  // Parse gentype mappings for return type and parameter types for all
  // built-in alternatives.
  llvm::ListInit *Mappings = R.getValueAsListInit("GentypeSubs");
  
  for(unsigned I = 0; I < GetAlternativesCount(); ++I) {
    
    // Get return OCLType for built-in alternative I and check if it's an OCLGenType.
    if(OCLGenType *GenType = llvm::dyn_cast<OCLGenType>(&GetReturnType(I))) {
      unsigned N = static_cast<unsigned>(GenType->getN());

      // Get the corresponding element from specialization list.
      llvm::Init *CurElem = Mappings->getElement(N - 1);
      llvm::ListInit *CurMappings = llvm::dyn_cast<llvm::ListInit>(CurElem);

      if(!CurMappings)
        llvm::PrintFatalError("Expected gentype mappings list. Found: " +
                              CurElem->getAsString());

      // Map each type specialization to the gentype. We cannot use a multimap
      // because we need to preserve the order in which specialization have been
      // declared by the user.
      if(!GenTypeSubs.count(GenType))
        for(unsigned J = 0, F = CurMappings->size(); J != F; ++J) {
          llvm::Record *CurSub = CurMappings->getElementAsRecord(J);
          GenTypeSubs[GenType].push_back(TypeTable.GetType(*CurSub));
        }
    }

    for(unsigned J = 0; J < GetParametersCount(); ++J) {
      // Get OCLType corresponding to parameter J for built-in alternative I and
      // check if it's an OCLGenType.
      OCLType &ParamTy = GetParameterType(I, J);
      if(OCLGenType *GenType = llvm::dyn_cast<OCLGenType>(&ParamTy)) {
        unsigned N = static_cast<unsigned>(GenType->getN());
       
        // Get the corresponding element from specialization list.
        llvm::Init *CurElem = Mappings->getElement(N - 1);
        llvm::ListInit *CurMappings = llvm::dyn_cast<llvm::ListInit>(CurElem);

        if(!CurMappings)
          llvm::PrintFatalError("Expected gentype mappings list. Found: " +
                                CurElem->getAsString());

        // Map each type specialization to the gentype. We cannot use a multimap
        // because we need to preserve the order in which specialization have been
        // declared by the user.
        if(!GenTypeSubs.count(GenType))
          for(unsigned K = 0, F = CurMappings->size(); K != F; ++K) {
            llvm::Record *CurSub = CurMappings->getElementAsRecord(K);
            GenTypeSubs[GenType].push_back(TypeTable.GetType(*CurSub));
          }
      }
    }

  }

  // Parse attributes.
  std::vector<llvm::Record *> Attrs = R.getValueAsListOfDefs("Attributes");
  for(unsigned I = 0, E = Attrs.size(); I != E; ++I)
    Attributes.push_back(AttributeTable.GetAttribute(*Attrs[I]));

  // Parse basic built-in implementation.
  BaseImpl = R.getValueAsString("BaseImpl");
}

OCLType &OCLLibBuiltin::GetSpecializedType(OCLType &Ty, unsigned SpecID) {
  if(OCLGenType *GenTy = llvm::dyn_cast<OCLGenType>(&Ty)) {
    if(!GenTypeSubs.count(GenTy))
      llvm::PrintFatalError("Type is not specialized: " + Ty.GetName());

    GenTypeSubsEntry &Subs = GenTypeSubs[GenTy];
    if(SpecID >= Subs.size())
      llvm::PrintFatalError("Not enough specializations");

    return *Subs[SpecID];
  }

  return Ty;
}

//
// LoadOCLBuiltins implementation.
//

OCLBuiltinContainer opencrun::LoadOCLBuiltins(const llvm::RecordKeeper &R) {
  std::vector<llvm::Record *> RawBlts;
  OCLBuiltinContainer Blts;

  RawBlts = R.getAllDerivedDefinitions("GenBuiltin");

  for(unsigned I = 0, E = RawBlts.size(); I < E; ++I)
    Blts.push_back(OCLLibBuiltin(*RawBlts[I]));

  return Blts;
}

//
// LoadOCLVectTypes implementation.
//

OCLVectTypeContainer opencrun::LoadOCLVectTypes(const llvm::RecordKeeper &R) {
  std::vector<llvm::Record *> RawVects;
  OCLVectTypeContainer Vects;

  RawVects = R.getAllDerivedDefinitions("OCLVectType");

  for(unsigned I = 0, E = RawVects.size(); I < E; ++I) {
    OCLType *Ty = TypeTable.GetType(*RawVects[I]);
    Vects.push_back(llvm::cast<OCLVectType>(Ty));
  }

  return Vects;
}
