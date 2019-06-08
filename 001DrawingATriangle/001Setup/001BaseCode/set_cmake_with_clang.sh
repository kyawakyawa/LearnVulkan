#!/bin/bash

CXX=clang++ CC=clang cmake -Bbuild -H. -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..

mv build/compile_commands.json .
