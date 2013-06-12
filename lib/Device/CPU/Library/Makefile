##===- lib/Device/CPU/Library/Makefile ---------------------*- Makefile -*-===##

#
# Indicate where we are relative to the top of the source tree.
#
LEVEL = ../../../..

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
BUILT_SOURCES = Builtins.CPU.inc

#
# The LLVM build system does not known recognize .cl files automatically.
#
SOURCES = Async.cl Common.cl Mfence.cl Geometric.cl Integer.cl Math.cl \
      Vmisc.cl Relational.cl Synchronization.cl Vloadstore.cl WorkItem.cl

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common
