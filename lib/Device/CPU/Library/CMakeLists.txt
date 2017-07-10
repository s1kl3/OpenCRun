set(CLANG_SRC_ROOT ${LLVM_SRC_ROOT}/tools/clang)
set(CLANG_OBJ_ROOT ${LLVM_OBJ_ROOT}/tools/clang)


include_directories(${CLANG_SRC_ROOT}/include ${CLANG_OBJ_ROOT}/include)

################################################################################
# Tools Setup
################################################################################

find_program(LLVMCC NAMES clang
  PATHS ${LLVM_OBJ_ROOT}/bin
  NO_DEFAULT_PATH
  NO_CMAKE_ENVIRONMENT_PATH
  NO_CMAKE_PATH
  NO_SYSTEM_ENVIRONMENT_PATH
  NO_CMAKE_SYSTEM_PATH
  )

find_program(LLVMCXX NAMES clang++
  PATHS ${LLVM_OBJ_ROOT}/bin
  NO_DEFAULT_PATH
  NO_CMAKE_ENVIRONMENT_PATH
  NO_CMAKE_PATH
  NO_SYSTEM_ENVIRONMENT_PATH
  NO_CMAKE_SYSTEM_PATH
  )

find_program(LLVMLINK NAMES llvm-link
  PATHS ${LLVM_OBJ_ROOT}/bin
  NO_DEFAULT_PATH
  NO_CMAKE_ENVIRONMENT_PATH
  NO_CMAKE_PATH
  NO_SYSTEM_ENVIRONMENT_PATH
  NO_CMAKE_SYSTEM_PATH
  )

find_program(LLVMAS NAMES llvm-as
  PATHS ${LLVM_OBJ_ROOT}/bin
  NO_DEFAULT_PATH
  NO_CMAKE_ENVIRONMENT_PATH
  NO_CMAKE_PATH
  NO_SYSTEM_ENVIRONMENT_PATH
  NO_CMAKE_SYSTEM_PATH
  )

find_program(LLVMOPT NAMES opt
  PATHS ${LLVM_OBJ_ROOT}/bin
  NO_DEFAULT_PATH
  NO_CMAKE_ENVIRONMENT_PATH
  NO_CMAKE_PATH
  NO_SYSTEM_ENVIRONMENT_PATH
  NO_CMAKE_SYSTEM_PATH
  )

if(NOT (LLVMCC AND LLVMCXX))
  message(FATAL_ERROR "Some Clang binaries not found at the LLVM object root!")
else()
  message(STATUS "Clang frontend: ${LLVMCC}")
  message(STATUS "Clang++ frontend: ${LLVMCXX}")
  message(STATUS "LLVM linker: ${LLVMLINK}")
  message(STATUS "LLVM assembler: ${LLVMAS}")
  message(STATUS "LLVM optimizer: ${LLVMOPT}")
endif()

################################################################################
# Automatic generation of OpenCL C built-in library code
################################################################################

set(OCLGen ${OCLGen} -I${CMAKE_CURRENT_SOURCE_DIR})

# Target-dependent header files.
add_custom_command(
  OUTPUT ${HeadersDir}/oclconst.${OPENCRUN_TARGET}.h
  COMMAND ${OCLGen} -gen-ocl-constants-target
                    -target ${OPENCRUN_TARGET}
                    -o ${HeadersDir}/oclconst.${OPENCRUN_TARGET}.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/${OPENCRUN_TARGET}.td
  DEPENDS ${OCLGenLibrary}
  COMMENT "Generating OCL target constants definition for '${OPENCRUN_TARGET}' with oclgen"
  )

add_custom_command(
  OUTPUT ${HeadersDir}/ocltype.${OPENCRUN_TARGET}.h
  COMMAND ${OCLGen} -gen-ocl-types-target
                    -target ${OPENCRUN_TARGET}
                    -o ${HeadersDir}/ocltype.${OPENCRUN_TARGET}.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/${OPENCRUN_TARGET}.td
  DEPENDS ${OCLGenLibrary}
  COMMENT "Generating OCL target types definition for '${OPENCRUN_TARGET}' with oclgen"
  )

add_custom_command(
  OUTPUT ${HeadersDir}/ocltarget.${OPENCRUN_TARGET}.h
  COMMAND ${OCLGen} -gen-ocl-target-defs
                    -target ${OPENCRUN_TARGET}
                    -o ${HeadersDir}/ocltarget.${OPENCRUN_TARGET}.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/${OPENCRUN_TARGET}.td
  DEPENDS ${OCLGenLibrary}
  COMMENT "Generating OCL target definitions for '${OPENCRUN_TARGET}' with oclgen"
  )

# Definitions of library functions.
add_custom_command(
  OUTPUT Builtins.${OPENCRUN_TARGET}.inc
  COMMAND ${OCLGen} -gen-ocl-builtin-impls
                    -target ${OPENCRUN_TARGET}
                    -o Builtins.${OPENCRUN_TARGET}.inc
                    ${CMAKE_CURRENT_SOURCE_DIR}/${OPENCRUN_TARGET}.td
  DEPENDS ${OCLGenLibrary}
  COMMENT "Generating OCL builtins library implementation for '${OPENCRUN_TARGET}' with oclgen"
  )

