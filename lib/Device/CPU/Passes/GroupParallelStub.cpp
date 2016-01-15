
#include "CPUPasses.h"

#include "opencrun/Util/BuiltinInfo.h"
#include "opencrun/Util/ModuleInfo.h"
#include "opencrun/Util/PassOptions.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

#define DEBUG_TYPE "CPU-group-parallel-stub"

using namespace llvm;
using namespace opencrun;

STATISTIC(GroupParallelStubs, "Number of CPU group parallel stubs created");

namespace {

//===----------------------------------------------------------------------===//
/// GroupParallelStub - Builds stubs to invoke OpenCL kernels from inside the
///  CPU runtime. The CPU runtime stores kernel arguments inside an array of
///  pointers. To correctly invoke the kernel, it is needed to unpack arguments
///  from the array. Each array element is a pointer. If the i-th kernel
///  parameter is a buffer, then the pointer directly points to it. If it is a
///  a value type, then it points to the value supplied by the user -- it must
///  be passed by copy to the kernel.
///
///  Example for void foo(int a, global int *b):
///
///  args: [ int * ][ int *]
///          |        |
///          |        +-> b: [ int ] ... [ int ]
///          |
///          +-> a: [ int ]
//===----------------------------------------------------------------------===//
class GroupParallelStub : public ModulePass {
public:
  static char ID;

public:
  GroupParallelStub(StringRef Kernel = "")
   : ModulePass(ID), Kernel(opencrun::GetKernelOption(Kernel)) {}

public:
  bool runOnModule(Module &Mod) override;

  const char *getPassName() const override {
    return "Group parallel stub builder";
  }

private:
  bool BuildStub(KernelInfo KI);

private:
  std::string Kernel;
  Function *Barrier;
};

char GroupParallelStub::ID = 0;

RegisterPass<GroupParallelStub> X(
  "cpu-group-parallel-stub",
  "Create kernel stub for cpu group parallel scheduler"
);

}

bool GroupParallelStub::runOnModule(Module &Mod) {
  ModuleInfo Info(Mod);

  Barrier = DeviceBuiltinInfo::getPrototype(Mod, "__internal_barrier", "vi");

  if (!Kernel.empty()) {
    auto I = Info.find(Kernel);
    if (I != Info.end())
      return BuildStub(*I);

    return false;
  }

  bool Modified = false;

  for (const auto &KI : Info)
    Modified |= BuildStub(KI);

  return Modified;
}

bool GroupParallelStub::BuildStub(KernelInfo KI) {
  Function *Kern = KI.getFunction();
  Module *Mod = Kern->getParent();
  
  if (!Mod)
    return false;

  LLVMContext &Ctx = Mod->getContext();

  // Make function type.
  Type *RetTy = Type::getVoidTy(Ctx);
  Type *Int8PtrTy = Type::getInt8PtrTy(Ctx);
  Type *ArgTy = PointerType::getUnqual(Int8PtrTy);
  FunctionType *StubTy = FunctionType::get(RetTy, ArgTy, false);

  // Create the stub.
  auto StubName = Kern->getName() + ".stub";
  auto *Stub = Function::Create(StubTy, Function::ExternalLinkage,
                                StubName.str(), Mod);

  // Register stub in kernel's metadata.
  SmallVector<Metadata*, 8> Info;
  for (auto &MD : KI.getCustomInfo()->operands())
    Info.push_back(MD.get());
  Info.push_back(MDNode::get(Ctx, {
    MDString::get(Ctx, "stub"),
    ConstantAsMetadata::get(Stub)
  }));
  KI.updateCustomInfo(MDNode::get(Ctx, Info));

  // Set argument name.
  Argument *Args = &*Stub->arg_begin();
  Args->setName("args");

  // Entry basic block.
  auto *Entry = BasicBlock::Create(Ctx, "entry", Stub);

  SmallVector<Value *, 8> KernArgs;

  // Copy each argument into a local variable.
  for (const auto &Arg : Kern->args()) {
    // Get J-th element address.
    auto *J = ConstantInt::get(Type::getInt32Ty(Ctx), Arg.getArgNo());
    Value *Addr = GetElementPtrInst::Create(nullptr, Args, J, "", Entry);

    // If it is passed by value, then we have to do an extra load.
    if (!Arg.getType()->isPointerTy())
      Addr = new LoadInst(Addr, "", Entry);

    // Cast to the appropriate type.
    auto *ArgPtrTy = PointerType::getUnqual(Arg.getType());
    auto *CastedAddr = CastInst::CreatePointerCast(Addr, ArgPtrTy, "", Entry);

    // Load argument.
    KernArgs.push_back(new LoadInst(CastedAddr, "", Entry));
  }

  // Call the kernel.
  CallInst::Create(Kern, KernArgs, "", Entry);

  // Call the implicit barrier.
  auto *Flag = ConstantInt::get(Barrier->arg_begin()->getType(), 0);
  auto *BarrierCall = CallInst::Create(Barrier, Flag, "", Entry);
  BarrierCall->setTailCall();

  // End stub.
  ReturnInst::Create(Ctx, Entry);

  // Update statistics.
  ++GroupParallelStubs;

  return true;
}

Pass *opencrun::cpu::createGroupParallelStubPass(StringRef Kernel) {
  return new GroupParallelStub(Kernel);
}
