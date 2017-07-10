# Test runner infrastructure for OpenCRun (inspired by the Clang test runner
# infrastructure). This configures the OpenCRun test trees for use by Lit, and
# delegates to LLVM's lit test handlers.

################################################################################
# Functions inspired by AddLLVM.cmake module.

# This function provides an automatic way to 'configure'-like generate a file
# based on a set of common and custom variables, specifically targeting the
# variables needed for the 'lit.site.cfg' files. This function bundles the
# common variables that any Lit instance is likely to need, and custom
# variables can be passed in.
function(configure_lit_site_cfg input output)
  set(SHLIBEXT "${LTDL_SHLIB_EXT}")

  # Configuration-time: See Unit/lit.site.cfg.in
  if (CMAKE_CFG_INTDIR STREQUAL ".")
    set(LLVM_BUILD_MODE ".")
  else ()
    set(LLVM_BUILD_MODE "%(build_mode)s")
  endif ()

  # They below might not be the build tree but provided binary tree.
  set(LLVM_SOURCE_DIR ${LLVM_MAIN_SRC_DIR})
  set(LLVM_BINARY_DIR ${LLVM_BINARY_DIR})
  string(REPLACE ${CMAKE_CFG_INTDIR} ${LLVM_BUILD_MODE} LLVM_TOOLS_DIR ${LLVM_TOOLS_BINARY_DIR})
  string(REPLACE ${CMAKE_CFG_INTDIR} ${LLVM_BUILD_MODE} LLVM_LIBS_DIR  ${LLVM_LIBRARY_DIR})

  # SHLIBDIR points the build tree.
  string(REPLACE ${CMAKE_CFG_INTDIR} ${LLVM_BUILD_MODE} SHLIBDIR "${LLVM_SHLIB_OUTPUT_INTDIR}")

  set(PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE})
  # FIXME: "ENABLE_SHARED" doesn't make sense, since it is used just for
  # plugins. We may rename it.
  if(LLVM_ENABLE_PLUGINS)
    set(ENABLE_SHARED "1")
  else()
    set(ENABLE_SHARED "0")
  endif()

  set(HOST_OS ${CMAKE_SYSTEM_NAME})
  set(HOST_ARCH ${CMAKE_SYSTEM_PROCESSOR})

  set(HOST_CC "${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1}")
  set(HOST_CXX "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}")
  set(HOST_LDFLAGS "${CMAKE_EXE_LINKER_FLAGS}")

  configure_file(${input} ${output} @ONLY)
endfunction()

function(add_opencrun_testsuite target comment)
  cmake_parse_arguments(ARG "" "" "PARAMS;DEPENDS;ARGS" ${ARGN})

  # EXCLUDE_FROM_ALL excludes the test ${target} out of check-all.
  if(NOT EXCLUDE_FROM_ALL)
    # Register the testsuites, params and depends for the global check rule.
    set_property(GLOBAL APPEND PROPERTY LLVM_LIT_TESTSUITES ${ARG_UNPARSED_ARGUMENTS})
    set_property(GLOBAL APPEND PROPERTY LLVM_LIT_PARAMS ${ARG_PARAMS})
    set_property(GLOBAL APPEND PROPERTY LLVM_LIT_DEPENDS ${ARG_DEPENDS})
    set_property(GLOBAL APPEND PROPERTY LLVM_LIT_EXTRA_ARGS ${ARG_ARGS})
  endif()

  set(LIT_ARGS "${ARG_ARGS} ${LLVM_LIT_ARGS}")
  separate_arguments(LIT_ARGS)
  if (NOT CMAKE_CFG_INTDIR STREQUAL ".")
    list(APPEND LIT_ARGS --param build_mode=${CMAKE_CFG_INTDIR})
  endif ()
  if (LLVM_MAIN_SRC_DIR)
    set (LIT_COMMAND ${PYTHON_EXECUTABLE} ${LLVM_MAIN_SRC_DIR}/utils/lit/lit.py)
  else()
    find_program(LIT_COMMAND llvm-lit)
  endif ()
  list(APPEND LIT_COMMAND ${LIT_ARGS})
  foreach(param ${ARG_PARAMS})
    list(APPEND LIT_COMMAND --param ${param})
  endforeach()
  if (ARG_UNPARSED_ARGUMENTS)
    add_custom_target(${target}
      COMMAND ${LIT_COMMAND} ${ARG_UNPARSED_ARGUMENTS}
      COMMENT "${comment}"
      ${cmake_3_2_USES_TERMINAL}
      )
  else()
    add_custom_target(${target}
      COMMAND ${CMAKE_COMMAND} -E echo "${target} does nothing, no tools built.")
    message(STATUS "${target} does nothing.")
  endif()
  if (ARG_DEPENDS)
    add_dependencies(${target} ${ARG_DEPENDS})
  endif()

  # Tests should be excluded from "Build Solution".
  set_target_properties(${target} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD ON)
endfunction()

################################################################################

set(LLVM_MAIN_SRC_DIR ${LLVM_SRC_ROOT})
set(LLVM_BINARY_DIR ${LLVM_OBJ_ROOT})
set(LLVM_TOOLS_BINARY_DIR "${LLVM_OBJ_ROOT}/bin")
set(LTDL_SHLIB_EXT ${CMAKE_SHARED_LIBRARY_SUFFIX})

set(PROJ_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(PROJ_BINARY_DIR ${PROJECT_BINARY_DIR})
set(PROJ_SHLIB_DIR ${PROJECT_SHLIB_DIR})

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  )

option(OPENCRUN_TEST_USE_VG "Run OpenCRun tests under Valgrind" OFF)
if(OPENCRUN_TEST_USE_VG)
  set(OPENCRUN_TEST_EXTRA_ARGS ${CLANG_TEST_EXTRA_ARGS} "--vg")
endif ()

set(OPENCRUN_TEST_DEPENDS
  opt
  FileCheck
  )

set(OPENCRUN_TEST_PARAMS
  opencrun_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  )

add_opencrun_testsuite(check-opencrun "Running the OpenCRun regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  #LIT ${LLVM_LIT}
  PARAMS ${OPENCRUN_TEST_PARAMS}
  DEPENDS ${OPENCRUN_TEST_DEPS}
  ARGS ${OPENCRUN_TEST_EXTRA_ARGS}
  )
set_target_properties(check-opencrun PROPERTIES FOLDER "OpenCRun tests")

# Add a legacy target spelling: opencrun-test
add_custom_target(opencrun-test)
add_dependencies(opencrun-test check-opencrun)
set_target_properties(opencrun-test PROPERTIES FOLDER "OpenCRun tests")
