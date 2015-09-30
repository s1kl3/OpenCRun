#include "CPUPasses.h"

#include "opencrun/Util/ModuleInfo.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <iterator>
#include <algorithm>

#define DEBUG_TYPE "automatic-local-variables"

using namespace opencrun;

namespace {

class KernelLocals {
private:
  typedef llvm::DenseMap<llvm::GlobalVariable *, unsigned> GlobalToFieldMapping;

public:
  typedef GlobalToFieldMapping::const_iterator iterator;

private:
  KernelLocals(llvm::Function &Kern,
               llvm::SmallVector<llvm::GlobalVariable *, 8> &GVs,
               llvm::SmallVector<llvm::Type *, 8> &Fields);

public:
  KernelLocals(const KernelLocals &KL)
   : GV2FieldMap(KL.GV2FieldMap), StructTy(KL.StructTy) { }

  static KernelLocals *getKernelLocals(llvm::Function &Kern);

public:
  llvm::StructType *getStructType() const {
    return StructTy;
  }

  unsigned getFieldNumber(llvm::GlobalVariable *GV) const {
    GlobalToFieldMapping::const_iterator I = GV2FieldMap.find(GV);
    assert(I != GV2FieldMap.end());
    return I->second;
  }

  iterator begin() const { return GV2FieldMap.begin(); }
  iterator end() const { return GV2FieldMap.end(); }
  bool empty() const { return GV2FieldMap.empty(); }
  unsigned size() const { return GV2FieldMap.size(); }

private:
  GlobalToFieldMapping GV2FieldMap;
  llvm::StructType *StructTy;
};

class CallChainLocals {
private:
  typedef llvm::DenseMap<KernelLocals *, unsigned long> KernelLocalsToOffset;
  typedef llvm::SmallVector<KernelLocals *, 4> KernelLocalsVector;

public:
  typedef KernelLocalsToOffset::const_iterator iterator;

public:
  explicit CallChainLocals(llvm::Function &F, KernelLocalsVector &Locals);
  CallChainLocals(const CallChainLocals &CCL)
    : KLs2Offset(CCL.KLs2Offset), StructTy(CCL.StructTy) { }

public:
  llvm::StructType *getStructType() const {
    return StructTy;
  }

  unsigned long getOffset(KernelLocals *KLs) {
    KernelLocalsToOffset::const_iterator I = KLs2Offset.find(KLs);
    assert(I != KLs2Offset.end());
    return I->second;
  }

  iterator begin() const { return KLs2Offset.begin(); }
  iterator end() const { return KLs2Offset.end(); }
  bool empty() const { return KLs2Offset.empty(); }
  unsigned size() const { return KLs2Offset.size(); }

private:
  KernelLocalsToOffset KLs2Offset;
  llvm::StructType *StructTy;
};

class ConstantUseIterator {
public:
  explicit ConstantUseIterator(llvm::Function &F, llvm::Constant *C,
                               bool End = false)
   : It(End ? C->use_end() : C->use_begin()), ItEnd(C->use_end()), Fn(F) {
    while (It != ItEnd && !llvm::isa<llvm::Instruction>(It->getUser()))
      step();
  }

  ConstantUseIterator(const ConstantUseIterator &I)
   : It(I.It), ItEnd(I.ItEnd), Stack(I.Stack), ExprPath(I.ExprPath), Fn(I.Fn) { }

