##===- lib/Headers/Makefile --------------------------------*- Makefile -*-===##

#
# Indicate where we are relative to the top of the source tree.
#
LEVEL = ../..

TABLEGEN_INC_FILES_COMMON = 1
BUILT_SOURCES = ocldef.h.inc

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common

###


HeaderDir := $(LibDir)/opencrun/include

$(HeaderDir)/ocldef.h: $(BUILT_SOURCES) $(HeaderDir)/.dir
	$(Verb) cp $< $@
	$(Echo) Copying $(notdir $<) to build dir

# Hook into the standard Makefile rules.
all-local:: $(HeaderDir)/ocldef.h


###

PROJ_headers := $(DESTDIR)$(PROJ_prefix)/lib/opencrun/include

HEADERS = ocldef.h

INSTHEADERS := $(addprefix $(PROJ_headers)/, $(HEADERS))

$(PROJ_headers):
	$(Verb) $(MKDIR) $@

$(INSTHEADERS): $(PROJ_headers)/%.h: $(HeaderDir)/%.h | $(PROJ_headers)
	$(Verb) $(DataInstall) $< $(PROJ_headers)
	$(Echo) Installing compiler include file: $(notdir $<)

install-local:: $(INSTHEADERS)

###

$(ObjDir)/ocldef.h.inc.tmp: $(LEVEL)/include/opencrun/Device/GenLib/OCLDef.td $(OCLGEN) $(ObjDir)/.dir
	$(Echo) Generating '$(notdir $@)' with oclgen
	$(Verb) $(OCLGen) -gen-ocldef -o $(call SYSPATH, $@) $<

