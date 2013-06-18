##===- lib/Headers/Makefile --------------------------------*- Makefile -*-===##

#
# Indicate where we are relative to the top of the source tree.
#
LEVEL = ../..

TABLEGEN_INC_FILES_COMMON = 1
BUILT_SOURCES = ocldef.h.inc ocltype.h.inc

HEADERS := ocldef.h ocltype.h

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common

$(ObjDir)/ocldef.h.inc.tmp: $(LIBRARY_DIR)/OpenCL.td $(OCLGENTool) $(ObjDir)/.dir
	$(Echo) Generating OCL builtins library definition with oclgen
	$(Verb) $(OCLGen) -gen-ocl-lib-def -o $(call SYSPATH, $@) $<

$(ObjDir)/ocltype.h.inc.tmp: $(LIBRARY_DIR)/OpenCL.td $(OCLGENTool) $(ObjDir)/.dir
	$(Echo) Generating OCL types definition with oclgen
	$(Verb) $(OCLGen) -gen-ocl-types -o $(call SYSPATH, $@) $<

###
