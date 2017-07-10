set(OPENCRUN_TARGET CPU)

add_library(opencrunCPUDevice
  AsyncCopyThread.cpp
  AsyncInternalCalls.cpp
  Command.cpp
  CPUCompiler.cpp
  CPUDevice.cpp
  CPUModuleInfo.cpp
  CPUThread.cpp
  ImageSupport.cpp
  Memory.cpp
  Mfence.cpp
  SynchronizationInternalCalls.cpp
  ThreadPool.cpp
  WorkItemInternalCalls.cpp
  )

set_target_properties(opencrunCPUDevice PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  )

add_subdirectory(Library)
add_subdirectory(Passes)

add_dependencies(opencrunCPUDevice Generate_Headers Generate_${OPENCRUN_TARGET}_Library)
