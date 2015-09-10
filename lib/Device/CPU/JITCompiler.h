#ifndef JITCOMPILER_H
#define JITCOMPILER_H

#include "llvm/ADT/StringRef.h"
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
  JITCompiler(llvm::StringRef Triple)
   : TM(initTargetMachine(Triple)),
     CompileLayer(ObjLayer, llvm::orc::SimpleCompiler(*TM)) {}

  void addModule(llvm::Module *M);
  void removeModule(llvm::Module *M);

  llvm::orc::JITSymbol findSymbol(const std::string &Name);

  void addSymbolMapping(llvm::StringRef Sym, void *Addr) {
    SymbolMap[Sym] = Addr;
  }

  void removeSymbolMapping(llvm::StringRef Sym) {
    SymbolMap.erase(Sym);
  }

private:
  static llvm::TargetMachine *initTargetMachine(llvm::StringRef Triple);

private:
  llvm::TargetMachine *TM;
  ObjectLayerT ObjLayer;
  IRCompileLayerT CompileLayer;

  llvm::DenseMap<llvm::Module*, ModuleHandleT> HandleMap;
  llvm::StringMap<void*> SymbolMap;
};

}
}

#endif
