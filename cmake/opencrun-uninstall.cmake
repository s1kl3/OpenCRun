# Manifest fill containing all installed file names.
set(MANIFEST "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")

if(NOT EXISTS ${MANIFEST})
  message(STATUS "Cannot find install manifest: ${MANIFEST}")
  return()
endif()

file(STRINGS ${MANIFEST} INSTALLED_FILES)
foreach(FILE ${INSTALLED_FILES})
  if(NOT EXISTS ${FILE})
    message(STATUS "File not found: ${FILE}")
  else()
    message(STATUS "Removing file: ${FILE}")
    
    exec_program(
      ${CMAKE_COMMAND} ARGS "-E remove ${FILE}"
      OUTPUT_VARIABLE CMD_OUT
      RETURN_VALUE CMD_RET
    )

    if(NOT ${CMD_RET} STREQUAL 0)
      message(FATAL_ERROR "Failed to remove file: ${FILE}")
    endif()
      
    exec_program(
      ${CMAKE_COMMAND} ARGS "-E remove ${MANIFEST}"
      OUTPUT_VARIABLE CMD_OUT
      RETURN_VALUE CMD_RET
    )

    if(NOT ${CMD_RET} STREQUAL 0)
      message(FATAL_ERROR "Failed to remove install manifest: ${MANIFEST}")
    endif()

  endif()
endforeach()
