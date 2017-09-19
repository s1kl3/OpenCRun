add_library(OCLBench
 Benchmark.cpp
 BenchmarkPrinter.cpp
 DataTypes.cpp
 )

replace_substring(OPENCRUN_CXXFLAGS "-fno-rtti" "-frtti")
replace_substring(OPENCRUN_CXXFLAGS "-fno-exceptions" "-fexceptions")
replace_substring(OPENCRUN_CXXFLAGS "-Wcast-qual" "-Wno-cast-qual")

set_target_compile_options(OCLBench OPENCRUN_CXXFLAGS)

include_directories(../include)

set_target_properties(OCLBench PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SHLIB_DIR}
  )
