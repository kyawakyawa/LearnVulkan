#!/bin/bash

rm -rf cmake-build-release
rm -rf cmake-build-debug

CXX=clang++ CC=clang cmake \
                       -Bcmake-build-release -H. -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .. \
                       -DCMAKE_BUILD_TYPE=Release \
                       -DVULKAN_APP_USE_GLOG=ON

CXX=clang++ CC=clang cmake \
                       -Bcmake-build-debug -H. -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .. \
                       -DCMAKE_BUILD_TYPE=Debug \
                       -DVULKAN_APP_USE_GLOG=ON

mv cmake-build-debug/compile_commands.json .
