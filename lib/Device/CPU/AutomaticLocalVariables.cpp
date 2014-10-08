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

class ConstantUseIterator {
public:
  explicit ConstantUseIterator(llvm::Function &F, llvm::Constant *C,
                               bool End = false)
   : It(End ? C->use_end() : C->use_begin()), ItEnd(C->use_end()), Fn(F) {
    while (It != ItEnd && !llvm::isa<llvm::Instruction>(*It))
      step();
  }

  ConstantUseIterator(const ConstantUseIterator &I)
   : It(I.It), ItEnd(I.ItEnd), Stack(I.Stack), ExprPath(I.ExprPath), Fn(I.Fn) {}

  ConstantUseIterator &operator++() {
    bool End = false;
    do {
      step();
      End = It == ItEnd;
      if (!End)
        if (llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(*It))
          End = I->getParent()->getParent() == &Fn;
    } while (!End);
    return *this;
  }

  bool operator==(const ConstantUseIterator I) const {
    return &Fn == &I.Fn && It == I.It && ItEnd == I.ItEnd &&
           ExprPath == I.ExprPath && Stack == I.Stack;
  }

  bool operator!=(const ConstantUseIterator I) const {
    return !(*this == I);
  }

  llvm::Instruction *getUser() const {
    return llvm::cast<llvm::Instruction>(*It);
  }

  llvm::ConstantExpr *getExprPathElem(unsigned i) const {
    return ExprPath[i];
  }

  unsigned getExprPathSize() const {
    return ExprPath.size();
  }

private:
  typedef llvm::Value::use_iterator BaseIter;
  typedef std::pair<BaseIter, BaseIter> StackElem;

private:
  void step() {
    if (!llvm::isa<llvm::Instruction>(*It) &&
        It->use_begin() != It->use_end()) {
      Stack.push_back(StackElem(llvm::next(It), ItEnd));
      ExprPath.push_back(llvm::cast<llvm::ConstantExpr>(*It));
      ItEnd = It->use_end();
      It = It->use_begin();
    } else {
      ++It;
      while (It == ItEnd && !Stack.empty()) {
        It = Stack.back().first;
        ItEnd = Stack.back().second;
        ExprPath.pop_back();
        Stack.pop_back();
      }
    }
  }

private:
  BaseIter It;
  BaseIter ItEnd;
  llvm::SmallVector<StackElem, 8> Stack;
  llvm::SmallVector<llvm::ConstantExpr*, 8> ExprPath;
  llvm::Function &Fn;
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
  void replaceKernelLocalUses(llvm::Function &F, llvm::GlobalVariable *V,
                              llvm::Value *Ptr, unsigned FieldOffset) const;

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

void
AutomaticLocalVariables::replaceKernelLocalUses(llvm::Function &F,
                                                llvm::GlobalVariable *V,
                                                llvm::Value *Ptr,
                                                unsigned FieldOffset) const {
  using namespace llvm;


  ConstantUseIterator UI(F, V);
  ConstantUseIterator UE(F, V, true);

  while (UI != UE) {
    Instruction *CurI = UI.getUser();
    IRBuilder<> B(CurI);

    Constant *Old = V;
    Value *New = B.CreateStructGEP(Ptr, FieldOffset);
    for (unsigned i = 0, e = UI.getExprPathSize(); i != e; ++i) {
      Instruction *I = UI.getExprPathElem(i)->getAsInstruction();
      B.Insert(I);
      I->replaceUsesOfWith(Old, New);
      Old = UI.getExprPathElem(i);
      New = I;
    }

    // Increment the iterator before replace the expr in the instruction.
    ++UI;

    CurI->replaceUsesOfWith(Old, New);
  }
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
    replaceKernelLocalUses(*NF, I->first, PtrLocals, I->second);

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
