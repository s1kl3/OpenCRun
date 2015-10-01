#ifndef CPUCOMPILER_H
#define CPUCOMPILER_H

#include "opencrun/Core/DeviceCompiler.h"

#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/LazyEmittingLayer.h"

namespace llvm {
class Function;
class TargetMachine;
}

namespace opencrun {
namespace cpu {

class JITCompiler {
private:
  using ObjectLayerT = llvm::orc::ObjectLinkingLayer<>;
  using IRCompileLayerT = llvm::orc::IRCompileLayer<ObjectLayerT>;

  using KernelModuleHandleT = IRCompileLayerT::ModuleSetHandleT;

public:
  JITCompiler(llvm::TargetMachine &TM)
   : TM(TM), CompileLayer(ObjLayer, llvm::orc::SimpleCompiler(TM)) {}

  void *addKernel(llvm::Function *Kern);
  void removeKernel(llvm::Function *Kern);

  void addSymbolMapping(llvm::StringRef Sym, void *Addr) {
    Symbols[Sym] = Addr;
  }

private:
  llvm::orc::JITSymbol findSymbolForKernel(llvm::Function *Kern,
                                           const std::string &Name);

private:
  llvm::TargetMachine &TM;
  ObjectLayerT ObjLayer;
  IRCompileLayerT CompileLayer;

  llvm::DenseMap<llvm::Function *, KernelModuleHandleT> Kernels;
  llvm::StringMap<void*> Symbols;
};

class CPUCompiler : public DeviceCompiler {
public:
  CPUCompiler();
  ~CPUCompiler();

  void *addKernel(llvm::Function *Kern) {
    llvm::sys::ScopedLock Lock(ThisLock);
    return JIT->addKernel(Kern);
  }

  void removeKernel(llvm::Function *Kern) {
    llvm::sys::ScopedLock Lock(ThisLock);
    JIT->removeKernel(Kern);
  }

  void addSymbolMapping(llvm::StringRef Sym, void *Addr) {
    JIT->addSymbolMapping(Sym, Addr);
  }

  void addInitialLoweringPasses(llvm::legacy::PassManager &PM) override;

private:
  std::unique_ptr<JITCompiler> JIT;
};

}
}

#endif
