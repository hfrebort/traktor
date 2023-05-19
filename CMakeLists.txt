cmake_minimum_required(VERSION 3.0.0)

set(EXECUTABLE traktor)
#set(CMAKE_CXX_FLAGS "-march=native")

project(${EXECUTABLE} VERSION 0.1.0)

include(CTest)
enable_testing()

set(SOURCES 
    src/main.cpp
    src/webserver.cpp
    src/stats.h
    #src/thread_detect.cpp
    src/shared.h
    src/detect_settings.cpp
    src/camera.h
    src/detect.h
    src/encode.h
    src/pipeline_1_camera.cpp
    src/pipeline_2_detect.cpp
    src/pipeline_3_encode_send.cpp
    src/thread_stats.cpp
    src/util.h
    src/util.cpp
    src/harrow.h
    src/harrow.cpp
    src/thread_center_harrow.cpp
    src/pipeline/auto_reset_event.cpp
    src/pipeline/auto_reset_event.hpp
    src/pipeline/Postbox.cpp
    src/pipeline/Postbox.hpp
    src/pipeline/ImagePipeline.cpp
    src/pipeline/ImagePipeline.hpp
)

add_executable(${EXECUTABLE} ${SOURCES})

target_precompile_headers(${EXECUTABLE} PRIVATE
  <string>
  <signal.h>
  <sys/stat.h>
  <stdio.h>
  <dirent.h>
  <iostream>
  <fstream>
  <functional>
  <memory>
  <condition_variable>
  <thread>
  <atomic>
  <cstddef>
  <chrono>
  <opencv2/core.hpp>
  <opencv2/imgproc.hpp>
  <opencv2/imgcodecs.hpp>
  <opencv2/highgui.hpp>
  [["httplib.h"]]
  [["json.hpp"]]
)

target_compile_features(${EXECUTABLE} PRIVATE cxx_std_14)

target_include_directories(${EXECUTABLE} PRIVATE "./deps/cpp-httplib")
target_include_directories(${EXECUTABLE} PRIVATE "./deps/json")

#
# libgpiod
#
target_include_directories(${EXECUTABLE} PRIVATE "/usr/local/include")
target_link_libraries     (${EXECUTABLE} PRIVATE "gpiod"    )

message(STATUS CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${EXECUTABLE} PRIVATE Threads::Threads)

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV REQUIRED)

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
#message(STATUS "OpenCV library status:")
#message(STATUS "    config: ${OpenCV_DIR}")
#message(STATUS "    version: ${OpenCV_VERSION}")
#message(STATUS "    libraries: ${OpenCV_LIBS}")
#message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

message(STATUS "OPENCV target_link_libraries: ${OpenCV_LIBS}")

# Link your application with OpenCV libraries
target_link_libraries     (${EXECUTABLE} PRIVATE ${OpenCV_LIBS})
target_include_directories(${EXECUTABLE} PRIVATE ${OpenCV_INCLUDE_DIRS})

set(CPACK_PROJECT_NAME    ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
