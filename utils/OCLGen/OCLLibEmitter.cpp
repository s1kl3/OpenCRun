
#include "llvm/TableGen/TableGenBackend.h"

#include "OCLLibEmitter.h"

using namespace opencrun;

namespace {

void EmitHeader(llvm::raw_ostream &OS, 
                OCLLibBuiltin &Blt, 
                unsigned AltID, 
                unsigned SpecID);

void EmitImplementation(llvm::raw_ostream &OS,
                        OCLLibBuiltin &Blt,
                        unsigned AltID,
                        unsigned SpecID);

void EmitBuiltinTypeOpeningGuard(llvm::raw_ostream &OS,
                                 OCLLibBuiltin &Blt); 

void EmitBuiltinTypeClosingGuard(llvm::raw_ostream &OS,
                                 OCLLibBuiltin &Blt); 

void EmitIncludes(llvm::raw_ostream &OS);
void EmitTypes(llvm::raw_ostream &OS, llvm::RecordKeeper &R);

void EmitWorkItemDecls(llvm::raw_ostream &OS);
void EmitAsyncCopyDecls(llvm::raw_ostream &OS);
void EmitSynchronizationDecls(llvm::raw_ostream &OS);
void EmitGeometricDecls(llvm::raw_ostream &OS);

void EmitMacros(llvm::raw_ostream &OS);
void EmitWorkItemRewritingMacros(llvm::raw_ostream &OS);
void EmitSynchronizationRewritingMacros(llvm::raw_ostream &OS);
void EmitAsyncCopyRewritingMacros(llvm::raw_ostream &OS);
void EmitGeometricRewritingMacros(llvm::raw_ostream &OS);

void EmitBuiltinRewritingMacro(llvm::raw_ostream &OS, 
                               OCLLibBuiltin &Blt);

void EmitAutoDecl(llvm::raw_ostream &OS, 
                  OCLType &Ty, 
                  llvm::StringRef Name);

void EmitAutoArrayDecl(llvm::raw_ostream &OS,
                       OCLScalarType &BaseTy,
                       int64_t N,
                       llvm::StringRef Name);

} // End anonymous namespace.

//
// EmitOCLDef implementation.
//

void opencrun::EmitOCLDef(llvm::raw_ostream &OS, llvm::RecordKeeper &R) {
  OCLBuiltinContainer Blts = LoadOCLBuiltins(R);

  llvm::emitSourceFileHeader("OpenCL C library definitions.", OS);

  // Emit opening guard.
  OS << "#ifndef __OPENCRUN_OCLDEF_H\n"
     << "#define __OPENCRUN_OCLDEF_H\n";

  EmitIncludes(OS);

  // Emit C++ opening guard.
  OS << "\n"
     << "#ifdef __cplusplus\n"
     << "extern \"C\" {\n"
     << "#endif\n";

  EmitTypes(OS, R);
  EmitWorkItemDecls(OS);
  EmitSynchronizationDecls(OS);
  EmitAsyncCopyDecls(OS);

  EmitMacros(OS);

  EmitWorkItemRewritingMacros(OS);
  EmitSynchronizationRewritingMacros(OS);
  EmitAsyncCopyRewritingMacros(OS);

  // Emit generic definitions.
  OS << "\n/* BEGIN AUTO-GENERATED PROTOTYPES */\n";
  for(OCLBuiltinContainer::iterator I = Blts.begin(),
                                    E = Blts.end();
                                    I != E;
                                    ++I) {
    for(unsigned J = 0, K = I->GetAlternativesCount(); J != K; ++J) {
      for(unsigned L = 0, M = I->GetSpecializationsCount(); L != M; ++L) {
        OS << "\n";
        EmitHeader(OS, *I, J, L);
        OS << ";";
      }
      OS << "\n";
    }
  }
  OS << "\n/* END AUTO-GENERATED PROTOTYPES */\n";

  // Emit builtin rewriting macros.
  OS << "\n/* BEGIN AUTO-GENERATED BUILTIN MACROS */\n"
     << "\n";
  for(OCLBuiltinContainer::iterator I = Blts.begin(),
                                    E = Blts.end();
                                    I != E;
                                    ++I)
    EmitBuiltinRewritingMacro(OS, *I);
  OS << "\n/* END AUTO-GENERATED BUILTIN MACROS */\n";

  // Emit C++ closing guard.
  OS << "\n"
     << "#ifdef __cplusplus\n"
     << "}\n"
     << "#endif\n";

  // Emit closing guard.
  OS << "\n#endif /* __CLANG_OCLDEF_H */\n";
}

