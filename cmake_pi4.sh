#!/bin/sh

#cmake -DCMAKE_LINKER=/path/to/linker -DCMAKE_CXX_LINK_EXECUTABLE="<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
#    -DCMAKE_LINKER="/usr/bin/lld"                       \
#    -DCMAKE_C_LINK_EXECUTABLE="/usr/bin/lld"            \
#    -DCMAKE_CXX_LINK_EXECUTABLE="/usr/bin/lld"          \

rm -rf ./build_pi/*

#-DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld"                \

cmake \
    -DCMAKE_BUILD_TYPE=Release                                          \
    -DCMAKE_TOOLCHAIN_FILE="../platform/raspi4.cmake"                   \
    -DCMAKE_C_FLAGS="-Wno-unused-command-line-argument -fuse-ld=lld"       \
    -DCMAKE_CXX_FLAGS="-Wno-unused-command-line-argument -fuse-ld=lld"     \
    -S . -B ./build_pi