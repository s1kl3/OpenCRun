#include "JITCompiler.h"

#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"

#include <dlfcn.h>

using namespace opencrun;
using namespace opencrun::cpu;

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

llvm::TargetMachine *JITCompiler::initTargetMachine(llvm::StringRef Triple) {
  // Init the native target.
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  std::string Err;
  auto *TheTarget = llvm::TargetRegistry::lookupTarget(Triple.data(), Err);

  if (!TheTarget)
    return nullptr;

  llvm::TargetOptions Options;
  return TheTarget->createTargetMachine(Triple, "", "", Options,
                                        llvm::Reloc::Model::Default,
                                        llvm::CodeModel::JITDefault);
}
