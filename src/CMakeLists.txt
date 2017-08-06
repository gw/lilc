# See http://llvm.org/docs/CMake.html#embedding-llvm-in-your-project

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Found LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Shouldn't need to pull in all of LLVM but I don't want to figure out
# which specific libs I need just yet
llvm_map_components_to_libnames(llvm_libs all)
# x86codegen implements LLVMInitializeX86Target, as called by
# LLVMInitializeNativeTarget
# llvm_map_components_to_libnames(llvm_libs core target x86codegen)
# message(STATUS "LLVM LIBS: ${llvm_libs}")

add_library(LILC_CORE ast.c ast.h codegen.c codegen.h lex.c lex.h parse.c parse.h token.c token.h util.c util.h)

include_directories(../lib)
target_link_libraries(LILC_CORE CFU ${llvm_libs})