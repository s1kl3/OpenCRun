add_library(opencrunCore
  Command.cpp
  CommandQueue.cpp
  Context.cpp
  Device.cpp
  DeviceCompiler.cpp
  Event.cpp
  Kernel.cpp
  MemoryObject.cpp
  Platform.cpp
  Profiler.cpp
  Program.cpp
  Sampler.cpp
  )

set_target_properties(opencrunCore PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  )
