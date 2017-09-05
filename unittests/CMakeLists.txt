add_custom_target(OpenCRunUnitTests)
set_target_properties(OpenCRunUnitTests PROPERTIES FOLDER "OpenCRun tests")

string(REPLACE "-Wcast-qual" "-Wno-cast-qual" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

################################################################################
# Custom function for adding unit tests
################################################################################

function(add_opencrun_unittest test_name)
  add_executable(${test_name} ${ARGN})

  target_include_directories(${test_name}
    PRIVATE "${LLVM_SRC_ROOT}/utils/unittest/googletest/include"
    PRIVATE "${LLVM_SRC_ROOT}/utils/unittest/googlemock/include"
    )

  target_link_libraries(${test_name} PRIVATE
    OpenCRun_Shared
    gtest_main
    gtest
    LLVMSupport # gtest needs it for raw_ostream.
    )

  add_dependencies(OpenCRunUnitTests ${test_name})

  get_target_property(test_suite_folder OpenCRunUnitTests FOLDER)

  if (NOT ${test_suite_folder} STREQUAL "NOTFOUND")
    set_property(TARGET ${test_name} PROPERTY FOLDER "${test_suite_folder}")
  endif ()
endfunction()

################################################################################

add_subdirectory(Skel)
add_subdirectory(Runtime)
add_subdirectory(Library)
add_subdirectory(API)
