# Name of C compiler.
set(CMAKE_C_COMPILER   "/home/bee/cross/aarch64-linux-musl-cross/bin/aarch64-linux-musl-gcc")
set(CMAKE_CXX_COMPILER "/home/bee/cross/aarch64-linux-musl-cross/bin/aarch64-linux-musl-g++")

set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(GCC_COMPILER_VERSION ""         CACHE STRING "GCC Compiler version")
set(GNU_MACHINE "aarch64-linux-gnu" CACHE STRING "GNU compiler triple")
include("${CMAKE_CURRENT_LIST_DIR}/arm.toolchain.cmake")
