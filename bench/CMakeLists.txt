function(add_opencrun_bench bench_dirname)
  include_directories(${PROJECT_SOURCE_DIR}/utils/OCLBench/include)

  add_executable(${bench_dirname} ${ARGN})

  target_link_libraries(${bench_dirname} PRIVATE
    OCLBench
    OpenCRun_Shared
    )

  # TODO: implement the ICD-based benchmarking framework and link with OpenCL
  # instead of OpenCRun.
  add_custom_command(
    TARGET ${bench_dirname} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cl
    ${CMAKE_CURRENT_BINARY_DIR}/
    )
endfunction()

replace_substring(CMAKE_CXX_FLAGS "-fno-rtti" "-frtti")
replace_substring(CMAKE_CXX_FLAGS "-fno-exceptions" "-fexceptions")
replace_substring(CMAKE_CXX_FLAGS "-Wcast-qual" "-Wno-cast-qual")

################################################################################

add_subdirectory(schoolbook-msquare)
add_subdirectory(blocked-msquare)
