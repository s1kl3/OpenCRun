add_library(opencrunPasses
  AllPasses.cpp
  AggressiveInliner.cpp
  FootprintEstimate.cpp
  )

set_target_properties(opencrunPasses PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  )
