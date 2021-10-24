#!/bin/bash

# https://qiita.com/rtakasuke/items/fd35ef60370e3d8c225d
set -e

clang-format -i --sort-includes main.cc
