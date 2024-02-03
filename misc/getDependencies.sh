#!/bin/sh
mkdir -p ./deps/cpp-httplib
mkdir -p ./deps/json
wget -O ./deps/cpp-httplib/httplib.h https://github.com/yhirose/cpp-httplib/raw/master/httplib.h
wget -O ./deps/json/json.hpp         https://github.com/nlohmann/json/raw/develop/single_include/nlohmann/json.hpp