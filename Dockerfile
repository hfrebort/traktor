FROM alpine:latest as build

RUN  apk update         \
  && apk upgrade        \
  && apk add --no-cache \
    wget                \
    clang               \
    alpine-sdk          \
    cmake               \
    libgpiod-dev        \
    opencv-dev

RUN     mkdir -p    /traktor/deps/cpp-httplib   \
                    /traktor/deps/json          \
    &&  wget -O     /traktor/deps/cpp-httplib/httplib.h https://github.com/yhirose/cpp-httplib/raw/master/httplib.h                     \
    &&  wget -O     /traktor/deps/json/json.hpp         https://github.com/nlohmann/json/raw/develop/single_include/nlohmann/json.hpp

COPY ./src              /traktor/src
COPY ./CMakeLists.txt   /traktor/CMakeLists.txt

WORKDIR /traktor/build
RUN     cmake -DCMAKE_BUILD_TYPE=Release .. \
    &&  make -j`nproc`                      \
    &&  strip --strip-all ./traktor
#
# MAIN image
#
from alpine:latest

RUN  apk update         \
  && apk add --no-cache \
    libgpiod            \
    opencv

#
# 2024-01-16 Spindler
#   libopencv_aruco.so is needed (warum a imma) and not part of the "opencv" package from alpine
#   dirty hack: copy it from the build stage. (ned schee)
#
COPY --from=build   /usr/lib/libopencv_aruco.so.4.8.1 /usr/lib/libopencv_aruco.so.4.8.1
RUN ln -s           /usr/lib/libopencv_aruco.so.4.8.1 /usr/lib/libopencv_aruco.so.408

COPY --from=build   /traktor/build/traktor  /app/
COPY                ./static                /app/static/

WORKDIR /app
EXPOSE 9080
ENTRYPOINT ["./traktor"]


