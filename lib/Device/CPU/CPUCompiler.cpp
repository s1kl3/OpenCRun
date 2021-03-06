#include "CPUCompiler.h"
#include "CPUModuleInfo.h"
#include "CPUPasses.h"

#include "opencrun/Util/ModuleInfo.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include <dlfcn.h>

using namespace opencrun;
using namespace opencrun::cpu;

static std::vector<llvm::Function*>
computeReachableFunctions(const std::vector<llvm::Function*> &Roots) {
  llvm::SmallPtrSet<llvm::Function*, 8> Reachable;
  llvm::SmallVector<llvm::Function*, 8> Worklist(Roots.begin(), Roots.end());
  do {
    auto *Cur = Worklist.back();
    Worklist.pop_back();

    assert(Cur && "Null function!");

    if (!Reachable.insert(Cur).second)
      continue;

    if (Cur->isDeclaration())
      continue;

    for (auto &I : llvm::instructions(Cur))
      if (auto *CI = llvm::dyn_cast<llvm::CallInst>(&I))
        Worklist.push_back(CI->getCalledFunction());
  } while (!Worklist.empty());

  return {Reachable.begin(), Reachable.end()};
}

static void
visitConstantExpr(llvm::ConstantExpr *CE,
                  llvm::SmallPtrSetImpl<llvm::ConstantExpr*> &Visited,
                  llvm::SmallPtrSetImpl<llvm::GlobalVariable*> &Reachable) {
  llvm::SmallVector<llvm::ConstantExpr*, 16> Worklist(1, CE);
  do {
    auto *Cur = Worklist.back();
    Worklist.pop_back();

    if (Visited.count(Cur))
      continue;

    for (auto *V : Cur->operand_values())
      if (auto *OpCE = llvm::dyn_cast<llvm::ConstantExpr>(V))
        Worklist.push_back(OpCE);
      else if (auto *OpGV = llvm::dyn_cast<llvm::GlobalVariable>(V))
        Reachable.insert(OpGV);
  } while (!Worklist.empty());
}

static std::vector<llvm::GlobalVariable*>
computeReachableGlobalVariables(const std::vector<llvm::Function*> &Fns) {
  llvm::SmallPtrSet<llvm::GlobalVariable*, 8> Reachable;
  llvm::SmallPtrSet<llvm::ConstantExpr *, 8> Visited;
  for (auto *F : Fns)
    for (auto &I : llvm::instructions(F))
      for (auto *V : I.operand_values()) {
        if (auto *CE = llvm::dyn_cast<llvm::ConstantExpr>(V))
          visitConstantExpr(CE, Visited, Reachable);
        else if (auto *GV = llvm::dyn_cast<llvm::GlobalVariable>(V))
          Reachable.insert(GV);
      }

  return {Reachable.begin(), Reachable.end()};
}

template<typename Ty>
static std::vector<Ty> makeSingleton(Ty Elem) {
  auto Singleton = std::vector<Ty>{};
  Singleton.push_back(std::move(Elem));
  return Singleton;
}

static std::shared_ptr<llvm::Module> extractKernelModule(llvm::Function *F) {
  auto OldM = F->getParent();
  auto NewM = llvm::make_unique<llvm::Module>((F->getName() + ".module").str(),
                                              F->getContext());
  NewM->setDataLayout(OldM->getDataLayout());
  NewM->setTargetTriple(OldM->getTargetTriple());

  auto Fns = computeReachableFunctions(makeSingleton(F));
  auto GVs = computeReachableGlobalVariables(Fns);

  llvm::ValueToValueMapTy VMap;

  // Create global variable declarations.
  for (auto *OldGV : GVs) {
    auto *NewGV =
      new llvm::GlobalVariable(*NewM, OldGV->getType()->getElementType(),
                               OldGV->isConstant(), OldGV->getLinkage(),
                               nullptr, OldGV->getName(), nullptr,
                               OldGV->getThreadLocalMode(),
                               OldGV->getType()->getAddressSpace());
    NewGV->copyAttributesFrom(OldGV);
    VMap[OldGV] = NewGV;
  }

  // Create function declarations.
  for (auto *OldFn : Fns) {
    auto *NewFn =
      llvm::Function::Create(OldFn->getFunctionType(), OldFn->getLinkage(),
                             OldFn->getName(), NewM.get());
    NewFn->copyAttributesFrom(OldFn);
    VMap[OldFn] = NewFn;
  }

  // Clone and map global variable initializers.
  for (auto *OldGV : GVs) {
    auto *NewGV = llvm::cast<llvm::GlobalVariable>(VMap[OldGV]);
    if (OldGV->hasInitializer())
      NewGV->setInitializer(llvm::MapValue(OldGV->getInitializer(), VMap));
  }

  // Clone and map function bodies.
  for (auto *OldFn : Fns) {
    auto *NewFn = llvm::cast<llvm::Function>(VMap[OldFn]);
    if (!OldFn->isDeclaration()) {
      auto NewArgI = NewFn->arg_begin();
      for (auto ArgI = OldFn->arg_begin(); ArgI != OldFn->arg_end(); ++ArgI) {
        NewArgI->setName(ArgI->getName());
        VMap[&*ArgI] = &*NewArgI++;
      }

      llvm::SmallVector<llvm::ReturnInst*, 8> Returns; // Ignore returns cloned.
      CloneFunctionInto(NewFn, OldFn, VMap, true, Returns);
    }

    if (OldFn->hasPersonalityFn())
      NewFn->setPersonalityFn(MapValue(OldFn->getPersonalityFn(), VMap));
  }

  ModuleInfo Info(*OldM);
  if (Info.IRisSPIR()) {
    // Regenereate opencl.kernels metadata.
    auto OldKernels = OldM->getNamedMetadata("opencl.kernels");
    auto NewKernels = NewM->getOrInsertNamedMetadata("opencl.kernels");

    for (auto *MD : OldKernels->operands()) {
      KernelInfo KI(Info.get(F->getName()));
      if (std::find(Fns.begin(), Fns.end(), KI.getFunction()) != Fns.end())
        NewKernels->addOperand(MapMetadata(MD, VMap));
    }
  }

  return NewM;
}

