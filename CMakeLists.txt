cmake_minimum_required(VERSION 3.0.0)

#link_directories          ("/home/bee/cross/opencv/build/lib")

set(EXECUTABLE traktor)

set(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_CROSSCOMPILING   TRUE)

# Name of C compiler.
#set(CMAKE_C_COMPILER   "/usr/bin/aarch64-linux-gnu-gcc")
#set(CMAKE_CXX_COMPILER "/usr/bin/aarch64-linux-gnu-g++")
set(CMAKE_C_COMPILER   "/home/bee/cross/aarch64-linux-musl-cross/bin/aarch64-linux-musl-gcc")
set(CMAKE_CXX_COMPILER "/home/bee/cross/aarch64-linux-musl-cross/bin/aarch64-linux-musl-g++")

# Where to look for the target environment. (More paths can be added here)
set(CMAKE_FIND_ROOT_PATH "/usr/aarch64-linux-gnu")
set(CMAKE_INCLUDE_PATH   "/usr/include/aarch64-linux-gnu")
set(CMAKE_LIBRARY_PATH   "/usr/lib/aarch64-linux-gnu")
set(CMAKE_PROGRAM_PATH   "/usr/bin/aarch64-linux-gnu")

project(${EXECUTABLE} VERSION 0.1.0)

include(CTest)
enable_testing()

set(SOURCES 
    src/main.cpp
    src/thread_send_jpeg.cpp
    src/webserver.cpp
    src/stats.h
    src/thread_detect.cpp
    src/shared.h
    src/thread_camera.cpp
    src/thread_stats
)

add_executable(${EXECUTABLE} ${SOURCES})

target_precompile_headers(${EXECUTABLE} PRIVATE
  <signal.h>
  <sys/stat.h>
  <stdio.h>
  <iostream>
  <functional>
  <memory>
  <condition_variable>
  <thread>
  <atomic>
  <cstddef>
  <chrono>
  <opencv2/opencv.hpp>
  [["httplib.h"]]
)

target_compile_features(${EXECUTABLE} PRIVATE cxx_std_17)

#target_include_directories(traktor_cpp_test PRIVATE "./deps/boost_1_76_0")
target_include_directories(${EXECUTABLE} PRIVATE "./deps/cpp-httplib")

#add_library(pistache SHARED IMPORTED) # or STATIC instead of SHARED
#set_target_properties(pistache PROPERTIES
#  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/deps/pistache/lib/x86_64-linux-gnu/libpistache.so.0.0.002"
#)
#target_link_libraries(${EXECUTABLE} PRIVATE pistache)

message(STATUS CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${EXECUTABLE} PRIVATE Threads::Threads)
target_link_libraries(${EXECUTABLE} PRIVATE -static)
#target_link_libraries(${EXECUTABLE} PRIVATE -static-libgcc -static-libstdc++)

set (OPENCV_STATIC_LIBS "/home/bee/cross/opencv/build/lib")

#target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_core.a)
#target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_videoio.a)
#target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_imgcodecs.a)

target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_calib3d.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_core.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_features2d.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_flann.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_highgui.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_imgcodecs.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_imgproc.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_ml.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_objdetect.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_photo.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_ts.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_video.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_videoio.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_LIBS}/libopencv_world.a)

set (OPENCV_STATIC_3RD_LIBS "/home/bee/cross/opencv/build/3rdparty/lib")

target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_3RD_LIBS}/libade.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_3RD_LIBS}/libittnotify.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_3RD_LIBS}/liblibjpeg-turbo.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_3RD_LIBS}/liblibopenjp2.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_3RD_LIBS}/liblibpng.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_3RD_LIBS}/liblibprotobuf.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_3RD_LIBS}/liblibtiff.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_3RD_LIBS}/liblibwebp.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_3RD_LIBS}/libquirc.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_3RD_LIBS}/libtegra_hal.a)
target_link_libraries(${EXECUTABLE} PRIVATE ${OPENCV_STATIC_3RD_LIBS}/libzlib.a)


# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
#find_package(OpenCV REQUIRED)

set(OpenCV_LIBS         "opencv_core;opencv_videoio;opencv_imgcodecs")
set(OpenCV_INCLUDE_DIRS "/usr/include/opencv4")

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    config:       ${OpenCV_DIR}")
message(STATUS "    version:      ${OpenCV_VERSION}")
message(STATUS "    libraries:    ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# Link your application with OpenCV libraries
#target_link_libraries     (${EXECUTABLE} PRIVATE ${OpenCV_LIBS})
target_include_directories(${EXECUTABLE} PRIVATE ${OpenCV_INCLUDE_DIRS})

set(CPACK_PROJECT_NAME    ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