add_custom_target(Generate_${OPENCRUN_TARGET}_Library_Src DEPENDS
  ${HeadersDir}/oclconst.${OPENCRUN_TARGET}.h
  ${HeadersDir}/ocltype.${OPENCRUN_TARGET}.h
  ${HeadersDir}/ocltarget.${OPENCRUN_TARGET}.h
  Builtins.${OPENCRUN_TARGET}.inc
  )

add_dependencies(Generate_${OPENCRUN_TARGET}_Library_Src Generate_Headers)

################################################################################
# OpenCL to Bitcode compilation
################################################################################

set(ModuleName opencrun${OPENCRUN_TARGET}Lib)
set(Module ${PROJECT_SHLIB_DIR}/${ModuleName}.bc)
set(ModuleDestDir ${CMAKE_INSTALL_PREFIX}/lib)
set(DestModule ${ModuleDestDir}/${ModuleName})

set(BCCompile.CL ${LLVMCXX} -g0 -march=native
  -pedantic
  -Wall -Wgnu -Wwrite-strings
  -Wno-empty-translation-unit -Wno-long-long -Wno-unused-parameter
  -DOPENCRUN_LIB_IMPL
  -I${CMAKE_CURRENT_BINARY_DIR}
  -I${PROJECT_SOURCE_DIR}/include
  -I${HeadersDir}
  -include ${HeadersDir}/ocltarget.${OPENCRUN_TARGET}.h
  )
set(LinkModule ${LLVMLINK})
set(BuildMode ${CMAKE_BUILD_TYPE})

set(CL_SourceFiles
  Async.cl
  Atomic.cl
  Common.cl
  Convert.cl
  Geometric.cl
  Image.cl
  Integer.cl
  Math.cl
  Mfence.cl
  Reinterpret.cl
  Relational.cl
  Synchronization.cl
  Vloadstore.cl
  Vmisc.cl
  WorkItem.cl
  )

foreach(CL_Src ${CL_SourceFiles})
  string(REGEX REPLACE "\\.cl$" "" CL_Src_BaseName ${CL_Src})

  if(DEBUG_RUNTIME)
    set(LLVMOPT_FLAGS "-O2")
  else()
    set(LLVMOPT_FLAGS "-O2" "-strip-debug")
  endif(DEBUG_RUNTIME)

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CL_Src_BaseName}.ll
    COMMAND ${BCCompile.CL}
      ${CMAKE_CURRENT_SOURCE_DIR}/${CL_Src}
      -o ${CMAKE_CURRENT_BINARY_DIR}/${CL_Src_BaseName}.ll
      -S -emit-llvm
    DEPENDS ${CL_Src}
    COMMENT "Compiling ${CL_Src} for ${BuildMode} build (bitcode)"
    )

  set(IR_SourceFiles ${IR_SourceFiles} ${CMAKE_CURRENT_BINARY_DIR}/${CL_Src_BaseName}.ll)

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CL_Src_BaseName}.bc
    COMMAND ${LLVMAS}
      ${CMAKE_CURRENT_BINARY_DIR}/${CL_Src_BaseName}.ll
      -f
      -o ${CMAKE_CURRENT_BINARY_DIR}/${CL_Src_BaseName}.bc
    COMMAND ${LLVMOPT}
      ${CMAKE_CURRENT_BINARY_DIR}/${CL_Src_BaseName}.bc
      ${LLVMOPT_FLAGS}
      -o=${CMAKE_CURRENT_BINARY_DIR}/${CL_Src_BaseName}.bc
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${CL_Src_BaseName}.ll
    COMMENT "Compiling ${CL_Src_BaseName}.ll for ${BuildMode} build (bitcode)"
    )

  set(BC_Files ${BC_Files} ${CMAKE_CURRENT_BINARY_DIR}/${CL_Src_BaseName}.bc)
endforeach()

add_custom_target(Generate_${OPENCRUN_TARGET}_Library_BC DEPENDS ${IR_SourceFiles} ${BC_Files})

add_dependencies(Generate_${OPENCRUN_TARGET}_Library_BC Generate_${OPENCRUN_TARGET}_Library_Src)

################################################################################
# Link Bitcode files
################################################################################

add_custom_command(
  OUTPUT ${Module}
  COMMAND ${LinkModule} -o ${Module} ${BC_Files}
  COMMENT "Building ${BuildMode} Bitcode Module ${ModuleName}.bc"
  )

add_custom_target(Generate_${OPENCRUN_TARGET}_Library DEPENDS ${Module})

add_dependencies(Generate_${OPENCRUN_TARGET}_Library
  Generate_${OPENCRUN_TARGET}_Library_BC
  )
