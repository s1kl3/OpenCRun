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

$(HeaderDir)/ocldef.h: ocldef.h.inc $(HeaderDir)/.dir
	$(Verb) cp $< $@
	$(Echo) Copying $(notdir $@) to build dir

###

# Hook into the standard Makefile rules.
all-local:: $(HeaderDir)/ocldef.h

install-local:: $(PROJ_headers)/ocldef.h
