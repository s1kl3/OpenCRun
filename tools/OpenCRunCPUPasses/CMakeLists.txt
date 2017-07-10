add_library(OpenCRunCPUPasses MODULE
  ForceLinking.cpp
  )

set_target_properties(OpenCRunCPUPasses PROPERTIES
  PREFIX "" SUFFIX ${LLVM_PLUGIN_EXT}
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  )

set(OPENCRUN_LIBS
  opencrunCPUPasses
  opencrunUtil
  )

include_directories(${PROJECT_SOURCE_DIR}/include)
target_link_libraries(OpenCRunCPUPasses PRIVATE ${OPENCRUN_LIBS})
