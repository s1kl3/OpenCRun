
#include "opencrun/Passes/AllPasses.h"
#include "opencrun/Passes/AggressiveInliner.h"
#include "opencrun/Util/ModuleInfo.h"

#define DEBUG_TYPE "aggressive-inliner"

using namespace opencrun;

bool AggressiveInliner::runOnSCC(llvm::CallGraphSCC &SCC) {
  // More than one entry in the SCC, cannot be inlined.
  if(!SCC.isSingular())
    AllInlined = false;

  // Recursive self-calling, cannot be inlined.
  else {
    llvm::CallGraphNode &CurNode = **SCC.begin();
    for(unsigned I = 0, E = CurNode.size(); I != E && AllInlined; ++I)
      AllInlined = (CurNode[I] != &CurNode);
  }

  if(!AllInlined)
    return false;

  return llvm::LegacyInlinerBase::runOnSCC(SCC);
}

bool AggressiveInliner::doInitialization(llvm::CallGraph &CG) {
  llvm::Module &Mod = CG.getModule();
  ModuleInfo Info(Mod);

  llvm::SmallVector<const llvm::Function *, 16> ToVisit;
  llvm::SmallPtrSet<const llvm::Function *, 16> Visited;

  // First, get the roots of the graph.
  if(Kernel != "" && Info.contains(Kernel))
    ToVisit.push_back(Info.get(Kernel).getFunction());
  else
    for(const auto &KI : Info)
      ToVisit.push_back(KI.getFunction());

  // Then, find all reachable functions.
  while(!ToVisit.empty()) {
    const llvm::Function *CurFun = ToVisit.pop_back_val();
    llvm::CallGraphNode &CurNode = *CG[CurFun];

    for(unsigned I = 0, E = CurNode.size(); I != E ; ++I) {
      const llvm::Function *CurFun = CurNode[I]->getFunction();

      if(!Visited.count(CurFun))
        ToVisit.push_back(CurFun);
    }

    Visited.insert(CurFun);
  }

  // At last, filter out functions to not visit during the pass.
  for(llvm::Module::iterator I = Mod.begin(), E = Mod.end(); I != E; ++I)
    if(!I->isDeclaration() && !Visited.count(&*I))
      NotVisit.insert(&*I);

  return false;
}

bool AggressiveInliner::doFinalization(llvm::CallGraph &CG) {
  return removeDeadFunctions(CG, &NotVisit);
}

char AggressiveInliner::ID = 0;

AggressiveInliner *
opencrun::CreateAggressiveInlinerPass(llvm::StringRef Kernel) {
  return new AggressiveInliner(Kernel);
}

using namespace llvm;

INITIALIZE_PASS_BEGIN(AggressiveInliner,
                      "aggressive-inliner",
                      "Inline all possible functions",
                      false,
                      false)
INITIALIZE_PASS_END(AggressiveInliner,
                    "aggressive-inliner",
                    "Inline all possible functions",
                    false,
                    false)
