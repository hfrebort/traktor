cmake_minimum_required(VERSION 3.0.0)

set(EXECUTABLE traktor)

project(${EXECUTABLE} VERSION 0.1.0)

include(CTest)
enable_testing()

set(SOURCES 
    src/main.cpp
    src/thread_send_jpeg.cpp
    src/webserver.h
    src/webserver.cpp
    src/stats.h
    src/detect.h
    src/detect.cpp
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
  <thread>
  <atomic>
  <cstddef>
  <thread>
  <chrono>
  <opencv2/opencv.hpp>
  [["pistache/endpoint.h"]]
)

target_compile_features(${EXECUTABLE} PRIVATE cxx_std_17)

#target_include_directories(traktor_cpp_test PRIVATE "./deps/boost_1_76_0")
target_include_directories(${EXECUTABLE} PRIVATE "./deps/pistache/include")

add_library(pistache SHARED IMPORTED) # or STATIC instead of SHARED
set_target_properties(pistache PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/deps/pistache/lib/x86_64-linux-gnu/libpistache.so.0.0.002"
)
target_link_libraries(${EXECUTABLE} PRIVATE pistache)

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

# Link your application with OpenCV libraries
target_link_libraries     (${EXECUTABLE} PRIVATE ${OpenCV_LIBS})
target_include_directories(${EXECUTABLE} PRIVATE ${OpenCV_INCLUDE_DIRS})

set(CPACK_PROJECT_NAME    ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
