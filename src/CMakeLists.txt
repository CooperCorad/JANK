file(GLOB compiler_src CONFIGURE_DEPENDS "*.cpp")

add_executable(JANK ${compiler_src})

include_directories("../include" "${LLVM_INCLUDE_DIR}")

llvm_map_components_to_libnames(llvm_libs core)

target_link_libraries(JANK ${llvm_libs})