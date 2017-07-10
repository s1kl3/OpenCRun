add_executable(oclgen
  OCLBuiltin.cpp
  OCLBuiltinDefEmitter.cpp
  OCLBuiltinImplEmitter.cpp
  OCLConstant.cpp
  OCLConstantEmitter.cpp
  OCLEmitterUtils.cpp
  OCLGen.cpp
  OCLTarget.cpp
  OCLTargetDefEmitter.cpp
  OCLType.cpp
  OCLTypeDefEmitter.cpp
  )

include_directories(.)

llvm_map_components_to_libnames(llvm_libs tablegen support)
target_link_libraries(oclgen ${llvm_libs})

set_target_properties(oclgen PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_TOOLS_DIR}
  )
