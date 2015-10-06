#include "CPUPasses.h"

#include "opencrun/Util/ModuleInfo.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#define DEBUG_TYPE "automatic-local-variables"

using namespace llvm;

namespace {

class AutomaticLocals {
private:
  using GlobalToFieldMapping = DenseMap<GlobalVariable*, unsigned>;

public:
  using iterator = GlobalToFieldMapping::const_iterator;

public:
  AutomaticLocals(const AutomaticLocals &KL) = default;
  AutomaticLocals(AutomaticLocals &&KL) = default;

  ~AutomaticLocals() {
    for (auto GV : GVs)
      GV->eraseFromParent();
  }

  static AutomaticLocals create(Module &M);

public:
  StructType *getStructType() const {
    return StructTy;
  }

  unsigned getFieldNumber(GlobalVariable *GV) const {
    GlobalToFieldMapping::const_iterator I = GV2FieldMap.find(GV);
    assert(I != GV2FieldMap.end());
    return I->second;
  }

  iterator begin() const { return GV2FieldMap.begin(); }
  iterator end() const { return GV2FieldMap.end(); }
  bool empty() const { return GV2FieldMap.empty(); }
  unsigned size() const { return GV2FieldMap.size(); }

  ArrayRef<GlobalVariable*> getGlobals() const { return GVs; }

private:
  explicit AutomaticLocals(ArrayRef<GlobalVariable*> GVs,
                           ArrayRef<Type*> Fields);

private:
  GlobalToFieldMapping GV2FieldMap;
  StructType *StructTy;
  std::vector<GlobalVariable*> GVs;
};

class AutomaticLocalVariables : public ModulePass {
public:
  static char ID;

public:
  AutomaticLocalVariables() : ModulePass(ID) {}

  const char *getPassName() const {
    return "CPU Automatic Locals Transform";
  }

  bool runOnModule(Module &M) override;

};

char AutomaticLocalVariables::ID = 0;
RegisterPass<AutomaticLocalVariables> X("cpu-automatic-locals",
                                        "CPU Automatic Locals Transform");
}

static bool isAutomaticLocal(ArrayRef<StringRef> Kernels,
                            const GlobalVariable &GV) {
  // All global variables rapresenting automatic local variables
  // have internal linkage (like C static variables).
  if (!GV.hasInternalLinkage())
    return false;

  if (GV.use_empty())
    return false;

  size_t SeparatorPos = GV.getName().find('.');
  if (SeparatorPos == StringRef::npos)
    return false;

  auto CandidateKernel = GV.getName().slice(0, SeparatorPos);
  for (auto &KernelName : Kernels)
    if (CandidateKernel == KernelName)
      return true;

  return false;
}

AutomaticLocals::AutomaticLocals(ArrayRef<GlobalVariable*> GVs,
                                 ArrayRef<Type*> Fields)
 : StructTy(nullptr) {
  assert(GVs.size() == Fields.size());

  this->GVs.assign(GVs.begin(), GVs.end());

  for (unsigned i = 0; i < GVs.size(); ++i)
    GV2FieldMap[GVs[i]] = i;

  StructTy =
    !Fields.empty() ? StructType::get(Fields[0]->getContext(), Fields)
                    : nullptr;
}

AutomaticLocals AutomaticLocals::create(Module &M) {
  SmallVector<StringRef, 8> Kernels;
  opencrun::ModuleInfo Info(M);
  for (const auto &KI : Info)
    Kernels.push_back(KI.getName());

  SmallVector<GlobalVariable *, 8> GVs;
  SmallVector<Type *, 8> Fields;
  for (auto &GV : M.globals())
    if (isAutomaticLocal(Kernels, GV)) {
      GVs.push_back(&GV);
      Fields.push_back(GV.getType()->getPointerElementType());
    }

  return AutomaticLocals {GVs, Fields};
}

