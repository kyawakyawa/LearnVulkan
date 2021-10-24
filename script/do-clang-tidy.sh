#!/bin/bash

# https://qiita.com/rtakasuke/items/fd35ef60370e3d8c225d
set -e

clang-tidy --extra-arg=-std=c++17 --fix --fix-errors main.cc
