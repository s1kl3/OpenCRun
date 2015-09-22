#include "CPUCompiler.h"

#include "opencrun/Device/CPUPasses/AllPasses.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/Support/TargetRegistry.h"

#include <dlfcn.h>

using namespace opencrun;
using namespace opencrun::cpu;

static void setupTargetMachine(llvm::TargetMachine &TM) {
  TM.setOptLevel(llvm::CodeGenOpt::Default);
  TM.Options.Reciprocals = llvm::TargetRecip();
  TM.Options.AllowFPOpFusion = llvm::FPOpFusion::Strict;
  TM.Options.LessPreciseFPMADOption = false;
  TM.Options.NoInfsFPMath = false;
  TM.Options.NoNaNsFPMath = false;
  TM.Options.UnsafeFPMath = false;
}

void JITCompiler::addModule(llvm::Module *M) {
  if (HandleMap.count(M))
    return;

  auto Resolver = llvm::orc::createLambdaResolver(
    [&](const std::string &Name) {
      if (auto Sym = findSymbol(Name))
        return llvm::RuntimeDyld::SymbolInfo(Sym.getAddress(), Sym.getFlags());

      if (void *Addr = dlsym(nullptr, Name.c_str()))
        return llvm::RuntimeDyld::SymbolInfo((uint64_t)Addr, llvm::JITSymbolFlags::Exported);
      return llvm::RuntimeDyld::SymbolInfo(nullptr);
    },
    [](const std::string &Name) {
      return nullptr;
    }
  );

  // The target machine is shared between the optimizer and the jit-compiler,
  // thus we need to reset some critical options.
  setupTargetMachine(TM);

  HandleMap[M] =
    CompileLayer.addModuleSet(std::vector<llvm::Module*>(1, M),
                              llvm::make_unique<llvm::SectionMemoryManager>(),
                              std::move(Resolver));
}

void JITCompiler::removeModule(llvm::Module *M) {
  auto I = HandleMap.find(M);
  if (I != HandleMap.end()) {
    CompileLayer.removeModuleSet(I->second);
    HandleMap.erase(I);
  }
}


llvm::orc::JITSymbol JITCompiler::findSymbol(const std::string &Name) {
  auto I = SymbolMap.find(Name);
  if (I != SymbolMap.end())
    return llvm::orc::JITSymbol(llvm::orc::TargetAddress(I->getValue()),
                                llvm::JITSymbolFlags::Exported);
  return CompileLayer.findSymbol(Name, false);
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
                                          Options, llvm::Reloc::Model::Default,
                                          llvm::CodeModel::JITDefault,
                                          llvm::CodeGenOpt::None));

  JIT = llvm::make_unique<JITCompiler>(*TM);
  JIT->addModule(Builtins.get());
}

CPUCompiler::~CPUCompiler() {
  JIT->removeModule(Builtins.get());
}

void CPUCompiler::addInitialLoweringPasses(llvm::legacy::PassManager &PM) {
  PM.add(createAutomaticLocalVariablesPass());
}
