#include "OCLTypeDefEmitter.h"
#include "OCLEmitterUtils.h"
#include "OCLType.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Casting.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace opencrun;

namespace {

OCLTypesContainer OCLTypes;

}

const char *BasicIntegerTy(unsigned BitWidth) {
  // Here we assume that the frontend enforce correct OpenCL bitwidth to 
  // builtin types.
  switch(BitWidth) {
    default: break;
    case 8: return "char";
    case 16: return "short";
    case 32: return "int";
    case 64: return "long";
  }
  llvm::PrintFatalError("Unsupported integer type!");
}

void EmitOCLScalarTypes(llvm::raw_ostream &OS) {
  for (unsigned i = 0, e = OCLTypes.size(); i != e; ++i) {
    if (const OCLIntegerType *I = llvm::dyn_cast<OCLIntegerType>(OCLTypes[i])) {
      if (I->isUnsigned()) {
        OS << "typedef unsigned " << BasicIntegerTy(I->getBitWidth()) 
           << " " << I->getName() << ";\n";
      }
    }
  }
}

void EmitOCLVectorTypes(llvm::raw_ostream &OS) {
  llvm::BitVector GroupPreds;
  for (unsigned i = 0, e = OCLTypes.size(); i != e; ++i) {
    if (const OCLVectorType *V = llvm::dyn_cast<OCLVectorType>(OCLTypes[i])) {
      const llvm::BitVector &Preds = V->getBaseType().getPredicates();
      if (Preds != GroupPreds) {
        EmitPredicatesEnd(OS, GroupPreds);
        OS << "\n";
        GroupPreds = Preds;
        EmitPredicatesBegin(OS, GroupPreds); 
      }

      OS << "typedef " << V->getBaseType().getName()
         << " __attribute__((ext_vector_type(" << V->getWidth() << "))) "
         << V->getName() << ";\n";
    }
  }
  EmitPredicatesEnd(OS, GroupPreds);
}

