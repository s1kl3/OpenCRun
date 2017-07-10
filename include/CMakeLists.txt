# Copy all files to be installed to the temporary location.
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION ${PROJECT_HEADERS_DIR}
  FILES_MATCHING REGEX ".+\\.(h|hpp|inc|td)$"
  )
