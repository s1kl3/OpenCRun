##===- unittests/Runtime/Makefile --------------------------*- Makefile -*-===##

#
# Relative path to the top of the source tree.
#
LEVEL = ../..

#
# Name of the test.
#
TESTNAME = Runtime

LINK_COMPONENTS = support

include $(LEVEL)/Makefile.config
include $(LLVM_SRC_ROOT)/unittests/Makefile.unittest

export OPENCRUN_PREFIX_LLVM=$(LLVM_OBJ_ROOT)/$(BuildMode)
export OPENCRUN_PREFIX=$(PROJ_OBJ_ROOT)/$(BuildMode)

# Define CXX.Flags after including rules.
CXX.Flags += -Wno-cast-qual
LD.Flags += $(RPATH) -Wl,$(SharedLibDir)

LIBS += -lOpenCRun
