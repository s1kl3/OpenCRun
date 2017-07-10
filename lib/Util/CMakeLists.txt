add_library(opencrunUtil
  BuiltinInfo.cpp
  DimensionInfo.cpp
  LLVMCodeGenAction.cpp
  ModuleInfo.cpp
  OpenCLTypeSystem.cpp
  PassOptions.cpp
  Table.cpp
  )

set_target_properties(opencrunUtil PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  )
