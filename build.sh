#!/bin/sh
echo
clang -g -Wno-c99-designator -Wno-dollar-in-identifier-extension -fdiagnostics-absolute-paths -Wno-format-invalid-specifier src/main.cpp src/utilities.cpp src/parser.cpp -o build/graph
build/graph
