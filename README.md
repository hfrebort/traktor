# tractor

A system to control a harrow

- The current view of camera
- Initialize harrow(drive to the middle)
- Adjust harrow left right
- Start/Stop automatic arrow adjustment

# setup raspberry

Use raspap.com to install an apache and install wifi connection
(https://www.youtube.com/watch?v=YbvSS8MJm2s)

# docker (easy)

    1. docker build -t traktor .
    2. docker run --rm --device=/dev/video0 -p 9080:9080 traktor

An USB camera on /dev/video0
Web UI should be reachable by: http://localhost:9080

# build on your own

## prerequistites

    sudo apt install libopencv-dev libgpiod-dev

    + mkdir -p ./deps/cpp-httplib ./deps/json
    + wget -O ./deps/cpp-httplib/httplib.h https://github.com/yhirose/cpp-httplib/raw/master/httplib.h
    + wget -O ./deps/json/json.hpp https://github.com/nlohmann/json/raw/develop/single_include/nlohmann/json.hpp

## build

    mkdir Release
    cd Release
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make

## run
    ./traktor
