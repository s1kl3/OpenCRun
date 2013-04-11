##===- lib/Device/CPULib/Makefile --------------------------*- Makefile -*-===##

#
# Indicate where we are relative to the top of the source tree.
#
LEVEL = ../../..

#
# Give the name of a library.
#
MODULE_NAME = opencrunCPULib

#
# The ocldef is in fact tblgen, so declare we want tblgen support to enable .inc
# files handling.
#
TABLEGEN_INC_FILES_COMMON = 1

#
# Auto-generate library.
#
BUILT_SOURCES = Builtins.inc 

#
# The LLVM build system does not known recognize .cl files automatically.
#
SOURCES = Async.cl Common.inc Geometric.cl Integer.cl Math.cl Relational.cl \
		  Synchronization.cl WorkItem.cl

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common

# To avoid "option unknown" warnings caused by "-Wno-maybe-uninitialized".
NO_MAYBE_UNINITIALIZED = -Wno-uninitialized

$(ObjDir)/Builtins.inc.tmp: $(PROJ_SRC_ROOT)/include/opencrun/Device/GenLib/OCLDef.td \
							$(OCLGEN) $(ObjDir)/.dir
	$(Echo) "Building Builtins library with oclgen"
	$(Verb) $(OCLGen) -gen-ocl-lib-impl -o $(call SYSPATH, $@) $<
