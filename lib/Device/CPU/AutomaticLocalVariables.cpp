#define DEBUG_TYPE "automatic-local-variables"
#include "opencrun/Util/ModuleInfo.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

using namespace opencrun;

namespace opencrun {
llvm::Pass *createAutomaticLocalVariablesPass();
}

namespace {

class KernelLocals {
private:
  typedef llvm::DenseMap<llvm::GlobalVariable*, unsigned> GlobalsFieldMapping;

public:
  typedef GlobalsFieldMapping::const_iterator iterator;

public:
  explicit KernelLocals(llvm::Function &F);
  KernelLocals(const KernelLocals &KL)
   : StructTy(KL.StructTy), GV2FieldMap(KL.GV2FieldMap) {}

  llvm::StructType *getStructType() const {
    return StructTy;
  }

  unsigned getFieldNumber(llvm::GlobalVariable *GV) const {
    GlobalsFieldMapping::const_iterator I = GV2FieldMap.find(GV);
    assert(I != GV2FieldMap.end());
    return I->second;
  }

  iterator begin() const { return GV2FieldMap.begin(); }
  iterator end() const { return GV2FieldMap.end(); }
  bool empty() const { return GV2FieldMap.empty(); }
  unsigned size() const { return GV2FieldMap.size(); }

private:
  llvm::StructType *StructTy;
  GlobalsFieldMapping GV2FieldMap;
};

class AutomaticLocalVariables : public llvm::ModulePass {
public:
  static char ID;
public:
  AutomaticLocalVariables() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module &M) LLVM_OVERRIDE;

  const char *getPassName() const { return "CPU Automatic Locals Transform"; }

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const LLVM_OVERRIDE {
    AU.addRequired<llvm::DataLayout>();
  }

private:
  bool prepareKernelForLocals(llvm::Function &F);
  void replaceKernel(llvm::Function *Old, llvm::Function *New);

private:
  llvm::DenseMap<llvm::Function*, llvm::Function*> ModifiedKernels;
};

char AutomaticLocalVariables::ID = 0;

}

static bool isAutomaticLocal(llvm::StringRef KernelName,
                            const llvm::GlobalVariable &GV) {
  size_t SeparatorPos = GV.getName().find('.');

  if (SeparatorPos == llvm::StringRef::npos)
    return false;

  return KernelName == GV.getName().slice(0, SeparatorPos);
}

KernelLocals::KernelLocals(llvm::Function &F) {
  llvm::StringRef KernelName = F.getName();
  llvm::Module &M = *F.getParent();

  llvm::SmallVector<llvm::Type*, 8> Fields;
  for (llvm::Module::global_iterator I = M.global_begin(),
       E = M.global_end(); I != E; ++I)
    if (isAutomaticLocal(KernelName, *I)) {
      Fields.push_back(I->getType()->getPointerElementType());
      GV2FieldMap[&*I] = Fields.size() - 1;
    }

  StructTy = llvm::StructType::get(M.getContext(), Fields);
}