static void expandConstantExprUsersOf(Constant *C) {
  for (auto &U : C->uses())
    if (auto *CE = dyn_cast<ConstantExpr>(U.getUser()))
      expandConstantExprUsersOf(CE);
  C->removeDeadConstantUsers();

  if (auto *CE = dyn_cast<ConstantExpr>(C)) {
    while (!CE->use_empty()) {
      auto &U = *CE->use_begin();
      auto *Inst = CE->getAsInstruction();

      if (auto *PN = dyn_cast<PHINode>(U.getUser())) {
        auto IP = PN->getIncomingBlock(U)->getTerminator();
        Inst->insertBefore(IP);
        U.set(Inst);
      } else {
        auto UserInst = cast<Instruction>(U.getUser());
        Inst->insertBefore(UserInst);
        UserInst->replaceUsesOfWith(CE, Inst);
      }
    }
  }
}

using InstFunctor = std::function<void(Instruction*)>;

static void visitInstructionUsers(const Use &U, InstFunctor Process) {
  if (auto *Inst = dyn_cast<Instruction>(U.getUser()))
    return Process(Inst);

  auto *CE = cast<ConstantExpr>(U.getUser());
  for (auto &U : CE->uses())
    visitInstructionUsers(U, Process);
}

using FunctionTranslationMap = DenseMap<Function*, Function*>;

static void regenerateFunction(Function *Fn, Type *HTy,
                               FunctionTranslationMap &F2FMap) {
  // Build the new function type.
  auto *FnTy = Fn->getFunctionType();
  SmallVector<Type *, 8> ParamTypes;
  for (auto *Ty : FnTy->params())
    ParamTypes.push_back(Ty);
  ParamTypes.push_back(HTy);

  auto *NewFnTy = FunctionType::get(FnTy->getReturnType(), ParamTypes,
                                    FnTy->isVarArg());

  // Create new function.
  auto *NewFn = Function::Create(NewFnTy, Fn->getLinkage());
  NewFn->takeName(Fn);
  Fn->getParent()->getFunctionList().insert(Fn, NewFn);
  F2FMap[Fn] = NewFn;

  // Move the function body from old to new.
  while (!Fn->getBasicBlockList().empty()) {
    auto *BB = &*Fn->begin();
    BB->removeFromParent();
    BB->insertInto(NewFn);
  }

  // Update references to function arguments
  auto NewArgI = NewFn->arg_begin();
  for (auto &Arg : Fn->args()) {
    NewArgI->setName(Arg.getName());
    Arg.replaceAllUsesWith(NewArgI);
    ++NewArgI;
  }
  NewArgI->setName("automatic.locals.ptr");

  // Finalize body deletion.
  Fn->deleteBody();
}

static void updateCallUsers(Use &U, const FunctionTranslationMap &F2FMap) {
  if (!isa<CallInst>(U.getUser()))
    llvm_unreachable(0);

  auto *Call = cast<CallInst>(U.getUser());
  auto *Callee = Call->getCalledFunction();
  auto *NewCallee = F2FMap.lookup(Callee);
  auto *HiddenArg = &Call->getParent()->getParent()->getArgumentList().back();
  unsigned HiddenArgPos = NewCallee->getFunctionType()->getNumParams() - 1;

  assert(HiddenArg && HiddenArgPos <= Call->getNumArgOperands());

  // Build new arguments list.
  SmallVector<Value*, 8> Args;
  for (unsigned i = 0; i != HiddenArgPos; ++i)
    Args.push_back(Call->getArgOperand(i));
  Args.push_back(HiddenArg);
  for (unsigned i = HiddenArgPos; i != Call->getNumArgOperands(); ++i)
    Args.push_back(Call->getArgOperand(i));

  // Replace call instruction.
  auto *NewCall = CallInst::Create(NewCallee, Args);
  NewCall->takeName(Call);
  NewCall->insertBefore(Call);
  if (Call->hasMetadata()) {
    SmallVector<std::pair<unsigned, MDNode*>, 4> TheMDs;
    Call->getAllMetadata(TheMDs);
    for (const auto &MD : TheMDs)
      NewCall->setMetadata(MD.first, MD.second);
  }
  Call->replaceAllUsesWith(NewCall);
  Call->eraseFromParent();
}