void JITCompiler::addKernel(llvm::Function *Kern) {
  if (Kernels.count(Kern))
    return;

  auto M = extractKernelModule(Kern);

  llvm::legacy::PassManager PM;
  PM.add(createAutomaticLocalVariablesPass());
  PM.add(createGroupParallelStubPass(Kern->getName()));
  PM.run(*M);

  auto Resolver = llvm::orc::createLambdaResolver(
    [this, Kern](const std::string &Name) {
      return findSymbolForKernel(Kern, Name);
    },
    [](const std::string &Name) {
      return nullptr;
    }
  );

  // The target machine is shared between the optimizer and the jit-compiler,
  // thus we need to reset some critical options.
  TM.setOptLevel(llvm::CodeGenOpt::Default);
  TM.Options.AllowFPOpFusion = llvm::FPOpFusion::Strict;
  TM.Options.NoInfsFPMath = false;
  TM.Options.NoNaNsFPMath = false;
  TM.Options.UnsafeFPMath = false;

  Kernels[Kern] = KernelHandleT {
    llvm::cantFail(CompileLayer.addModule(M, std::move(Resolver))),
    M
  };
}

void JITCompiler::removeKernel(llvm::Function *Kern) {
  auto I = Kernels.find(Kern);
  if (I != Kernels.end()) {
    CompileLayer.removeModule(I->second.Handle);
    Kernels.erase(I);
  }
}

void *JITCompiler::getEntryPoint(llvm::Function *Kern) {
  assert(Kernels.count(Kern));
  ModuleInfo Info(*Kernels[Kern].Module);
  CPUKernelInfo KI(Info.get(Kern->getName()));
  auto StubName = KI.getStub()->getName().str();
  if (auto EntryPointOrError = findSymbolForKernel(Kern, StubName).getAddress()) {
    uintptr_t EntryPoint = *EntryPointOrError;
    return reinterpret_cast<void *>(EntryPoint);
  } else
    return nullptr;
}

size_t JITCompiler::getAutomaticLocalsSize(llvm::Function *Kern) {
  assert(Kernels.count(Kern));
  CPUModuleInfo Info(*Kernels[Kern].Module);
  return Info.getAutomaticLocalsSize();
}

llvm::JITSymbol
JITCompiler::findSymbolForKernel(llvm::Function *Kern,
                                 const std::string &Name) {
  auto I = Kernels.find(Kern);
  if (I == Kernels.end())
    return llvm::JITSymbol(nullptr);

  if (auto Addr = (llvm::JITTargetAddress)Symbols.lookup(Name))
    return llvm::JITSymbol(Addr, llvm::JITSymbolFlags::Exported);

  if (auto Addr = (llvm::JITTargetAddress)dlsym(nullptr, Name.c_str()))
    return llvm::JITSymbol(Addr, llvm::JITSymbolFlags::Exported);

  return CompileLayer.findSymbolIn(I->second.Handle, Name, false);
}

CPUCompiler::CPUCompiler() : DeviceCompiler("CPU") {
  auto Triple = llvm::sys::getProcessTriple();
  auto TargetCPU = llvm::sys::getHostCPUName();
  std::string Err;
  auto *TheTarget = llvm::TargetRegistry::lookupTarget(Triple.c_str(), Err);

  if (!TheTarget)
    llvm::report_fatal_error("No valid target!", true);

  std::vector<std::string> Features;
  llvm::StringMap<bool> FeaturesMap;
  if (llvm::sys::getHostCPUFeatures(FeaturesMap)) {
    TargetFeatures.reserve(FeaturesMap.size());
    for (const auto &E : FeaturesMap)
      Features.push_back(((E.getValue() ? "+" : "-") + E.getKey()).str());
  }

  llvm::TargetOptions Options;
  auto TargetFeatures = llvm::join(Features.begin(), Features.end(), ",");
  TM.reset(TheTarget->createTargetMachine(Triple, TargetCPU, TargetFeatures,
                                          Options, llvm::Reloc::Model::PIC_,
                                          llvm::CodeModel::Kernel,
                                          llvm::CodeGenOpt::None));

  JIT = llvm::make_unique<JITCompiler>(*TM);
}

CPUCompiler::~CPUCompiler() {}
