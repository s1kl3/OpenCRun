add_custom_command(
  OUTPUT ${HeadersDir}/oclconst.h
  COMMAND ${OCLGen} -gen-ocl-constants -o ${HeadersDir}/oclconst.h ${CommonLibraryDir}/OpenCL.td 
  DEPENDS ${OCLGenLibrary}
  COMMENT "Generating OCL constants definition with oclgen..."
  )

add_custom_command(
  OUTPUT ${HeadersDir}/ocltype.h
  COMMAND ${OCLGen} -gen-ocl-types -o ${HeadersDir}/ocltype.h ${CommonLibraryDir}/OpenCL.td
  DEPENDS ${OCLGenLibrary}
  COMMENT "Generating OCL builtins library definition with oclgen..."
  )

add_custom_command(
  OUTPUT ${HeadersDir}/oclbuiltin.h
  COMMAND ${OCLGen} -gen-ocl-builtin-defs -o ${HeadersDir}/oclbuiltin.h ${CommonLibraryDir}/OpenCL.td
  DEPENDS ${OCLGenLibrary}
  COMMENT "Generating OCL types definition with oclgen..."
  )

add_custom_target(Generate_Headers DEPENDS
  ${HeadersDir}/oclconst.h
  ${HeadersDir}/ocltype.h
  ${HeadersDir}/oclbuiltin.h
  )

add_dependencies(Generate_Headers oclgen)