  ConstantUseIterator &operator++() {
    bool End = false;
    do {
      step();
      End = It == ItEnd;
      if (!End)
        if (llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(It->getUser()))
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
    return llvm::cast<llvm::Instruction>(It->getUser());
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
    llvm::User *U = It->getUser();
    if (!llvm::isa<llvm::Instruction>(U) &&
        U->use_begin() != U->use_end()) {
      Stack.push_back(StackElem(std::next(It), ItEnd));
      ExprPath.push_back(llvm::cast<llvm::ConstantExpr>(U));
      ItEnd = U->use_end();
      It = U->use_begin();
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
private:
  typedef llvm::DenseMap<llvm::Function *, unsigned> KernelToIdxMapping;
  typedef llvm::DenseMap<llvm::Function *, KernelLocals *> KernelToLocalsMapping;
  typedef llvm::DenseMap<llvm::Function *, llvm::Function *> FunctionToFunctionMapping;
  typedef llvm::SmallVector<KernelLocals *, 4> KernelLocalsVector;
  typedef llvm::SmallPtrSet<llvm::Function *, 4> FunctionSet;
  typedef llvm::SmallPtrSet<llvm::CallInst *, 8> CallInstSet;
  typedef std::map<llvm::Function *, std::pair<FunctionSet, CallInstSet>> FunctionToRKsMapping;

public:
  static char ID;

public:
  AutomaticLocalVariables() : llvm::ModulePass(ID) { }
  ~AutomaticLocalVariables() { llvm::DeleteContainerPointers(Locals); }

  bool runOnModule(llvm::Module &M) override;

  const char *getPassName() const { return "CPU Automatic Locals Transform"; }

private:
  bool collectKernelLocals(llvm::Function &F);
  bool getReachableKernels(llvm::Function &F,
                           FunctionSet &RKSet,
                           CallInstSet &CInstSet,
                           FunctionSet &Visited);
  void replaceKernelLocalUses(llvm::Function &F,
                              llvm::GlobalVariable *V,
                              llvm::Value *Ptr,
                              unsigned KernelLocalsIdx,
                              unsigned FieldIndex) const;
  void updateCallInst(llvm::CallInst *CInst);
  void prepareFunctionForLocals(llvm::Function &F);
  void prepareFunctionSignature(llvm::Function &F, bool isKernel = false);
  void replaceFunction(llvm::Function *Old, llvm::Function *New);

private:
  KernelToIdxMapping K2IdxMap;
  KernelToLocalsMapping K2LocalsMap;
  KernelLocalsVector Locals;
  CallInstSet CInstSet;
  FunctionToRKsMapping RKCache;
  FunctionToFunctionMapping ModifiedFunctions;
};

char AutomaticLocalVariables::ID = 0;

}

static bool isAutomaticLocal(llvm::StringRef KernelName,
                            const llvm::GlobalVariable &GV) {
  // All global variables rapresenting automatic local variables
  // have internal linkage (like C static variables).
  if (!GV.hasInternalLinkage())
    return false;

  size_t SeparatorPos = GV.getName().find('.');

  if (SeparatorPos == llvm::StringRef::npos)
    return false;

  return KernelName == GV.getName().slice(0, SeparatorPos);
}

KernelLocals::KernelLocals(llvm::Function &Kern,
                           llvm::SmallVector<llvm::GlobalVariable *, 8> &GVs,
                           llvm::SmallVector<llvm::Type *, 8> &Fields) {
  assert(GVs.size() == Fields.size());
  llvm::Module &M = *Kern.getParent();

  for (unsigned i = 0; i < GVs.size(); ++i)
    GV2FieldMap[GVs[i]] = i;  

  StructTy = llvm::StructType::get(M.getContext(), Fields);
}

KernelLocals *KernelLocals::getKernelLocals(llvm::Function &Kern) {
  llvm::StringRef KernelName = Kern.getName();
  llvm::Module &M = *Kern.getParent();

  llvm::SmallVector<llvm::GlobalVariable *, 8> GVs;
  llvm::SmallVector<llvm::Type *, 8> Fields;
  for (llvm::Module::global_iterator I = M.global_begin(),
       E = M.global_end(); I != E; ++I)
    if (isAutomaticLocal(KernelName, *I)) {
      GVs.push_back(&*I);

      // The type of an llvm::GlobalValue (i.e. functions or global variable)
      // is always a pointer to its content. This pointer must be dereferenced
      // to get the effective type of the global variable.
      Fields.push_back(I->getType()->getPointerElementType());
    }

  return !Fields.empty() ?
    new KernelLocals(Kern, GVs, Fields) : nullptr;
}

CallChainLocals::CallChainLocals(llvm::Function &F, 
                                 KernelLocalsVector &Locals) {
  using namespace llvm;

  Module &M = *F.getParent();
  const DataLayout &DL = M.getDataLayout();

  SmallVector<llvm::Type *, 8> Fields;
  unsigned long Offset = 0;
  for (KernelLocalsVector::iterator I = Locals.begin(),
       E = Locals.end(); I != E; ++I) {
    StructType *KLs_struct = (*I)->getStructType();
    Fields.push_back(KLs_struct);

    assert(!KLs2Offset.count(*I));
    KLs2Offset[*I] = Offset;

    // Calculate the offset for the next field.
    Offset += DL.getTypeStoreSize(KLs_struct);
  }

  StructTy = StructType::get(M.getContext(), Fields);
}

bool AutomaticLocalVariables::collectKernelLocals(llvm::Function &F) {
  KernelLocals *KLs = KernelLocals::getKernelLocals(F);

  if (!KLs)
    return false;

  K2LocalsMap[&F] = KLs;
  Locals.push_back(KLs);
  return true;
}

bool
AutomaticLocalVariables::getReachableKernels(llvm::Function &F,
                                             FunctionSet &RKSet,
                                             CallInstSet &CInstSet,
                                             FunctionSet &Visited) {
  using namespace llvm;

  // True if the current function F reaches one or more kernels.
  bool isKR = false;

  FunctionToRKsMapping::const_iterator It = RKCache.find(&F);
  if (It != RKCache.end()) {
    const FunctionSet &Cached_RKSet = It->second.first;
    const CallInstSet &Cached_CInstSet = It->second.second;
    if (Cached_RKSet.size()) {
      RKSet.insert(Cached_RKSet.begin(), Cached_RKSet.end());
      CInstSet.insert(Cached_CInstSet.begin(), Cached_CInstSet.end());
      isKR = true;
    }
    return isKR; 
  }

  Visited.insert(&F);
  Module &M = *F.getParent();
  FunctionSet Tmp_RKSet;
  CallInstSet Tmp_CInstSet;

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    if (CallInst *CInst = dyn_cast<CallInst>(&*I)) {
      Function *Callee = CInst->getCalledFunction();

      // Recursive call
      if (Visited.count(Callee))
        continue;

      // Previously modified functions are still present as declarations.
      if (Callee->isDeclaration() && 
          ModifiedFunctions.find(Callee) == ModifiedFunctions.end())
        continue;

      if (ModuleInfo(M).hasKernel(Callee->getName())) {
        isKR = true;
        Tmp_RKSet.insert(Callee);
        Tmp_CInstSet.insert(CInst);
        getReachableKernels(*Callee, Tmp_RKSet, Tmp_CInstSet, Visited);
      } else {
        if (getReachableKernels(*Callee, Tmp_RKSet, Tmp_CInstSet, Visited)) {
          isKR = true;
          Tmp_CInstSet.insert(CInst);
        }
      }
    }

  RKCache[&F] = std::make_pair(Tmp_RKSet, Tmp_CInstSet);
  if (Tmp_RKSet.size()) {
    RKSet.insert(Tmp_RKSet.begin(), Tmp_RKSet.end());
    CInstSet.insert(Tmp_CInstSet.begin(), Tmp_CInstSet.end());
  }

  return isKR;
}

void
AutomaticLocalVariables::replaceKernelLocalUses(llvm::Function &F,
                                                llvm::GlobalVariable *V,
                                                llvm::Value *Ptr,
                                                unsigned KernelLocalsIdx,
                                                unsigned FieldIndex) const {
  using namespace llvm;

  KernelToLocalsMapping::const_iterator It = K2LocalsMap.find(&F);
  assert(It != K2LocalsMap.end());
  Type *PtrTy = It->second->getStructType()->getPointerTo();

  LLVMContext &Ctx = F.getContext();
  Type *I32Ty = Type::getInt32Ty(Ctx);

  ConstantUseIterator UI(F, V);
  ConstantUseIterator UE(F, V, true);

  while (UI != UE) {
    Instruction *CurI = UI.getUser();
    IRBuilder<> B(CurI);

    Constant *Old = V;

    // %arrayidx = getelementptr inbounds i8** %locals_array, i32 ->KernelLocalsIdx<-
    Value *IdxList_1[1] = {
      ConstantInt::get(I32Ty, KernelLocalsIdx)
    };
    Value *GEP_1 = B.CreateInBoundsGEP(Ptr, IdxList_1);

    // %tmp_1 = load i8** %arrayidx, align 8
    LoadInst *LDInst = new LoadInst(GEP_1);
    LDInst->setAlignment(8);
    Value *Load_1 = B.Insert(LDInst);

    // %tmp_2 = bitcast i8* %tmp_1 to %struct.F_KLs*
    Value *BitCast = B.CreateBitCast(Load_1, PtrTy);
    
    // %field.addr = getelementptr inbounds %struct.F_KLs* %tmp_2, i320 0, i32 ->FieldIndex<- 
    Value *IdxList_2[2] = {
      ConstantInt::get(I32Ty, 0),
      ConstantInt::get(I32Ty, FieldIndex)
    };
    Value *New = B.CreateInBoundsGEP(BitCast, IdxList_2);

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

void AutomaticLocalVariables::updateCallInst(llvm::CallInst *CInst) {
  using namespace llvm;

  FunctionToFunctionMapping::const_iterator CalleeIt =
    ModifiedFunctions.find(CInst->getCalledFunction());
  
  // Check if the called function hasn't been modified adding an
  // (i8**) argument.
  if (CalleeIt == ModifiedFunctions.end())
    return;

  Function *NewCallee = CalleeIt->second;

  SmallVector<Value *, 8> Args;
  for (unsigned I = 0; I < CInst->getNumArgOperands(); ++I)
    Args.push_back(CInst->getArgOperand(I));

  // Add the extra-argument (i8**) from the caller function, in order
  // to pass it to the called function as its additional parameter.
  Function *Caller = CInst->getParent()->getParent();
  Args.push_back(std::prev(Caller->arg_end()));

  BasicBlock::iterator BI(CInst);
  ReplaceInstWithInst(CInst->getParent()->getInstList(),
                      BI,
                      CallInst::Create(NewCallee, Args));
}

static llvm::ConstantAsMetadata *getIntegerMD(llvm::Type *Ty, uint64_t V) {
  return llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(Ty, V));
}

void AutomaticLocalVariables::prepareFunctionForLocals(llvm::Function &F) {
  using namespace llvm;

  if (F.isDeclaration())
    return;
 
  Module &M = *F.getParent();
  LLVMContext &Ctx = F.getContext();
  bool isKernel = ModuleInfo(M).hasKernel(F.getName());

  FunctionSet RKSet, Visited;
  getReachableKernels(F, RKSet, CInstSet, Visited);

  if (isKernel) {
    // The current kernel F hasn't any automatic local variables and it
    // doesn't reach any kernel which may have them.
    if(!K2LocalsMap.count(&F) && RKSet.empty())
      return;

    // Collect KernelLocals for the current kernel F and all the kernels
    // it reaches.
    KernelLocalsVector RKLocals;
    KernelToLocalsMapping::iterator KLsIt = K2LocalsMap.find(&F);
    if (KLsIt != K2LocalsMap.end())
      RKLocals.push_back(KLsIt->second);
    for (FunctionSet::const_iterator I = RKSet.begin(), E = RKSet.end(); I != E; ++I) {
      KLsIt = K2LocalsMap.find(*I);
      if (KLsIt != K2LocalsMap.end())
        RKLocals.push_back(KLsIt->second);
    }

    // No automatic locals both inside the current kernel and
    // the reachable kernels.
    if (RKLocals.empty())
      return;
    
    CallChainLocals CCL(F, RKLocals);

    // Meta-data decoration:
    //
    // !<n> = !{!"static_local_infos", i32 <KLs_num>, i64 <sz>,
    //          !<n+1>, !<n+2>, ...}
    // !<n+1> = !{i32 <array_idx_F>, i64 <offset_F>}
    // !<n+2> = !{i32 <array_idx_K_i>, i64 <offset_K_i>}
    // ...
    //
    // where <KLs_num> is the number of kernel local structures (see KernelLocals)
    // within the LLVM module, while <sz> is the total size (in bytes) for the
    // structure belonging to the call-chain having F as its root.
    const DataLayout &DL = M.getDataLayout();
    KernelInfo KI = ModuleInfo(M).getKernelInfo(F.getName());
    llvm::MDNode *InfoMD = KI.getCustomInfo();

    SmallVector<Metadata *, 8> Info;
    for (unsigned I = 0, E = InfoMD->getNumOperands(); I != E; ++I)
      Info.push_back(InfoMD->getOperand(I));

    Type *I32Ty = Type::getInt32Ty(Ctx);
    Type *I64Ty = Type::getInt64Ty(Ctx);

    SmallVector<Metadata *, 8> Args;
    Args.push_back(MDString::get(Ctx, "static_local_infos"));
    Args.push_back(getIntegerMD(I32Ty, Locals.size()));
    Args.push_back(getIntegerMD(I64Ty, DL.getTypeStoreSize(CCL.getStructType())));
   
    // Add a meta-data for the current kernel if it declares automatic locals.
    if (K2IdxMap.count(&F)) {
      Metadata *Args_F[2] = {
        getIntegerMD(I32Ty, K2IdxMap[&F]),
        getIntegerMD(I64Ty, CCL.getOffset(K2LocalsMap[&F]))
      };
      Args.push_back(MDNode::get(Ctx, Args_F));
    }

    // The same for each reachable kernel in the call-chain declaring
    // automatic locals.
    for (FunctionSet::const_iterator I = RKSet.begin(), E = RKSet.end(); I != E; ++I)
      if (K2IdxMap.find(*I) != K2IdxMap.end()) {
        Metadata *Args_RK_I[2] = {
          getIntegerMD(I32Ty, K2IdxMap[*I]),
          getIntegerMD(I64Ty, CCL.getOffset(K2LocalsMap[*I]))
        };
        Args.push_back(MDNode::get(Ctx, Args_RK_I));
      }

    Info.push_back(MDNode::get(Ctx, Args));

    llvm::MDNode *KernelMD = KI.getKernelInfo();
    assert(KernelMD);
    unsigned NumOp = 1; 
    while (NumOp < KernelMD->getNumOperands()) {
      llvm::MDNode *N = llvm::cast<llvm::MDNode>(KernelMD->getOperand(NumOp));
      llvm::MDString *S = llvm::cast<llvm::MDString>(N->getOperand(0));
      if (S->getString() == "custom_info")
        break;
      NumOp++; 
    }

    KernelMD->replaceOperandWith(NumOp, MDNode::get(Ctx, Info));

    // All kernel functions that reach this point will have __local
    // automatic variables and their signature needs updated.
    prepareFunctionSignature(F, true);
  } else {
    // The current function F is not a kernel and it doesn't reach any kernel.
    if(RKSet.empty())
      return;
 
    // A non-kernel functions gets its signature updated only if its
    // reachable kernels have __local automatic variables.
    if (std::any_of(RKSet.begin(), RKSet.end(),
        [this](llvm::Function *K) { return K2LocalsMap.count(K); }))  
      prepareFunctionSignature(F);
  }
  
}

void AutomaticLocalVariables::prepareFunctionSignature(llvm::Function &F,
                                                       bool isKernel) {

  using namespace llvm;

  LLVMContext &Ctx = F.getContext();
  FunctionType *FTy = F.getFunctionType();

  // Compute the new function signature.
  SmallVector<Type *, 8> ArgsTypes;
  ArgsTypes.reserve(FTy->getNumParams() + 1);
  for (FunctionType::param_iterator I = FTy->param_begin(),
      E = FTy->param_end(); I != E; ++I)
    ArgsTypes.push_back(*I);
  // Add an i8** argument to the function signature.
  ArgsTypes.push_back(Type::getInt8Ty(Ctx)->getPointerTo()->getPointerTo());

  FunctionType *NewFTy = FunctionType::get(FTy->getReturnType(), ArgsTypes,
      FTy->isVarArg());

  // Create the new function and move the code from the original one.
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

  if (isKernel && K2LocalsMap.count(&F)) {
    Value *PtrLocals = &*DestI;
    KernelLocals *KL = K2LocalsMap[&F];
    unsigned KLStructIdx = K2IdxMap[&F];

    // Update kernel mappings.
    K2LocalsMap[NF] = KL;
    K2IdxMap[NF] = KLStructIdx;

    // Replace uses of globals with a GEP to the correspondent field in the
    // locals structure.
    for (KernelLocals::iterator I = KL->begin(), E = KL->end(); I != E; ++I)
      replaceKernelLocalUses(*NF, I->first, PtrLocals, KLStructIdx, I->second);

  }

  ModifiedFunctions[&F] = NF;

}

void AutomaticLocalVariables::replaceFunction(llvm::Function *Old,
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
  K2LocalsMap.erase(Old);
  M.getFunctionList().push_back(New);
}

bool AutomaticLocalVariables::runOnModule(llvm::Module &M) {
  using namespace llvm;

  ModuleInfo MI(M);

  // For each kernel function collect the automatic local variables inside
  // a struct data type.
  unsigned Idx = 0;
  for (ModuleInfo::kernel_info_iterator KI = MI.kernel_info_begin(),
       KE = MI.kernel_info_end(); KI != KE; ++KI) {
    Function *Kern = KI->getFunction();
    if (collectKernelLocals(*Kern))
      K2IdxMap[Kern] = Idx++;
  }

  for (Module::iterator FI = M.begin(), FE = M.end(); FI != FE; ++FI)
    prepareFunctionForLocals(*FI);

  // Update call instructions changing their called function to the corresponding
  // newly built function.
  for (CallInstSet::iterator I = CInstSet.begin(), E = CInstSet.end(); I != E; ++I)
    updateCallInst(*I);

  // Erase old functions from the module and force the update of metadata references.
  while (ModifiedFunctions.size()) {
    for (FunctionToFunctionMapping::iterator I = ModifiedFunctions.begin(),
         E = ModifiedFunctions.end(); I != E; ++I)
      if (I->first->use_empty()) {
        replaceFunction(I->first, I->second);
        ModifiedFunctions.erase(I);
      }
  }

  return !ModifiedFunctions.empty();
}

llvm::Pass *opencrun::cpu::createAutomaticLocalVariablesPass() {
  return new AutomaticLocalVariables();
}

