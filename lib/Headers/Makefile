##===- lib/Headers/Makefile --------------------------------*- Makefile -*-===##

#
# Indicate where we are relative to the top of the source tree.
#
LEVEL = ../..

TABLEGEN_INC_FILES_COMMON = 1
BUILT_SOURCES = ocldef.h.inc ocltype.h.inc

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common

$(ObjDir)/ocldef.h.inc.tmp: $(BUILTINS_DIR)/OpenCL.td $(OCLGENTool) $(ObjDir)/.dir
	$(Echo) Generating OCL builtins library definition with oclgen
	$(Verb) $(OCLGen) -gen-ocl-lib-def -o $(call SYSPATH, $@) $<

$(ObjDir)/ocltype.h.inc.tmp: $(BUILTINS_DIR)/OpenCL.td $(OCLGENTool) $(ObjDir)/.dir
	$(Echo) Generating OCL types definition with oclgen
	$(Verb) $(OCLGen) -gen-ocl-types -o $(call SYSPATH, $@) $<

###

HEADERS := ocldef.h ocltype.h
OBJHEADERS := $(addprefix $(HeaderDir)/, $(HEADERS))
INSTHEADERS := $(addprefix $(PROJ_headers)/, $(HEADERS))

$(OBJHEADERS) : $(HeaderDir)/%.h: %.h.inc $(HeaderDir)/.dir
	$(Verb) cp $< $@
	$(Echo) Copying $(notdir $@) to build dir

$(PROJ_headers):
	$(Verb) $(MKDIR) $@

$(INSTHEADERS) : $(PROJ_headers)/%.h: $(HeaderDir)/%.h | $(PROJ_headers)
	$(Verb) $(DataInstall) $< $(PROJ_headers)
	$(Echo) Installing compiler include file: $(notdir $<)

# Hook into the standard Makefile rules.
all-local:: $(OBJHEADERS)

install-local:: $(INSTHEADERS)
