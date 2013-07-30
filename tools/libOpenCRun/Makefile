##===- tools/libOpenCRun/Makefile --------------------------*- Makefile -*-===##

LEVEL = ../..

LIBRARYNAME = OpenCRun

EXPORTED_SYMBOL_FILE = $(PROJ_SRC_DIR)/libOpenCRun.exports

LINK_LIBS_IN_SHARED = 1
SHARED_LIBRARY = 1

LINK_COMPONENTS = x86 jit asmparser bitreader bitwriter instrumentation \
		  codegen ipo selectiondag irreader option
LLVMLIBS = clangFrontend.a clangDriver.a clangSerialization.a clangCodeGen.a \
           clangParse.a clangSema.a clangAnalysis.a clangAST.a clangLex.a \
           clangBasic.a clangEdit.a clangLex.a LLVMLinker.a
USEDLIBS = opencrunCore.a opencrunDevice.a opencrunCPUDevice.a \
	   opencrunCPUPasses.a opencrunPasses.a opencrunUtil.a opencrunSystem.a

include $(LEVEL)/Makefile.common

LIBS += -lrt
