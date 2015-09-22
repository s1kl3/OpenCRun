
#ifndef OPENCRUN_CORE_DEVICECOMPILER_H
#define OPENCRUN_CORE_DEVICECOMPILER_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Mutex.h"

namespace llvm {
class Module;
class raw_ostream;
class TargetMachine;

namespace legacy {
class PassManager;
}
}

namespace clang {
class CompilerInvocation;
class DiagnosticsEngine;
}

namespace opencrun {

class DeviceCompiler {
public:
  explicit DeviceCompiler(llvm::StringRef Name);
  virtual ~DeviceCompiler();

  std::unique_ptr<llvm::Module>
      compileSource(llvm::MemoryBuffer &Src, llvm::StringRef Opts,
                    llvm::raw_ostream &OS);

  bool optimize(llvm::Module &M);

  std::unique_ptr<llvm::Module>
      linkModules(llvm::ArrayRef<llvm::Module*> Modules);

  bool linkInBuiltins(llvm::Module &M);

  std::unique_ptr<llvm::Module> loadBitcode(llvm::MemoryBufferRef Cnt);

protected:
  virtual void addInitialLoweringPasses(llvm::legacy::PassManager &PM);

private:
  std::unique_ptr<clang::CompilerInvocation>
      createInvocation(llvm::MemoryBuffer &Src, llvm::StringRef UserOpts,
                       clang::DiagnosticsEngine &Diags);

protected:
  mutable llvm::sys::Mutex ThisLock;

  llvm::LLVMContext Ctx;
  std::unique_ptr<llvm::Module> Builtins;
  std::unique_ptr<llvm::TargetMachine> TM;
  std::vector<std::string> TargetFeatures;
  llvm::StringRef Name;
};

}

#endif
