#!/bin/sh
echo

CXX="ccache clang"
CXX_FLAGS="-g -Wno-c99-designator -Wno-dollar-in-identifier-extension -fdiagnostics-absolute-paths -Wno-format-invalid-specifier -fno-rtti -fno-exceptions"

${CXX} ${CXX_FLAGS} -c src/main.cpp      -o build/main.o
${CXX} ${CXX_FLAGS} -c src/utilities.cpp -o build/utilities.o
${CXX} ${CXX_FLAGS} -c src/parser.cpp    -o build/parser.o
${CXX} build/*.o -o build/graph
build/graph
