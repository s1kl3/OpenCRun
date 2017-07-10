add_library(OCLBench
 Benchmark.cpp
 BenchmarkPrinter.cpp
 DataTypes.cpp
 )

set_target_properties(OCLBench PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  )

include_directories(../include)

replace_substring(CMAKE_CXX_FLAGS "-fno-rtti" "-frtti")
replace_substring(CMAKE_CXX_FLAGS "-fno-exceptions" "-fexceptions")
replace_substring(CMAKE_CXX_FLAGS "-Wcast-qual" "-Wno-cast-qual")
