##===- test/Makefile ---------------------------------------*- Makefile -*-===##

LEVEL = ..
DIRS  =

all:: check-local

# Force including configuration here. We need vars for Makefile.tests.
include $(LEVEL)/Makefile.config

# Include other test rules
include Makefile.tests

#===------------------------------------------------------------------------===#
# DejaGNU testing support
#===------------------------------------------------------------------------===#

ifneq ($(GREP_OPTIONS),)
$(warning GREP_OPTIONS environment variable may interfere with test results)
endif

ifdef VERBOSE
LIT_ARGS := -v
else
LIT_ARGS := -s -v
endif

ifdef TESTSUITE
LIT_TESTSUITE := $(TESTSUITE)
CLEANED_TESTSUITE := $(patsubst %/,%,$(TESTSUITE))
CLEANED_TESTSUITE := $(patsubst test/%,%,$(CLEANED_TESTSUITE))
else
LIT_TESTSUITE := .
endif

ifdef VG
VALGRIND := valgrind --tool=memcheck --quiet --trace-children=yes --error-exitcode=3 --leak-check=full $(VALGRIND_EXTRA_ARGS)
endif

# Check what to run for -all.
LIT_ALL_TESTSUITES := $(LIT_TESTSUITE)

extra-site-cfgs::
.PHONY: extra-site-cfgs

check-local:: lit.site.cfg
	( $(ULIMIT) \
	  $(PYTHON) $(LLVM_SRC_ROOT)/utils/lit/lit.py $(LIT_ARGS) $(LIT_TESTSUITE) )

# This is a legacy alias dating from when both DejaGNU and lit were in use.
check-local-lit:: check-local

check-local-all:: lit.site.cfg extra-site-cfgs
	( $(ULIMIT) \
	  $(PYTHON) $(LLVM_SRC_ROOT)/utils/lit/lit.py $(LIT_ARGS) $(LIT_ALL_TESTSUITES) )

clean::
	$(RM) -rf `find $(LLVM_OBJ_ROOT)/test -name Output -type d -print`

ifeq ($(TARGET_OS),Darwin)
  DSYMUTIL := dsymutil
else
  DSYMUTIL := true
endif

ifdef TargetCommonOpts
  BUGPOINT_TOPTS := "-gcc-tool-args $(TargetCommonOpts)"
else
  BUGPOINT_TOPTS := ""
endif

FORCE:

ifeq ($(DISABLE_ASSERTIONS),1)
  ENABLE_ASSERTIONS := 0
else
  ENABLE_ASSERTIONS := 1
endif

LLVM_TOOL_DIR = $(LLVM_OBJ_ROOT)/$(BuildMode)/bin

#
# Keep LLVM substitution in place. We have added two substitution for locating
# project source and object roots (PROJ_SOURCE_DIR and PROJ_BINARY_DIR
# respectively). The ToolDir substitution has been modifier in order to look for
# the real LLVM ToolDir, instead of the one of this tool.
#


lit.site.cfg: FORCE
	@echo "Making OpenCRun 'lit.site.cfg' file..."
	@$(ECHOPATH) s=@LLVM_HOSTTRIPLE@=$(HOST_TRIPLE)=g > lit.tmp
	@$(ECHOPATH) s=@TARGET_TRIPLE@=$(TARGET_TRIPLE)=g >> lit.tmp
	@$(ECHOPATH) s=@LLVM_SOURCE_DIR@=$(LLVM_SRC_ROOT)=g >> lit.tmp
	@$(ECHOPATH) s=@LLVM_BINARY_DIR@=$(LLVM_OBJ_ROOT)=g >> lit.tmp
	@$(ECHOPATH) s=@LLVM_TOOLS_DIR@=$(LLVM_TOOL_DIR)=g >> lit.tmp
	@$(ECHOPATH) s=@SHLIBEXT@=$(SHLIBEXT)=g >> lit.tmp
	@$(ECHOPATH) s=@LTO_IS_ENABLED@=$(LTO_IS_ENABLED)=g >> lit.tmp
	@$(ECHOPATH) s=@HOST_OS@=$(HOST_OS)=g >> lit.tmp
	@$(ECHOPATH) s=@HOST_ARCH@=$(HOST_ARCH)=g >> lit.tmp
	@$(ECHOPATH) s=@PROJ_SOURCE_DIR@=$(PROJ_SRC_ROOT)=g >> lit.tmp
	@$(ECHOPATH) s=@PROJ_BINARY_DIR@=$(PROJ_OBJ_ROOT)=g >> lit.tmp
	@$(ECHOPATH) s=@PROJ_SHLIB_DIR@=$(SharedLibDir)=g >> lit.tmp
	@sed -f lit.tmp $(PROJ_SRC_DIR)/lit.site.cfg.in > $@
	@-rm -f lit.tmp
