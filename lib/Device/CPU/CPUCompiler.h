#ifndef CPUCOMPILER_H
#define CPUCOMPILER_H

#include "opencrun/Core/DeviceCompiler.h"

#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/LazyEmittingLayer.h"

namespace llvm {
class TargetMachine;
}

namespace opencrun {
namespace cpu {

class JITCompiler {
public:
  using ObjectLayerT = llvm::orc::ObjectLinkingLayer<>;
  using IRCompileLayerT = llvm::orc::IRCompileLayer<ObjectLayerT>;

  using ModuleHandleT = IRCompileLayerT::ModuleSetHandleT;

public:
  JITCompiler(llvm::TargetMachine &TM)
   : TM(TM), CompileLayer(ObjLayer, llvm::orc::SimpleCompiler(TM)) {}

  void addModule(llvm::Module *M);
  void removeModule(llvm::Module *M);

  llvm::orc::JITSymbol findSymbol(const std::string &Name);

  void addSymbolMapping(llvm::StringRef Sym, void *Addr) {
    SymbolMap[Sym] = Addr;
  }

private:
  llvm::TargetMachine &TM;
  ObjectLayerT ObjLayer;
  IRCompileLayerT CompileLayer;

  llvm::DenseMap<llvm::Module*, ModuleHandleT> HandleMap;
  llvm::StringMap<void*> SymbolMap;
};

class CPUCompiler : public DeviceCompiler {
public:
  CPUCompiler();
  ~CPUCompiler();

  void addModule(llvm::Module *M) {
    llvm::sys::ScopedLock Lock(ThisLock);
    JIT->addModule(M);
  }

  void removeModule(llvm::Module *M) {
    JIT->removeModule(M);
  }

  void addSymbolMapping(llvm::StringRef Sym, void *Addr) {
    JIT->addSymbolMapping(Sym, Addr);
  }

  void *getSymbolAddr(const std::string &Name) {
    if (auto Sym = JIT->findSymbol(Name))
      return reinterpret_cast<void*>(Sym.getAddress());
    return nullptr;
  }

  void addInitialLoweringPasses(llvm::legacy::PassManager &PM) override;

private:
  std::unique_ptr<JITCompiler> JIT;
};

}
}

#endif
