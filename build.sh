#!/bin/sh
echo
clang -Wno-dollar-in-identifier-extension -fdiagnostics-absolute-paths src/main.cpp -o build/graph
build/graph
