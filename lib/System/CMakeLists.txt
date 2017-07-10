add_library(opencrunSystem
  Env.cpp
  Hardware.cpp
  Monitor.cpp
  OS.cpp
  Thread.cpp
  Time.cpp
  )

set_target_properties(opencrunSystem PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  )
