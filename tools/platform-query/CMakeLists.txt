add_executable(platform_query
  platform-query.cpp
  )

replace_substring(OPENCRUN_CXXFLAGS "-fno-rtti" "-frtti")
replace_substring(OPENCRUN_CXXFLAGS "-fno-exceptions" "-fexceptions")
replace_substring(OPENCRUN_CXXFLAGS "-Wcast-qual" "-Wno-cast-qual")

set_target_compile_options(platform_query OPENCRUN_CXXFLAGS)

target_link_libraries(platform_query PRIVATE OpenCRun_Shared)

set_target_properties(platform_query PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_TOOLS_DIR}
  )
