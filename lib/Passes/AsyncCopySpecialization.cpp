
#include "opencrun/Passes/AllPasses.h"
#include "opencrun/Passes/AsyncCopySpecialization.h"
#include "opencrun/Util/OpenCLMetadataHandler.h"

#define DEBUG_TYPE "asynccopy-specialization"

#include "llvm/IR/Instructions.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/InstIterator.h"

#include <string>

using namespace opencrun;

bool AsyncCopySpecialization::runOnFunction(llvm::Function &Fun) {

  OpenCLMetadataHandler MDHandler(*Fun.getParent());

  // When we specialize a CallInst inside a kernel Function object the
  // kernel is modified.
  bool SpecDone = false;

  if(MDHandler.IsKernel(Fun) && (Kernel == Fun.getName() || Kernel == "")) {

    for(llvm::Function::iterator BI = Fun.begin(), BE = Fun.end(); BI != BE; ++BI) {
      for(llvm::BasicBlock::iterator I = BI->begin(), E = BI->end(); I != E; ++I) {
        // We search for function calls to asynchronous copy 
        // and prefetch functions.
        if(llvm::CallInst *Call = llvm::dyn_cast<llvm::CallInst>(&*I)) {
          llvm::Function *CalledFun = Call->getCalledFunction();

          // The only allowed functions to call are external functions -- 
          // typically internal calls/runtime builtins.
          if(!CalledFun || !CalledFun->hasExternalLinkage())
            return false;
          
          llvm::StringRef FunName = CalledFun->getName();
          
          // Only async copy and prefetch functions must be specialized.
          if(FunName.count(llvm::StringRef("__builtin_ocl_async_work_group_")) ||
             FunName.count(llvm::StringRef("__builtin_ocl_prefetch"))) {

            std::string BuiltinName;
            llvm::Function *BuiltinFun;

            llvm::Type *dst_type = NULL, *src_type = NULL;

            if(FunName.count(llvm::StringRef("__builtin_ocl_async_work_group_copy"))) {

              BuiltinName = "async_work_group_copy_";

              dst_type = (Call->getArgOperand(0))->getType(); // 1st argument
              src_type = (Call->getArgOperand(1))->getType(); // 2nd argument
              
              // dst and src types must be the same and they must be both PointerType.
              if((dst_type != src_type) && !(dst_type->isPointerTy()) && !(src_type->isPointerTy()))
                return false;

            } else if(FunName.count(llvm::StringRef("__builtin_ocl_async_work_group_strided_copy"))) {

              BuiltinName = "async_work_group_strided_copy_";

              dst_type = (Call->getArgOperand(0))->getType(); // 1st argument
              src_type = (Call->getArgOperand(1))->getType(); // 2nd argument

              // dst and src types must be the same and they must be both PointerType.
              if((dst_type != src_type) && !(dst_type->isPointerTy()) && !(src_type->isPointerTy()))
                return false;

            } else if(FunName.count(llvm::StringRef("__builtin_ocl_prefetch"))) {

              BuiltinName = "prefetch_";

              dst_type = (Call->getArgOperand(0))->getType(); // 1st argument
              
              if(!(dst_type->isPointerTy()))
                return false;

            }
                      
            // We know here that dst_type and src_type are pointer types so we use a
            // "checked" cast.
            llvm::PointerType *ptr_type = llvm::cast<llvm::PointerType>(&*dst_type);
            llvm::Type *ptd_type = ptr_type->getElementType();
            
            if(ptd_type->isIntegerTy()) {
              // dst and src are pointers to integer types.
              if(ptd_type->isIntegerTy(8)) {
                // CL: char , LLVM: i8
                BuiltinName += "char";
              } else if(ptd_type->isIntegerTy(16)) {
                // CL: short , LLVM: i16
                BuiltinName += "short";
              } else if(ptd_type->isIntegerTy(32)) {
                // CL: int , LLVM: i32
                BuiltinName += "int";
              } else if(ptd_type->isIntegerTy(64)) {
                // CL: long, LLVM: i64
                BuiltinName += "long";
              }
            } else if(ptd_type->isFloatingPointTy()) {
              // dst and src are pointers to floating point types.
              if(ptd_type->isFloatTy()) {
                // CL: float, LLVM: float
                BuiltinName += "float";
              }
            } else if(ptd_type->isArrayTy() || ptd_type->isVectorTy()) {

              uint64_t el_sz;
              llvm::Type *el_ty;

              if(ptd_type->isArrayTy()) {
                // dst and src are pointers to array types.
                llvm::ArrayType *array_type = llvm::cast<llvm::ArrayType>(&*ptd_type);
  
                // Store array size and element type.
                el_sz = array_type->getNumElements();
                el_ty = array_type->getElementType();
              } else if(ptd_type->isVectorTy()) {
                // dst and src are pointers to array types.
                llvm::VectorType *vector_type = llvm::cast<llvm::VectorType>(&*ptd_type);
  
                // Store array size and element type.
                el_sz = vector_type->getNumElements();
                el_ty = vector_type->getElementType();
              }

              if(el_sz == 2) {
                if(el_ty->isIntegerTy()) {
                  if(el_ty->isIntegerTy(8)) {
                    // CL: char2 , LLVM: [2 x i8]
                    BuiltinName += "char2";
                  } else if(el_ty->isIntegerTy(16)) {
                    // CL: short2 , LLVM: [2 x i16]
                    BuiltinName += "short2";
                  } else if(el_ty->isIntegerTy(32)) {
                    // CL: int2 , LLVM: [2 x i32]
                    BuiltinName += "int2";
                  } else if(el_ty->isIntegerTy(64)) {
                    // CL: long2 , LLVM: [2 x i64]
                    BuiltinName += "long2";
                  }
                } else if(el_ty->isFloatingPointTy()) {
                  if(el_ty->isFloatTy()) {
                    // CL: float2 , LLVM: [2 x float]
                    BuiltinName += "float2";
                  }
                }
              } else if(el_sz == 3) {
                if(el_ty->isIntegerTy()) {
                  if(el_ty->isIntegerTy(8)) {
                    // CL: char3 , LLVM: [3 x i8]
                    BuiltinName += "char3";
                  } else if(el_ty->isIntegerTy(16)) {
                    // CL: short3 , LLVM: [3 x i16]
                    BuiltinName += "short3";
                  } else if(el_ty->isIntegerTy(32)) {
                    // CL: int3 , LLVM: [3 x i32]
                    BuiltinName += "int3";
                  } else if(el_ty->isIntegerTy(64)) {
                    // CL: long3 , LLVM: [3 x i64]
                    BuiltinName += "long3";
                  }
                } else if(el_ty->isFloatingPointTy()) {
                  if(el_ty->isFloatTy()) {
                    // CL: float3 , LLVM: [3 x float]
                    BuiltinName += "float3";
                  }
                }
              } else if(el_sz == 4) {
                if(el_ty->isIntegerTy()) {
                  if(el_ty->isIntegerTy(8)) {
                    // CL: char4 , LLVM: [4 x i8]
                    BuiltinName += "char4";
                  } else if(el_ty->isIntegerTy(16)) {
                    // CL: short4 , LLVM: [4 x i16]
                    BuiltinName += "short4";
                  } else if(el_ty->isIntegerTy(32)) {
                    // CL: int4 , LLVM: [4 x i32]
                    BuiltinName += "int4";
                  } else if(el_ty->isIntegerTy(64)) {
                    // CL: long4 , LLVM: [4 x i64]
                    BuiltinName += "long4";
                  }
                } else if(el_ty->isFloatingPointTy()) {
                  if(el_ty->isFloatTy()) {
                    // CL: float4 , LLVM: [4 x float]
                    BuiltinName += "float4";
                  }
                }
              } else if(el_sz == 8) {
                if(el_ty->isIntegerTy()) {
                  if(el_ty->isIntegerTy(8)) {
                    // CL: char8 , LLVM: [8 x i8]
                    BuiltinName += "char8";
                  } else if(el_ty->isIntegerTy(16)) {
                    // CL: short8 , LLVM: [8 x i16]
                    BuiltinName += "short8";
                  } else if(el_ty->isIntegerTy(32)) {
                    // CL: int8 , LLVM: [8 x i32]
                    BuiltinName += "int8";
                  } else if(el_ty->isIntegerTy(64)) {
                    // CL: long8 , LLVM: [8 x i64]
                    BuiltinName += "long8";
                  }
                } else if(el_ty->isFloatingPointTy()) {
                  if(el_ty->isFloatTy()) {
                    // CL: float8 , LLVM: [8 x float]
                    BuiltinName += "float8";
                  }
                }
              } else if(el_sz == 16) {
                if(el_ty->isIntegerTy()) {
                  if(el_ty->isIntegerTy(8)) {
                    // CL: char16 , LLVM: [16 x i8]
                    BuiltinName += "char16";
                  } else if(el_ty->isIntegerTy(16)) {
                    // CL: short16 , LLVM: [16 x i16]
                    BuiltinName += "short16";
                  } else if(el_ty->isIntegerTy(32)) {
                    // CL: int16 , LLVM: [16 x i32]
                    BuiltinName += "int16";
                  } else if(el_ty->isIntegerTy(64)) {
                    // CL: long16 , LLVM: [16 x i64]
                    BuiltinName += "long16";
                  }
                } else if(el_ty->isFloatingPointTy()) {
                  if(el_ty->isFloatTy()) {
                    // CL: float16 , LLVM: [16 x float]
                    BuiltinName += "float16";
                  }
                }
              }
            }
            
            BuiltinFun = MDHandler.GetBuiltin(llvm::StringRef(BuiltinName));
            Call->setCalledFunction(BuiltinFun);
            SpecDone = true;
          
          }

        }
      }
    }

  }

  return SpecDone;

}

char AsyncCopySpecialization::ID = 0;

AsyncCopySpecialization *
opencrun::CreateAsyncCopySpecializationPass(llvm::StringRef Kernel) {
  return new AsyncCopySpecialization(Kernel);
}

using namespace llvm;

INITIALIZE_PASS_BEGIN(AsyncCopySpecialization,
                      "asynccopy-specialization",
                      "Asynchronous copy and prefetch function specialization",
                      false,
                      false)
INITIALIZE_PASS_END(AsyncCopySpecialization,
                    "asynccopy-specialization",
                    "Asynchronous copy and prefetch function specialization",
                    false,
                    false)