//
// EmitOCLLibImpl implementation.
//

void opencrun::EmitOCLLibImpl(llvm::raw_ostream &OS, llvm::RecordKeeper &R) {
  OCLBuiltinContainer Blts = LoadOCLBuiltins(R);

  llvm::emitSourceFileHeader("OpenCL C generic library implementation.", OS);

  OS << "\n//===- Function prototypes -===//\n\n";

  for(OCLBuiltinContainer::iterator I = Blts.begin(),
                                    E = Blts.end();
                                    I != E;
                                    ++I) {
    // Emit opening guard.
    EmitBuiltinTypeOpeningGuard(OS, *I);

    for(unsigned J = 0, K = I->GetAlternativesCount(); J != K; ++J) {
      for(unsigned L = 0, M = I->GetSpecializationsCount(); L != M; ++L) {
        if((J > 0) && ((L % 6) == 0))
          continue;
        EmitHeader(OS, *I, J, L);
        OS << ";\n";
      }
      OS << "\n";
    }

    // Emit closing guard.
    EmitBuiltinTypeClosingGuard(OS, *I);
  }

  OS << "\n//===- Function definitions -===//\n\n";

  for(OCLBuiltinContainer::iterator I = Blts.begin(),
                                    E = Blts.end();
                                    I != E;
                                    ++I) {
    // Emit opening guard.
    EmitBuiltinTypeOpeningGuard(OS, *I);

    for(unsigned J = 0, K = I->GetAlternativesCount(); J != K; ++J)
      for(unsigned L = 0, M = I->GetSpecializationsCount(); L != M; ++L) {
        EmitImplementation(OS, *I, J, L);
        OS << "\n";
      }

    // Emit closing guard.
    EmitBuiltinTypeClosingGuard(OS, *I);
  }
}

