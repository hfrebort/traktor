set(CMAKE_CROSSCOMPILING TRUE)
#SET(CMAKE_SYSTEM_NAME Generic)
#set(CMAKE_SYSTEM_PROCESSOR arm)

#clang --target=aarch64-linux-gnu --sysroot=/home/bee/dev/jetroot -fuse-ld=lld -O3

# Clang target triple
SET(TARGET aarch64-linux-gnu)
set(CMAKE_SYSROOT "/home/bee/dev/jetroot")

SET(CMAKE_C_COMPILER_TARGET ${TARGET})
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER_TARGET ${TARGET})
SET(CMAKE_CXX_COMPILER clang++)
SET(CMAKE_ASM_COMPILER_TARGET ${TARGET})
SET(CMAKE_ASM_COMPILER clang)

#SET(CUDA_TOOLKIT_ROOT_DIR "/home/bee/dev/jetroot/usr/local/cuda-10.2")
