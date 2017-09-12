#ifndef CPUCOMPILER_H
#define CPUCOMPILER_H

#include "opencrun/Core/DeviceCompiler.h"

#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/LazyEmittingLayer.h"

namespace llvm {
class Function;
class TargetMachine;
}

namespace opencrun {
namespace cpu {

class JITCompiler {
public:
  JITCompiler(llvm::TargetMachine &TM)
   : TM(TM),
     ObjLayer([]() { return std::make_shared<llvm::SectionMemoryManager>(); }),
     CompileLayer(ObjLayer, llvm::orc::SimpleCompiler(TM)) {}

  void addKernel(llvm::Function *Kern);
  void removeKernel(llvm::Function *Kern);

  void *getEntryPoint(llvm::Function *Kern);
  size_t getAutomaticLocalsSize(llvm::Function *Kern);

  void addSymbolMapping(llvm::StringRef Sym, void *Addr) {
    Symbols[Sym] = Addr;
  }

private:
  using ObjectLayerT = llvm::orc::RTDyldObjectLinkingLayer;
  using CompileFtorT = llvm::orc::SimpleCompiler;
  using IRCompileLayerT = llvm::orc::IRCompileLayer<ObjectLayerT, CompileFtorT>;

  using ModuleHandleT = IRCompileLayerT::ModuleHandleT;

  struct KernelHandleT {
    ModuleHandleT Handle;
    std::shared_ptr<llvm::Module> Module;
  };

private:
  llvm::JITSymbol findSymbolForKernel(llvm::Function *Kern,
                                      const std::string &Name);

private:
  llvm::TargetMachine &TM;
  ObjectLayerT ObjLayer;
  IRCompileLayerT CompileLayer;

  llvm::DenseMap<llvm::Function *, KernelHandleT> Kernels;
  llvm::StringMap<void*> Symbols;
};

class CPUCompiler : public DeviceCompiler {
public:
  CPUCompiler();
  ~CPUCompiler();

  void addKernel(llvm::Function *Kern) {
    llvm::sys::ScopedLock Lock(ThisLock);
    JIT->addKernel(Kern);
  }

  void removeKernel(llvm::Function *Kern) {
    llvm::sys::ScopedLock Lock(ThisLock);
    JIT->removeKernel(Kern);
  }

  void *getEntryPoint(llvm::Function *Kern) {
    llvm::sys::ScopedLock Lock(ThisLock);
    return JIT->getEntryPoint(Kern);
  }

  size_t getAutomaticLocalsSize(llvm::Function *Kern) {
    llvm::sys::ScopedLock Lock(ThisLock);
    return JIT->getAutomaticLocalsSize(Kern);
  }

  void addSymbolMapping(llvm::StringRef Sym, void *Addr) {
    JIT->addSymbolMapping(Sym, Addr);
  }

private:
  std::unique_ptr<JITCompiler> JIT;
};

}
}

#endif