bool AutomaticLocalVariables::prepareKernelForLocals(llvm::Function &F) {
  using namespace llvm;

  KernelLocals Locals(F);

  if (Locals.empty())
    return false;

  LLVMContext &Ctx = F.getContext();

  FunctionType *FTy = F.getFunctionType();

  // Compute the new kernel signature.
  SmallVector<Type*, 8> ArgsTypes;
  ArgsTypes.reserve(FTy->getNumParams() + 1);
  for (FunctionType::param_iterator I = FTy->param_begin(),
       E = FTy->param_end(); I != E; ++I)
    ArgsTypes.push_back(*I);
  ArgsTypes.push_back(Locals.getStructType()->getPointerTo());

  FunctionType *NewFTy = FunctionType::get(FTy->getReturnType(), ArgsTypes,
                                           FTy->isVarArg());

  // Create the new kernel function and move the code from the original one.
  Function *NF = Function::Create(NewFTy, F.getLinkage(), F.getName());
  NF->getBasicBlockList().splice(NF->end(), F.getBasicBlockList());

  // Remap parameters uses to the new parameters and move attributes.
  Function::arg_iterator DestI = NF->arg_begin();
  AttributeSet OldAttrs = F.getAttributes();
  for (Function::arg_iterator I = F.arg_begin(), E = F.arg_end();
       I != E; ++I, ++DestI) {
    DestI->setName(I->getName());
    AttributeSet attrs = OldAttrs.getParamAttributes(I->getArgNo());
    if (attrs.getNumSlots() > 0)
      DestI->addAttr(attrs);
    I->replaceAllUsesWith(&*DestI);
  }
  DestI->setName(F.getName() + ".locals");
  NF->setAttributes(NF->getAttributes()
                      .addAttributes(Ctx, AttributeSet::ReturnIndex,
                                     OldAttrs.getRetAttributes()));
  NF->setAttributes(NF->getAttributes()
                      .addAttributes(Ctx, AttributeSet::FunctionIndex,
                                     OldAttrs.getFnAttributes()));

  // Replace uses of globals with a GEP to the correspondent field in the
  // locals structure.
  Value *PtrLocals = &*DestI;
  for (KernelLocals::iterator I = Locals.begin(),
       E = Locals.end(); I != E; ++I)
    for (Value::use_iterator UI = I->first->use_begin(),
         UE = I->first->use_end(); UI != UE; ) {
      // We are modifing the use, so compute the next use.
      User *CurUser = *UI++;

      // Users should be only instructions...
      Instruction *CurI = cast<Instruction>(CurUser);

      // ... in the kernel itself!
      assert(CurI->getParent()->getParent() == NF &&
             "Use outside the kernel!");

      IRBuilder<> B(CurI);

      if (GetElementPtrInst *OldGEP = dyn_cast<GetElementPtrInst>(CurI)) {
        SmallVector<Value*, 4> Indices;
        Indices.push_back(ConstantInt::get(Type::getInt32Ty(Ctx), 0));
        Indices.push_back(ConstantInt::get(Type::getInt32Ty(Ctx), I->second));
        for (GetElementPtrInst::op_iterator
             II = llvm::next(OldGEP->idx_begin()),
             IE = OldGEP->idx_end(); II != IE; ++II)
          Indices.push_back(II->get());
        Value *NewGEP = OldGEP->isInBounds()
                          ? B.CreateInBoundsGEP(PtrLocals, Indices)
                          : B.CreateGEP(PtrLocals, Indices);
        OldGEP->replaceAllUsesWith(NewGEP);
        OldGEP->eraseFromParent();
      } else {
        Value *GEP = B.CreateStructGEP(PtrLocals, I->second);
        CurI->replaceUsesOfWith(I->first, GEP);
      }
    }

  ModifiedKernels[&F] = NF;


  llvm::Module &M = *F.getParent();
  llvm::DataLayout &DL = getAnalysis<llvm::DataLayout>();
  llvm::MDNode *InfoMD = ModuleInfo(M).getKernelInfo(F.getName()).getInfo();

  llvm::SmallVector<llvm::Value *, 8> Info;
  for (unsigned I = 0, E = InfoMD->getNumOperands(); I != E; ++I)
    Info.push_back(InfoMD->getOperand(I));

  llvm::Type *I64Ty = llvm::Type::getInt64Ty(Ctx);
  llvm::Value *Args[2] = {
    llvm::MDString::get(Ctx, "static_local_size"),
    llvm::ConstantInt::get(I64Ty, DL.getTypeStoreSize(Locals.getStructType()))
  };
  Info.push_back(llvm::MDNode::get(Ctx, Args));

  InfoMD->replaceAllUsesWith(llvm::MDNode::get(Ctx, Info));

  return true;
}

void AutomaticLocalVariables::replaceKernel(llvm::Function *Old,
                                            llvm::Function *New) {
  using namespace llvm;

  Module &M = *Old->getParent();

  // Now we assume there are no proper users.
  assert(Old->use_empty());

  // Trigger metadata references update.
  if (Old->hasValueHandle())
    ValueHandleBase::ValueIsRAUWd(Old, New);

  // Replace the kernel function.
  Old->eraseFromParent();
  M.getFunctionList().push_back(New);
}

bool AutomaticLocalVariables::runOnModule(llvm::Module &M) {
  using namespace llvm;

  ModuleInfo MI(M);

  for (ModuleInfo::kernel_info_iterator I = MI.kernel_info_begin(),
       E = MI.kernel_info_end(); I != E; ++I)
    prepareKernelForLocals(*I->getFunction());

  for (DenseMap<Function*, Function*>::iterator I = ModifiedKernels.begin(),
       E = ModifiedKernels.end(); I != E; ++I)
    replaceKernel(I->first, I->second);

  return !ModifiedKernels.empty();
}

llvm::Pass *opencrun::createAutomaticLocalVariablesPass() {
  return new AutomaticLocalVariables();
}