bool AutomaticLocalVariables::runOnModule(Module &M) {
  auto Locals = AutomaticLocals::create(M);

  if (Locals.empty())
    return false;

  // Expand all constant expressions referring automatic locals.
  for (const auto &Entry : Locals)
    expandConstantExprUsersOf(Entry.first);

  // Compute the set of functions to modify.
  SmallPtrSet<Function*, 8> Fns;
  SmallVector<Constant*, 16> WorkList(Locals.getGlobals().begin(),
                                      Locals.getGlobals().end());
  do {
    auto *Cur = WorkList.pop_back_val();

    if (auto *Fn = dyn_cast<Function>(Cur))
      if (!Fns.insert(Fn).second)
        continue;

    assert(isa<GlobalVariable>(Cur) || isa<Function>(Cur));

    for (auto &U : Cur->uses()) {
      auto AddFnToWorkList = [&WorkList](Instruction *I) {
        WorkList.push_back(I->getParent()->getParent());
      };

      visitInstructionUsers(U, AddFnToWorkList);
    }
  } while (!WorkList.empty());

  auto *PtrStructTy = PointerType::get(Locals.getStructType(), 0);

  // Mutate function types and create hidden arguments.
  FunctionTranslationMap F2FMap;
  for (auto *Fn : Fns)
    regenerateFunction(Fn, PtrStructTy, F2FMap);

  // Visit all CallInst users of the functions and propagate the corresponding
  // hidden argument value.
  for (auto *Fn : Fns)
    for (auto &U : Fn->uses())
      updateCallUsers(U, F2FMap);

  // Replace uses of automatic local global variables with a GEP of the struct
  // passed through the hidden argument.
  for (auto &Entry : Locals)
    while (!Entry.first->use_empty()) {
      auto &U = *Entry.first->use_begin();
      auto *Inst = cast<Instruction>(U.getUser());
      auto *Fn = Inst->getParent()->getParent();
      auto *HiddenArg = &Fn->getArgumentList().back();

      IRBuilder<> B(Inst);
      U.set(B.CreateStructGEP(Locals.getStructType(), HiddenArg,
                              Entry.second, Entry.first->getName()));
    }

  // Rewrite kernels metadata.
  auto *Kernels = M.getNamedMetadata("opencl.kernels");
  for (unsigned i = 0; i != Kernels->getNumOperands(); ++i) {
    SmallVector<Metadata*, 8> Info;
    auto *MD = Kernels->getOperand(i);
    for (auto &Op : MD->operands())
      Info.push_back(Op.get());

    auto *Fn = mdconst::extract<Function>(Info[0]);
    Info[0] = (ConstantAsMetadata::get(F2FMap.lookup(Fn)));

    Kernels->setOperand(i, MDNode::get(M.getContext(), Info));
  }

  // Remove old dead functions.
  for (auto *Fn : Fns)
    Fn->eraseFromParent();

  // Register stub in automatic locals size.
  auto &Ctx = M.getContext();
  auto &DL = M.getDataLayout();
  auto *SL = DL.getStructLayout(Locals.getStructType());
  auto *IntTy = Type::getInt64Ty(Ctx);
  auto *ModInfo = M.getOrInsertNamedMetadata("opencrun.module.info");
  auto *ALSMD = MDNode::get(Ctx, {
    MDString::get(Ctx, "automatic_locals"),
    ConstantAsMetadata::get(ConstantInt::get(IntTy, SL->getSizeInBytes())),
    ConstantAsMetadata::get(ConstantInt::get(IntTy, SL->getAlignment()))
  });
  ModInfo->addOperand(ALSMD);

  return true;
}

Pass *opencrun::cpu::createAutomaticLocalVariablesPass() {
  return new AutomaticLocalVariables();
}
