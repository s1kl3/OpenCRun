set(LIBRARY_FILES
  CommandQueue.cpp
  Context.cpp
  Device.cpp
  EnqueueCommand.cpp
  Event.cpp
  Extension.cpp
  FlushFinish.cpp
  ICDExtension.cpp
  Kernel.cpp
  MemoryObject.cpp
  Platform.cpp
  Profiling.cpp
  Program.cpp
  Sampler.cpp
  )

add_library(OpenCRun_Shared SHARED ${LIBRARY_FILES})
add_library(OpenCRun_Static STATIC ${LIBRARY_FILES})

set_target_properties(OpenCRun_Shared PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  OUTPUT_NAME OpenCRun CLEAN_DIRECT_OUTPUT 1
  )
set_target_properties(OpenCRun_Static PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  OUTPUT_NAME OpenCRun CLEAN_DIRECT_OUTPUT 1
  )

llvm_map_components_to_libnames(LLVM_LIBS
  x86
  x86codegen
  orcjit
  runtimedyld
  mcjit
  asmparser
  bitreader
  bitwriter
  instrumentation
  codegen
  ipo
  selectiondag
  irreader
  option
  profiledata
  linker
  )

set(CLANG_LIBS
  clangFrontend
  clangDriver
  clangSerialization
  clangCodeGen
  clangParse
  clangSema
  clangAnalysis
  clangAST
  clangLex
  clangBasic
  clangEdit
  )

set(OPENCRUN_LIBS
  opencrunCore
  opencrunCPUDevice
  opencrunCPUPasses
  opencrunPasses
  opencrunUtil
  opencrunSystem
  )

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(OpenCRun_Shared
  PRIVATE ${OPENCRUN_LIBS} ${HWLOC_LIBRARY} ${CLANG_LIBS} ${LLVM_LIBS} ${LLVM_SYSTEM_LIBS}
  )

target_link_libraries(OpenCRun_Static
  PRIVATE ${OPENCRUN_LIBS} ${HWLOC_LIBRARY} ${CLANG_LIBS} ${LLVM_LIBS} ${LLVM_SYSTEM_LIBS}
  )