namespace {

void EmitHeader(llvm::raw_ostream &OS, OCLLibBuiltin &Blt, unsigned AltID, unsigned SpecID) {
  // Emit attributes.
  OS << "__attribute__((";
  for(unsigned I = 0, E = Blt.GetAttributesCount(); I != E; ++I) {
    if(I != 0)
      OS << ",";
    OS << Blt.GetAttribute(I);
  }
  OS << "))\n";

  // Emit return type.
  OS << Blt.GetReturnType(AltID, SpecID) << " ";

  // Emit name.
  OS << "__builtin_ocl_" << Blt.GetName();

  // Emit arguments.
  OS << "(";
  for(unsigned I = 0, E = Blt.GetParametersCount(); I != E; ++I) {
    if(I != 0)
      OS << ", ";
    OS << Blt.GetParameterType(AltID, I, SpecID) << " x" << I;
  }
  OS << ")";
}

void EmitImplementation(llvm::raw_ostream &OS,
                        OCLLibBuiltin &Blt,
                        unsigned AltID,
                        unsigned SpecID) {

  if((AltID > 0) && ((SpecID % 6) == 0))
    return;

  EmitHeader(OS, Blt, AltID, SpecID);

  OS << " {\n";

  if((SpecID % 6) == 0) {
    // Base specialization has ID 0: emit user-given code.
    OS << "  " << Blt.GetBaseImplementation() << "\n";
  } else {
    // Recursive case, call base.

    if(Blt.GetType() == OCLLibBuiltin::Relational_2_Builtin) {
      OCLVectType *ParTy;
      ParTy = llvm::dyn_cast<OCLVectType>(&Blt.GetParameterType(AltID, 0, SpecID));
      if(!ParTy)
        llvm::PrintFatalError("Expected vector type. Found: " + ParTy->GetName());

      OS.indent(2);
      OS << "return\n";
      for(unsigned I = 0, E = ParTy->GetWidth(); I != E; ++I) {
        OS.indent(2);
        OS << "__builtin_ocl_" + Blt.GetName();
        OS << "(x0[" << I << "])";
        if(I != E - 1) {
          if(Blt.GetName() == "any")
            OS << " ||\n";
          else if(Blt.GetName() == "all")
            OS << " &&\n";
        }
      }

      OS << ";\n";

    } else {
      OCLVectType *RetTy;
      RetTy = llvm::dyn_cast<OCLVectType>(&Blt.GetReturnType(AltID, SpecID));
      if(!RetTy)
        llvm::PrintFatalError("Expected vector type. Found: " + RetTy->GetName());

      // This variable holds return value in non-vector form.
      OS.indent(2);
      EmitAutoArrayDecl(OS, RetTy->GetBaseType(), RetTy->GetWidth(), "RawRetValue");
      OS << ";\n\n";

      // Call base implementation for each vector element.
      for(unsigned I = 0, E = RetTy->GetWidth(); I != E; ++I) {
        // The i-th call writes the i-th element of the return value.
        OS.indent(2);
        OS << "RawRetValue[" << I << "] = " << "__builtin_ocl_" + Blt.GetName();
        OS << "(";

        // Push arguments.
        for(unsigned J = 0, F = Blt.GetParametersCount(); J != F; ++J) {
          if(J != 0)
            OS << ", ";

          // Non-gentype arguments are passed directly to callee.
          OS << "x" << J;

          // Gentype arguments are passed "element-wise".
          if(llvm::isa<OCLGenType>(&Blt.GetParameterType(AltID, J)))
            if(llvm::isa<OCLVectType>(&Blt.GetParameterType(AltID, J, SpecID)))
              OS << "[" << I << "]";
        }

        if(Blt.GetType() == OCLLibBuiltin::Relational_1_Builtin)
          OS << ") ? -1 : 0;\n";
        else
          OS << ");\n";

      }

      OS << "\n";

      // Declare a return value in vector form.
      OS.indent(2);
      EmitAutoDecl(OS, *RetTy, "RetValue");
      OS << " = {\n";

      // Emit the initializer.
      for(unsigned I = 0, E = RetTy->GetWidth(); I != E; ++I) {
        if(I != 0)
          OS << ",\n";
        OS.indent(4);
        OS << "RawRetValue[" << I << "]";
      }
      OS << "\n";

      OS.indent(2) << "};\n\n";

      // Return the vector.
      OS.indent(2) << "return RetValue;\n";
    }
  }

  OS << "}\n";
}

void EmitBuiltinTypeOpeningGuard(llvm::raw_ostream &OS, OCLLibBuiltin &Blt) {
  switch(Blt.GetType()) {
    case OCLLibBuiltin::CommonBuiltin:
      OS << "#ifdef __BUILTINS_COMMON\n\n";
      break;
    case OCLLibBuiltin::IntegerBuiltin:
      OS << "#ifdef __BUILTINS_INTEGER\n\n";
      break;
    case OCLLibBuiltin::MathBuiltin:
      OS << "#ifdef __BUILTINS_MATH\n\n";
      break;
    case OCLLibBuiltin::Relational_1_Builtin:
      OS << "#ifdef __BUILTINS_RELATIONAL\n\n";
      break;
    case OCLLibBuiltin::Relational_2_Builtin:
      OS << "#ifdef __BUILTINS_RELATIONAL\n\n";
      break;
  }
}

void EmitBuiltinTypeClosingGuard(llvm::raw_ostream &OS, OCLLibBuiltin &Blt) {
  switch(Blt.GetType()) {
    case OCLLibBuiltin::CommonBuiltin:
      OS << "#endif /* __BUILTINS_COMMON */\n\n";
      break;
    case OCLLibBuiltin::IntegerBuiltin:
      OS << "#endif /* __BUILTINS_INTEGER */\n\n";
      break;
    case OCLLibBuiltin::MathBuiltin:
      OS << "#endif /* __BUILTINS_MATH */\n\n";
      break;
    case OCLLibBuiltin::Relational_1_Builtin:
      OS << "#endif /* __BUILTINS_RELATIONAL */\n\n";
      break;
    case OCLLibBuiltin::Relational_2_Builtin:
      OS << "#endif /* __BUILTINS_RELATIONAL */\n\n";
      break;
  }
}

void EmitIncludes(llvm::raw_ostream &OS) {
  OS << "\n"
     << "#include <float.h>\n"
     << "#include <stddef.h>\n"
     << "#include <stdint.h>\n";
}

void EmitTypes(llvm::raw_ostream &OS, llvm::RecordKeeper &R) {
  // In OpenCL C unsigned types have a shortcut.
  OS << "\n"
     << "/* Built-in Scalar Data Types */\n"
     << "\n"
     << "typedef unsigned char uchar;\n"
     << "typedef unsigned short ushort;\n"
     << "typedef unsigned int uint;\n"
     << "typedef unsigned long ulong;\n";

  // Emit vectors using definitions found in the .td files.
  OCLVectTypeContainer Vectors = LoadOCLVectTypes(R);

  OS << "\n"
     << "/* Built-in Vector Data Types */\n"
     << "\n";

  for(OCLVectTypeContainer::iterator I = Vectors.begin(),
                                     E = Vectors.end();
                                     I != E;
                                     ++I) {
    OCLVectType &VectTy = **I;
    OS << "typedef __attribute__(("
       << "ext_vector_type(" << (*I)->GetWidth() << ")"
       << ")) "
       << VectTy.GetBaseType() << " " << VectTy << ";\n";
  }

  // Other types.
  OS << "\n"
     << "/* Other derived types */\n"
     << "\n"
     << "typedef ulong cl_mem_fence_flags;\n";
}

void EmitWorkItemDecls(llvm::raw_ostream &OS) {
  OS << "\n"
     << "/* Work-Item Functions */\n"
     << "\n"
     << "uint __builtin_ocl_get_work_dim();\n"
     << "size_t __builtin_ocl_get_global_size(uint dimindx);\n"
     << "size_t __builtin_ocl_get_global_id(uint dimindx);\n"
     << "size_t __builtin_ocl_get_local_size(uint dimindx);\n"
     << "size_t __builtin_ocl_get_local_id(uint dimindx);\n"
     << "size_t __builtin_ocl_get_num_groups(uint dimindx);\n"
     << "size_t __builtin_ocl_get_group_id(uint dimindx);\n"
     << "size_t __builtin_ocl_get_global_offset(uint dimindx);\n";
}

void EmitSynchronizationDecls(llvm::raw_ostream &OS) {
  OS << "\n"
     << "/* Synchronization Functions */\n"
     << "\n"
     << "void __builtin_ocl_barrier(cl_mem_fence_flags flags);\n";
}

void EmitAsyncCopyDecls(llvm::raw_ostream &OS) {
  OS << "\n"
     << "/* Asynchronous Copy & Prefetch Functions */\n"
     << "\n"
     << "void __builtin_ocl_wait_group_events(int num_events, event_t *event_list);\n"
     << "\n"
     << "#define ASYNCCOPY_PROTO(gentype) \\\n"
     << "\t__attribute__((overloadable))\\\n"
     << "\tevent_t __builtin_ocl_async_work_group_copy(__local gentype *dst, \\\n"
     << "\t\tconst __global gentype *src, \\\n"
     << "\t\tsize_t num_gentypes, \\\n"
     << "\t\tevent_t event);\\\n"
     << "\\\n"
     << "\t__attribute__((overloadable))\\\n"
     << "\tevent_t __builtin_ocl_async_work_group_copy(__global gentype *dst, \\\n"
     << "\t\tconst __local gentype *src, \\\n"
     << "\t\tsize_t num_gentypes, \\\n"
     << "\t\tevent_t event);\n"
     << "\n"
     << "#define ASYNCSTRIDEDCOPY_PROTO(gentype) \\\n"
     << "\t__attribute__((overloadable))\\\n"
     << "\tevent_t __builtin_ocl_async_work_group_strided_copy(__local gentype *dst, \\\n"
     << "\t\tconst __global gentype *src, \\\n"
     << "\t\tsize_t num_gentypes, \\\n"
     << "\t\tsize_t stride, \\\n"
     << "\t\tevent_t event);\\\n"
     << "\\\n"
     << "\t__attribute__((overloadable))\\\n"
     << "\tevent_t __builtin_ocl_async_work_group_strided_copy(__global gentype *dst, \\\n"
     << "\t\tconst __local gentype *src, \\\n"
     << "\t\tsize_t num_gentypes, \\\n"
     << "\t\tsize_t stride, \\\n"
     << "\t\tevent_t event);\\\n"
     << "\n"
     << "#define PREFETCH_PROTO(gentype) \\\n"
     << "\t__attribute__((overloadable))\\\n"
     << "\tvoid __builtin_ocl_prefetch(const __global gentype *p, size_t num_gentypes);\n"
     << "\n"
     << "#define BUILD_ASYNC_LIB_PROTO(gentype) \\\n"
     << "\tASYNCCOPY_PROTO(gentype) \\\n"
     << "\tASYNCSTRIDEDCOPY_PROTO(gentype) \\\n"
     << "\tPREFETCH_PROTO(gentype)\n"
     << "\n\n"
	 << "BUILD_ASYNC_LIB_PROTO(char)\n"
	 << "BUILD_ASYNC_LIB_PROTO(char2)\n"
	 << "BUILD_ASYNC_LIB_PROTO(char3)\n"
	 << "BUILD_ASYNC_LIB_PROTO(char4)\n"
	 << "BUILD_ASYNC_LIB_PROTO(char8)\n"
	 << "BUILD_ASYNC_LIB_PROTO(char16)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uchar)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uchar2)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uchar3)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uchar4)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uchar8)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uchar16)\n"
	 << "BUILD_ASYNC_LIB_PROTO(short)\n"
	 << "BUILD_ASYNC_LIB_PROTO(short2)\n"
	 << "BUILD_ASYNC_LIB_PROTO(short3)\n"
	 << "BUILD_ASYNC_LIB_PROTO(short4)\n"
	 << "BUILD_ASYNC_LIB_PROTO(short8)\n"
	 << "BUILD_ASYNC_LIB_PROTO(short16)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ushort)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ushort2)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ushort3)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ushort4)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ushort8)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ushort16)\n"
	 << "BUILD_ASYNC_LIB_PROTO(int)\n"
	 << "BUILD_ASYNC_LIB_PROTO(int2)\n"
	 << "BUILD_ASYNC_LIB_PROTO(int3)\n"
	 << "BUILD_ASYNC_LIB_PROTO(int4)\n"
	 << "BUILD_ASYNC_LIB_PROTO(int8)\n"
	 << "BUILD_ASYNC_LIB_PROTO(int16)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uint)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uint2)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uint3)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uint4)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uint8)\n"
	 << "BUILD_ASYNC_LIB_PROTO(uint16)\n"
	 << "BUILD_ASYNC_LIB_PROTO(long)\n"
	 << "BUILD_ASYNC_LIB_PROTO(long2)\n"
	 << "BUILD_ASYNC_LIB_PROTO(long3)\n"
	 << "BUILD_ASYNC_LIB_PROTO(long4)\n"
	 << "BUILD_ASYNC_LIB_PROTO(long8)\n"
	 << "BUILD_ASYNC_LIB_PROTO(long16)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ulong)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ulong2)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ulong3)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ulong4)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ulong8)\n"
	 << "BUILD_ASYNC_LIB_PROTO(ulong16)\n"
	 << "BUILD_ASYNC_LIB_PROTO(float)\n"
	 << "BUILD_ASYNC_LIB_PROTO(float2)\n"
	 << "BUILD_ASYNC_LIB_PROTO(float3)\n"
	 << "BUILD_ASYNC_LIB_PROTO(float4)\n"
	 << "BUILD_ASYNC_LIB_PROTO(float8)\n"
	 << "BUILD_ASYNC_LIB_PROTO(float16)\n"
     << "\n";
}

void EmitGeometricDecls(llvm::raw_ostream &OS) {
  OS << "\n"
     << "/* Geometric Functions */\n"
     << "\n"
     << "__attribute__((overloadable,pure))\n"
     << "float4 __builtin_ocl_cross(float4 p0, float4 p1);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float3 __builtin_ocl_cross(float3 p0, float3 p1);\n"
     << "\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_dot(float p0, float p1);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_dot(float2 p0, float2 p1);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_dot(float3 p0, float3 p1);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_dot(float4 p0, float4 p1);\n"
     << "\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_length(float p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_length(float2 p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_length(float3 p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_length(float4 p);\n"
     << "\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_distance(float p0, float p1);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_distance(float2 p0, float2 p1);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_distance(float3 p0, float3 p1);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_distance(float4 p0, float4 p1);\n"
     << "\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_normalize(float p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float2 __builtin_ocl_normalize(float2 p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float3 __builtin_ocl_normalize(float3 p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float4 __builtin_ocl_normalize(float4 p);\n"
     << "\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_fast_length(float p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_fast_length(float2 p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_fast_length(float3 p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_fast_length(float4 p);\n"
     << "\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_fast_distance(float p0, float p1);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_fast_distance(float2 p0, float2 p1);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_fast_distance(float3 p0, float3 p1);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_fast_distance(float4 p0, float4 p1);\n"
     << "\n"
     << "__attribute__((overloadable,pure))\n"
     << "float __builtin_ocl_fast_normalize(float p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float2 __builtin_ocl_fast_normalize(float2 p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float3 __builtin_ocl_fast_normalize(float3 p);\n"
     << "__attribute__((overloadable,pure))\n"
     << "float4 __builtin_ocl_fast_normalize(float4 p);";
}

void EmitMacros(llvm::raw_ostream &OS) {
  // Math macros.
  OS << "\n"
     << "/*\n"
     << " * These macros are defined in math.h, but because we\n"
     << " * cannot include it, define them here. Definitions picked-up\n"
     << " * from GNU math.h.\n"
     << " */\n"
     << "\n"
     << "#define M_E            2.71828182845904523540f  /* e          */\n"
     << "#define M_LOG2E        1.44269504088896340740f  /* log_2 e    */\n"
     << "#define M_LOG10E       0.43429448190325182765f  /* log_10 e   */\n"
     << "#define M_LN2          0.69314718055994530942f  /* log_e 2    */\n"
     << "#define M_LN10         2.30258509299404568402f  /* log_e 10   */\n"
     << "#define M_PI           3.14159265358979323846f  /* pi         */\n"
     << "#define M_PI_2         1.57079632679489661923f  /* pi/2       */\n"
     << "#define M_PI_4         0.78539816339744830962f  /* pi/4       */\n"
     << "#define M_1_PI         0.31830988618379067154f  /* 1/pi       */\n"
     << "#define M_2_PI         0.63661977236758134308f  /* 2/pi       */\n"
     << "#define M_2_SQRTPI     1.12837916709551257390f  /* 2/sqrt(pi) */\n"
     << "#define M_SQRT2        1.41421356237309504880f  /* sqrt(2)    */\n"
     << "#define M_SQRT1_2      0.70710678118654752440f  /* 1/sqrt(2)  */\n";

  // Synchronization macros.
  OS << "\n"
     << "/* Synchronization Macros */\n"
     << "#define CLK_LOCAL_MEM_FENCE  0\n"
     << "#define CLK_GLOBAL_MEM_FENCE 1\n";
}

void EmitWorkItemRewritingMacros(llvm::raw_ostream &OS) {
  OS << "\n"
     << "/* Work-Item Rewriting Macros */\n"
     << "\n"
     << "#define get_work_dim __builtin_ocl_get_work_dim\n"
     << "#define get_global_size __builtin_ocl_get_global_size\n"
     << "#define get_global_id __builtin_ocl_get_global_id\n"
     << "#define get_local_size __builtin_ocl_get_local_size\n"
     << "#define get_local_id __builtin_ocl_get_local_id\n"
     << "#define get_num_groups __builtin_ocl_get_num_groups\n"
     << "#define get_group_id __builtin_ocl_get_group_id\n"
     << "#define get_global_offset __builtin_ocl_get_global_offset\n";
}

void EmitSynchronizationRewritingMacros(llvm::raw_ostream &OS) {
  OS << "\n"
     << "/* Synchronization Rewriting Macros */\n"
     << "\n"
     << "#define barrier __builtin_ocl_barrier\n";
}

void EmitAsyncCopyRewritingMacros(llvm::raw_ostream &OS) {
  OS << "\n"
     << "/* Async Copy and Prefetch Rewriting Macros */\n"
     << "\n"
     << "#define wait_group_events __builtin_ocl_wait_group_events\n"
     << "#define async_work_group_copy __builtin_ocl_async_work_group_copy\n"
     << "#define async_work_group_strided_copy __builtin_ocl_async_work_group_strided_copy\n"
     << "#define wait_group_events __builtin_ocl_wait_group_events\n"
     << "#define prefetch __builtin_ocl_prefetch\n";
}

void EmitGeometricRewritingMacros(llvm::raw_ostream &OS) {
  OS << "\n"
     << "/* Geometric Rewriting Macros */\n"
     << "\n"
     << "#define cross __builtin_ocl_cross\n"
     << "#define dot __builtin_ocl_dot\n"
     << "#define length __builtin_ocl_length\n"
     << "#define distance __builtin_ocl_distance\n"
     << "#define normalize __builtin_ocl_normalize\n"
     << "#define fast_length __builtin_ocl_fast_length\n"
     << "#define fast_distance __builtin_ocl_fast_distance\n"
     << "#define fast_normalize __builtin_ocl_fast_normalize\n";
}

void EmitBuiltinRewritingMacro(llvm::raw_ostream &OS, OCLLibBuiltin &Blt) {
  OS << "#define " << Blt.GetName() << " "
                   << "__builtin_ocl_" << Blt.GetName()
                   << "\n";
}

void EmitAutoDecl(llvm::raw_ostream &OS, OCLType &Ty, llvm::StringRef Name) {
  OS << Ty << " " << Name;
}

void EmitAutoArrayDecl(llvm::raw_ostream &OS,
                       OCLScalarType &BaseTy,
                       int64_t N,
                       llvm::StringRef Name) {
  OS << BaseTy << " " << Name << "[" << N << "]";
}

} // End anonymous namespace.