bool opencrun::EmitOCLTypeDef(llvm::raw_ostream &OS, 
                              llvm::RecordKeeper &R) {
  LoadOCLTypes(R, OCLTypes);

  emitSourceFileHeader("OCL Type definitions", OS);

  OS << "#ifndef OPENCRUN_OCLTYPE_H\n";
  OS << "#define OPENCRUN_OCLTYPE_H\n\n";

  OS << "#include <stddef.h>\n\n";

  OS << "\n"
     << "/*\n"
     << " * These macros are defined in math.h, but because we\n"
     << " * cannot include it, define them here. Definitions picked-up\n"
     << " * from GNU math.h.\n"
     << " */\n"
     << "\n"
     << "#define M_EF           __builtin_expf(1.0f)        /* e          */\n"
     << "#define M_LOG2EF       __builtin_log2f(M_EF)       /* log_2 e    */\n"
     << "#define M_LOG10EF      __builtin_log10f(M_EF)      /* log_10 e   */\n"
     << "#define M_LN2F         __builtin_logf(2.0f)        /* log_e 2    */\n"
     << "#define M_LN10F        __builtin_logf(10.0f)       /* log_e 10   */\n"
     << "#define M_PIF          3.14159265358979323846f     /* pi         */\n"
     << "#define M_PI_2F        M_PIF/2.0f                  /* pi/2       */\n"
     << "#define M_PI_4F        M_PIF/4.0f                  /* pi/4       */\n"
     << "#define M_1_PIF        1.0f/M_PIF                  /* 1/pi       */\n"
     << "#define M_2_PIF        2.0f/M_PIF                  /* 2/pi       */\n"
     << "#define M_2_SQRTPIF    2.0f/__builtin_sqrtf(M_PIF) /* 2/sqrt(pi) */\n"
     << "#define M_SQRT2F       __builtin_sqrtf(2.0f)       /* sqrt(2)    */\n"
     << "#define M_SQRT1_2F     1.0f/__builtin_sqrtf(2.0f)  /* 1/sqrt(2)  */\n"
     << "#define LOG_PIF        __builtin_logf(M_PIF)       /* log_e pi   */\n"
     << ""
     << "#define M_E            __builtin_exp(1.0)                          /* e          */\n"
     << "#define M_LOG2E        __builtin_log2(M_E)                         /* log_2 e    */\n"
     << "#define M_LOG10E       __builtin_log10(M_E)                        /* log_10 e   */\n"
     << "#define M_LN2          __builtin_log(2.0)                          /* log_e 2    */\n"
     << "#define M_LN10         __builtin_log(10.0)                         /* log_e 10   */\n"
     << "#define M_PI           3.141592653589793238462643383279502884      /* pi         */\n"
     << "#define M_PI_2         M_PI/2.0                                    /* pi/2       */\n"
     << "#define M_PI_4         M_PI/4.0                                    /* pi/4       */\n"
     << "#define M_1_PI         1.0/M_PI                                    /* 1/pi       */\n"
     << "#define M_2_PI         2.0/M_PI                                    /* 2/pi       */\n"
     << "#define M_2_SQRTPI     2.0/__builtin_sqrt(M_PI)                    /* 2/sqrt(pi) */\n"
     << "#define M_SQRT2        __builtin_sqrt(2.0)                         /* sqrt(2)    */\n"
     << "#define M_SQRT1_2      1.0/__builtin_sqrt(2.0)                     /* 1/sqrt(2)  */\n"
     << "#define LOG_PI         __builtin_log(M_PI)                         /* log_e pi   */\n" 
     << "\n"
     << "#define FLT_DIG        6\n"
     << "#define FLT_MANT_DIG   24\n"
     << "#define FLT_MAX_10_EXP +38\n"
     << "#define FLT_MAX_EXP    +128\n"
     << "#define FLT_MIN_10_EXP -37\n"
     << "#define FLT_MIN_EXP    -125\n"
     << "#define FLT_RADIX      2\n"
     << "#define FLT_MAX        0x1.fffffep127f\n"
     << "#define FLT_MIN        0x1.0p-126f\n"
     << "#define FLT_EPSILON    0x1.0p-23f\n"
     << "\n"
     << "#define DBL_DIG        15\n"
     << "#define DBL_MANT_DIG   53\n"
     << "#define DBL_MAX_10_EXP +308\n"
     << "#define DBL_MAX_EXP    +1024\n"
     << "#define DBL_MIN_10_EXP -307\n"
     << "#define DBL_MIN_EXP    -1021\n"
     << "#define DBL_MAX        0x1.fffffffffffffp1023\n"
     << "#define DBL_MIN        0x1.0p-1022\n"
     << "#define DBL_EPSILON    0x1.0p-52\n"
     << "\n"
     << "#define CHAR_BIT       8\n"
     << "#define CHAR_MAX       SCHAR_MAX\n"
     << "#define CHAR_MIN       SCHAR_MIN\n"
     << "#define INT_MAX        2147483647\n"
     << "#define INT_MIN        (-2147483647 - 1)\n"
     << "#define LONG_MAX       0x7fffffffffffffffL\n"
     << "#define LONG_MIN       (-0x7fffffffffffffffL - 1)\n"
     << "#define SCHAR_MAX      127\n"
     << "#define SCHAR_MIN      (-127 - 1)\n"
     << "#define SHRT_MAX       32767\n"
     << "#define SHRT_MIN       (-32767 - 1)\n"
     << "#define UCHAR_MAX      255\n"
     << "#define USHRT_MAX      65535\n"
     << "#define UINT_MAX       0xffffffff\n"
     << "#define ULONG_MAX      0xffffffffffffffffUL\n"
     << "\n"
     << "/* Get machine-dependent HUGE_VAL value (returned on overflow).\n"
     << " * On all IEEE754 machines, this is +Infinity.\n"
     << " */\n"
     << "\n"
     << "# define HUGE_VAL	(__builtin_huge_val())\n"
     << "# define HUGE_VALF	(__builtin_huge_valf())\n"
     << "\n\n";
  

  OS << "// Scalar types\n";
  EmitOCLScalarTypes(OS);
  OS << "\n";

  OS <<"// Vector types\n";
  EmitOCLVectorTypes(OS);

  OS << "\n#endif\n";
  
  return false;
}
