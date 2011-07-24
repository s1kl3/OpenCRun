
#include "OCLLibEmitter.h"

#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

//
// OCLLibEmitter implementation.
//

void OCLLibEmitter::EmitHeader(raw_ostream &OS, OCLLibBuiltin &Blt, unsigned SpecID) {
  // Emit attributes.
  OS << "__attribute__((";
  for(unsigned I = 0, E = Blt.GetAttributesCount(); I != E; ++I) {
    if(I != 0)
      OS << ",";
    OS << Blt.GetAttribute(I);
  }
  OS << "))\n";

  // Emit return type.
  OS << Blt.GetReturnType(SpecID) << " ";

  // Emit name.
  OS << Blt.GetName();

  // Emit arguments.
  OS << "(";
  for(unsigned I = 0, E = Blt.GetParametersCount(); I != E; ++I) {
    if(I != 0)
      OS << ", ";
    OS << Blt.GetParameterType(I, SpecID) << " x" << I;
  }
  OS << ")";
}

//
// OCLLibImplEmitter implementation.
//

void OCLLibImplEmitter::run(raw_ostream &OS) {
  OCLBuiltinContainer Blts = LoadOCLBuiltins(Records);

  EmitSourceFileHeader("OpenCL C generic library implementation.", OS);

  for(OCLBuiltinContainer::iterator I = Blts.begin(),
                                    E = Blts.end();
                                    I != E;
                                    ++I)
    for(unsigned J = 0, F = I->GetSpecializationsCount(); J != F; ++J) {
      if(J != 0)
        OS << "\n";
      EmitImplementation(OS, *I, J);
    }
}

void OCLLibImplEmitter::EmitImplementation(raw_ostream &OS,
                                           OCLLibBuiltin &Blt,
                                           unsigned SpecID) {
  EmitHeader(OS, Blt, SpecID);

  OS << " {\n";

  // Base specialization has ID 0: emit user-given code.
  if(SpecID == 0)
    OS << "  " << Blt.GetBaseImplementation() << "\n";

  // Recursive case, call base.
  else {
    OCLVectType *RetTy;
    RetTy = dynamic_cast<OCLVectType *>(&Blt.GetReturnType(SpecID));
    if(!RetTy)
      llvm_unreachable("Not yet implemented");

    // This variable holds return value in non-vector form.
    OS.indent(2);
    EmitArrayDecl(OS, RetTy->GetBaseType(), RetTy->GetWidth(), "RawRetValue");
    OS << ";\n\n";

    // Call base implementation for each vector element.
    for(unsigned I = 0, E = RetTy->GetWidth(); I != E; ++I) {
      // The i-th call writes the i-th element of the return value.
      OS.indent(2);
      OS << "RawRetValue[" << I << "] = " << Blt.GetName();
      OS << "(";

      // Push arguments.
      for(unsigned J = 0, F = Blt.GetParametersCount(); J != F; ++J) {
        if(J != 0)
          OS << ", ";

        // Non-gentype arguments are passed directly to callee.
        OS << "x" << J;

        // Gentype arguments are passed "element-wise".
        if(dynamic_cast<OCLGenType *>(&Blt.GetParameterType(J)))
          OS << "[" << I << "]";
      }

      OS << ");\n";
    }

    OS << "\n";

    // Declare a return value in vector form.
    OS.indent(2);
    EmitDecl(OS, *RetTy, "RetValue");
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

  OS << "}\n";
}

void OCLLibImplEmitter::EmitDecl(raw_ostream &OS,
                                 OCLType &Ty,
                                 llvm::StringRef Name) {
  OS << Ty << " " << Name;
}

void OCLLibImplEmitter::EmitArrayDecl(raw_ostream &OS,
                                      OCLScalarType &BaseTy,
                                      int64_t N,
                                      llvm::StringRef Name) {
  OS << BaseTy << " " << Name << "[" << N << "]";
}

//
// OCLDefEmitter implementation.
//

void OCLDefEmitter::run(raw_ostream &OS) {
  OCLBuiltinContainer Blts = LoadOCLBuiltins(Records);

  EmitSourceFileHeader("OpenCL C library definitions.", OS);

  // Emit opening guard.
  OS << "#ifndef __CLANG_OCLDEF_H\n"
     << "#define __CLANG_OCLDEF_H\n";

  EmitIncludes(OS);

  // Emit C++ opening guard.
  OS << "\n"
     << "#ifdef __cplusplus\n"
     << "extern \"C\" {\n"
     << "#endif\n";

  EmitTypes(OS);
  EmitWorkItemDecls(OS);
  EmitSynchronizationDecls(OS);

  EmitMacros(OS);

  // Emit generic definitions.
  OS << "\n/* BEGIN AUTO-GENERATED PROTOTYPES */\n";
  for(OCLBuiltinContainer::iterator I = Blts.begin(),
                                    E = Blts.end();
                                    I != E;
                                    ++I) {
    for(unsigned J = 0, F = I->GetSpecializationsCount(); J != F; ++J) {
        OS << "\n";
      EmitHeader(OS, *I, J);
      OS << ";";
    }
    OS << "\n";
  }
  OS << "\n/* END AUTO-GENERATED PROTOTYPES */\n";

  // Emit C++ closing guard.
  OS << "\n"
     << "#ifdef __cplusplus\n"
     << "}\n"
     << "#endif\n";

  // Emit closing guard.
  OS << "\n#endif /* __CLANG_OCLDEF_H */\n";
}

void OCLDefEmitter::EmitIncludes(raw_ostream &OS) {
  OS << "\n"
     << "#include <float.h>\n"
     << "#include <stddef.h>\n"
     << "#include <stdint.h>\n";
}

void OCLDefEmitter::EmitTypes(raw_ostream &OS) {
  // In OpenCL C unsigned types have a shortcut.
  OS << "\n"
     << "/* Built-in Scalar Data Types */\n"
     << "\n"
     << "typedef unsigned char uchar;\n"
     << "typedef unsigned short ushort;\n"
     << "typedef unsigned int uint;\n"
     << "typedef unsigned long ulong;\n";

  // Emit vectors using definitions found in the .td files.
  OCLVectTypeContainer Vectors = LoadOCLVectTypes(Records);

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

void OCLDefEmitter::EmitWorkItemDecls(raw_ostream &OS) {
  OS << "\n"
     << "/* Work-Item Functions */\n"
     << "\n"
     << "uint get_work_dim();\n"
     << "size_t get_global_size(uint dimindx);\n"
     << "size_t get_global_id(uint dimindx);\n"
     << "size_t get_local_size(uint dimindx);\n"
     << "size_t get_local_id(uint dimindx);\n"
     << "size_t get_num_groups(uint dimindx);\n"
     << "size_t get_group_id(uint dimindx);\n"
     << "size_t get_global_offset(uint dimindx);\n";
}

void OCLDefEmitter::EmitSynchronizationDecls(raw_ostream &OS) {
  OS << "\n"
     << "/* Synchronization Functions */\n"
     << "\n"
     << "void barrier(cl_mem_fence_flags flags);\n";
}

void OCLDefEmitter::EmitMacros(raw_ostream &OS) {
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
     << "/* Synchronization Macros*/\n"
     << "#define CLK_LOCAL_MEM_FENCE  0\n"
     << "#define CLK_GLOBAL_MEM_FENCE 1\n";
}
