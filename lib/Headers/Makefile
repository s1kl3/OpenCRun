##===- lib/Headers/Makefile --------------------------------*- Makefile -*-===##

#
# Indicate where we are relative to the top of the source tree.
#
LEVEL = ../..

TABLEGEN_INC_FILES_COMMON = 1
BUILT_SOURCES = oclconst.h.inc ocltype.h.inc oclbuiltin.h.inc

HEADERS := oclconst.h ocltype.h oclbuiltin.h

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common

$(ObjDir)/oclconst.h.inc.tmp: $(CommonLibraryDir)/OpenCL.td $(OCLGEN) $(OCLGenLibrary) $(ObjDir)/.dir
	$(Echo) Generating OCL builtins library definition with oclgen
	$(Verb) $(OCLGen) -gen-ocl-constants -o $(call SYSPATH, $@) $<

$(ObjDir)/oclbuiltin.h.inc.tmp: $(CommonLibraryDir)/OpenCL.td $(OCLGEN) $(OCLGenLibrary) $(ObjDir)/.dir
	$(Echo) Generating OCL builtins library definition with oclgen
	$(Verb) $(OCLGen) -gen-ocl-builtin-defs -o $(call SYSPATH, $@) $<

$(ObjDir)/ocltype.h.inc.tmp: $(CommonLibraryDir)/OpenCL.td $(OCLGEN) $(OCLGenLibrary) $(ObjDir)/.dir
	$(Echo) Generating OCL types definition with oclgen
	$(Verb) $(OCLGen) -gen-ocl-types -o $(call SYSPATH, $@) $<

###
