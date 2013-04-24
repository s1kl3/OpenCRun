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
# Auto-generate some libraries.
#
BUILT_SOURCES = Builtins.CPU.inc ocldef.CPU.h.inc

#
# The LLVM build system does not known recognize .cl files automatically.
#
SOURCES = WorkItem.cl Synchronization.cl Math.cl

#
# Define target headers
#
TargetHeaders = $(HeaderDir)/ocldef.CPU.h

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common

#
# Append implicit include in order to build correctly the library
#
TargetCommonOpts += -include "ocldef.CPU.h"

# Hook into the standard Makefile rules.
all-local:: $(HeaderDir)/ocldef.CPU.h

install-local:: $(PROJ_headers)/ocldef.CPU.h
