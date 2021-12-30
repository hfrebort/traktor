#!/bin/sh

#cmake -DCMAKE_LINKER=/path/to/linker -DCMAKE_CXX_LINK_EXECUTABLE="<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
#    -DCMAKE_LINKER="/usr/bin/lld"                       \
#    -DCMAKE_C_LINK_EXECUTABLE="/usr/bin/lld"            \
#    -DCMAKE_CXX_LINK_EXECUTABLE="/usr/bin/lld"          \

cmake \
    -DCMAKE_TOOLCHAIN_FILE="../platform/jetson.cmake"                   \
    -DCMAKE_C_FLAGS="-fuse-ld=lld"                                      \
    -DCMAKE_CXX_FLAGS="-fuse-ld=lld"                                    \
    -DCUDA_TOOLKIT_ROOT_DIR="/home/bee/dev/jetroot/usr/local/cuda-10.2" \
    -DCUDAToolkit_ROOT="/home/bee/dev/jetroot/usr/local/cuda-10.2"      \
    -S . -B buildJetson
