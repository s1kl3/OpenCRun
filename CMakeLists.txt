cmake_minimum_required(VERSION 3.2)
project(OpenCRun)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using ClangConfig.cmake in: ${Clang_DIR}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_DIR}" "${Clang_DIR}")

if((NOT LLVM_SRC_ROOT) OR (NOT LLVM_OBJ_ROOT))
  message(FATAL_ERROR "LLVM_SRC_ROOT and LLVM_OBJ_ROOT variables not set!")
endif()

message(STATUS "LLVM source root: ${LLVM_SRC_ROOT}")
message(STATUS "LLVM object root: ${LLVM_OBJ_ROOT}")

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_OBJ_ROOT}/lib)

add_definitions(${LLVM_DEFINITIONS})

################################################################################
# Utility functions and macros
################################################################################

# Set variables to llvm-config returned values.
macro(llvm_config OUT_VAR)
  execute_process(
    COMMAND "${LLVM_CONFIG_CMD}" ${ARGN}
    OUTPUT_VARIABLE ${OUT_VAR}
    RESULT_VARIABLE LLVM_CONFIG_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  if(LLVM_CONFIG_RESULT)
    message(SEND_ERROR "Error executing command: 'llvm-config ${ARGN}'")
  endif()
endmacro(llvm_config)

# Replace substring from a CMake variable, appending the replacement substring
# if there are no occurences of the substring.
macro(replace_substring variable substr repstr)
  string(REPLACE "${substr}" "${repstr}" ${variable} "${${variable}}")
  string(REGEX REPLACE "\\+\\+" "\\\\+\\\\+" repstr_escpd ${repstr})
  if(NOT(${variable} MATCHES ${repstr_escpd}))
    set(${variable} "${${variable}} ${repstr}")
  endif()
endmacro()

################################################################################
# Main settings for C++ compiler and static linker
################################################################################

find_program(LLVM_CONFIG_CMD NAMES llvm-config
  PATHS ${LLVM_OBJ_ROOT}/bin
  NO_DEFAULT_PATH
  NO_CMAKE_ENVIRONMENT_PATH
  NO_CMAKE_PATH
  NO_SYSTEM_ENVIRONMENT_PATH
  NO_CMAKE_SYSTEM_PATH
  )

if(NOT LLVM_CONFIG_CMD)
  message(FATAL_ERROR "llvm-config not found in the LLVM object root!")
endif()

message(STATUS "Found llvm-config: ${LLVM_CONFIG_CMD}")

llvm_config(LLVM_CPPFLAGS --cppflags)
llvm_config(LLVM_CFLAGS --cflags)
llvm_config(LLVM_CXXFLAGS --cxxflags)
llvm_config(LLVM_LDFLAGS --ldflags)
llvm_config(LLVM_LIBRARY_DIR --libdir)
llvm_config(LLVM_TOOLS_BINARY_DIR --bindir)
llvm_config(LLVM_SYSTEM_LIBS --system-libs)
llvm_config(LLVM_BUILD_MODE --build-mode)
llvm_config(LLVM_ASSERTION_MODE --assertion-mode)
llvm_config(LLVM_HAS_RTTI --has-rtti)

set(LLVM_CXXFLAGS "${LLVM_CXXFLAGS}")
set(CMAKE_CXX_FLAGS "${LLVM_CXXFLAGS}")
set(CMAKE_BUILD_TYPE "${LLVM_BUILD_MODE}")

# Auto-detect the target architecture (Warning! Resulting binaries may not
# work on different targets).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed")

# Instead of C++11 standard, the GNU C++11 standard is required to avoid
# errors when building unit tests.
replace_substring(CMAKE_CXX_FLAGS "-std=c++11" "-std=gnu++11")

# Suppress some warnings. These warnings should be checked carefully later.
replace_substring(CMAKE_CXX_FLAGS "-Wignored-attributes" "-Wno-ignored-attributes")
replace_substring(CMAKE_CXX_FLAGS "-Wstrict-aliasing" "-Wno-strict-aliasing")

message(STATUS "C++ compiler flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Binaries, headers and libraries to be installed are collected all together
# in a seprated tree under the build directory.
set(PROJECT_TREE "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(PROJECT_TOOLS_DIR "${PROJECT_TREE}/bin")
set(PROJECT_HEADERS_DIR "${PROJECT_TREE}/include") 
set(PROJECT_SHLIB_DIR "${PROJECT_TREE}/lib")
set(PROJECT_OCL_HEADERS_DIR "${PROJECT_SHLIB_DIR}/opencrun/include") 
file(MAKE_DIRECTORY
  ${PROJECT_TOOLS_DIR}
  ${PROJECT_HEADERS_DIR}
  ${PROJECT_SHLIB_DIR}
  ${PROJECT_OCL_HEADERS_DIR}
  )

# Finally we'll move the tree as a whole.
install(
  DIRECTORY "${PROJECT_TREE}/"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  USE_SOURCE_PERMISSIONS
  )

################################################################################
# Python interpreter
################################################################################

# Verify that we can find a Python 2 interpreter.  Python 3 is unsupported.
# FIXME: We should support systems with only Python 3, but that requires work
# on LLDB.
set(Python_ADDITIONAL_VERSIONS 2.7)
include(FindPythonInterp)
if( NOT PYTHONINTERP_FOUND )
  message(FATAL_ERROR
"Unable to find Python interpreter, required for builds and testing.

Please install Python or specify the PYTHON_EXECUTABLE CMake variable.")
endif()

if( ${PYTHON_VERSION_STRING} VERSION_LESS 2.7 )
  message(FATAL_ERROR "Python 2.7 or newer is required")
endif()

################################################################################
# Third-party libraries
################################################################################

find_library(HWLOC_LIBRARY NAMES hwloc)

if(NOT HWLOC_LIBRARY)
  message(FATAL_ERROR "Portable Hardware Locality (hwloc) library not found!")
else()
  message(STATUS "Portable Hardware Locality (hwloc) library: ${HWLOC_LIBRARY}")
endif()

################################################################################

add_subdirectory(include)
add_subdirectory(utils)
add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(test)
add_subdirectory(unittests)
add_subdirectory(bench)
