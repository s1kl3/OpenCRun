string(REGEX REPLACE "-fno-rtti|-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_executable(platform_query
  platform-query.cpp
  )

replace_substring(CMAKE_CXX_FLAGS "-fno-rtti" "-frtti")
replace_substring(CMAKE_CXX_FLAGS "-fno-exceptions" "-fexceptions")
replace_substring(CMAKE_CXX_FLAGS "-Wcast-qual" "-Wno-cast-qual")

target_link_libraries(platform_query PRIVATE OpenCRun_Shared)

set_target_properties(platform_query PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_TOOLS_DIR}
  )
