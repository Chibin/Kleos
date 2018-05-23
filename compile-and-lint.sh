#! /bin/bash

BUILD_DIR="${BUILD_DIR:-./build}"

cd "$(pwd)" || exit 1
mkdir -p "$BUILD_DIR"
clang-format -i src/*.cpp src/*.h
(
    cd "$BUILD_DIR" &&
        export CC=/usr/bin/clang &&
        export CXX=/usr/bin/clang++ &&
        cmake -D USE_OPENGL_ES=1 ..

    make "$@" &&
        echo \"success!\"
) &&
    run-clang-tidy-3.8.py
