include_directories(..)

add_library(opencrunCPUPasses
  AllPasses.cpp
  AutomaticLocalVariables.cpp
  GroupParallelStub.cpp
  )

set_target_properties(opencrunCPUPasses PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  )
