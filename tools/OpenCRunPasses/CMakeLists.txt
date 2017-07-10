add_library(OpenCRunPasses MODULE
  ForceLinking.cpp
  )

set_target_properties(OpenCRunPasses PROPERTIES
  PREFIX "" SUFFIX ${LLVM_PLUGIN_EXT}
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  )

set(OPENCRUN_LIBS
  opencrunPasses
  opencrunUtil
  )

include_directories(${PROJECT_SOURCE_DIR}/include)
target_link_libraries(OpenCRunPasses PRIVATE ${OPENCRUN_LIBS})
