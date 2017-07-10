add_opencrun_unittest(APITests
  Context.cpp
  Kernel.cpp
  )

if(NOT CMAKE_CXX_FLAGS STREQUAL "")
  list(FILTER CMAKE_CXX_FLAGS EXCLUDE REGEX "-fno-exceptions")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-cast-qual")
endif(NOT CMAKE_CXX_FLAGS STREQUAL "")
