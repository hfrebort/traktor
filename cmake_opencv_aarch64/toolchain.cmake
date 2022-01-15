# Specify target environment
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify LLVM as cross-compilation toolchain
set(LLVM_ROOT "/usr")
set(CLANG_TARGET_TRIPLE aarch64-linux-gnu)

set(CMAKE_C_COMPILER ${LLVM_ROOT}/bin/clang)
set(CMAKE_C_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})

set(CMAKE_CXX_COMPILER ${LLVM_ROOT}/bin/clang++)
set(CMAKE_CXX_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})

set(CMAKE_ASM_COMPILER ${LLVM_ROOT}/bin/clang)
set(CMAKE_ASM_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Use LLVM's lld linker
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld")

# Don't run the linker on compiler check
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Specify sysroot of target platform
set(SYSROOT "home/bee/dev/pi_sysroot")
set(CMAKE_SYSROOT ${SYSROOT})
set(CMAKE_INSTALL_PREFIX "/usr")

# Specify pkg-config search path
# set(ENV{PKG_CONFIG_PATH} "${SYSROOT}/usr/lib/${TARGET}/pkgconfig")
# set(ENV{PKG_CONFIG_LIBDIR} "${SYSROOT}/usr/lib/${TARGET}/pkgconfig")
set(ENV{PKG_CONFIG_PATH} ${SYSROOT}/usr/lib/pkgconfig)
set(ENV{PKG_CONFIG_LIBDIR} ${SYSROOT}/usr/lib/pkgconfig)
set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})

# Specify cmake search path
set(CMAKE_FIND_ROOT_PATH ${SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)