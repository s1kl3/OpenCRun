include_directories(${PROJECT_SOURCE_DIR}/include)

set(OCLGen ${PROJECT_TOOLS_DIR}/oclgen -I${PROJECT_SOURCE_DIR}/include)
set(CommonLibraryDir ${PROJECT_SOURCE_DIR}/include/opencrun/Device/Library)
set(HeadersDir ${PROJECT_OCL_HEADERS_DIR})

file(GLOB_RECURSE OCLGenLibrary ${CommonLibraryDir}/*.td)

################################################################################

add_subdirectory(Headers)
add_subdirectory(System)
add_subdirectory(Util)
add_subdirectory(Passes)
add_subdirectory(Core)
add_subdirectory(Device)
