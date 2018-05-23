#! /bin/bash

BUILD_DIR="${BUILD_DIR:-./build}"

mkdir -p "$BUILD_DIR"
clang-format -i src/*.cpp src/*.h
(
    cd "$BUILD_DIR" &&
        CC=$(which clang) &&
        CXX=$(which clang++) &&
        export CC &&
        export CXX &&
        cmake -D USE_OPENGL_ES=1 -G Ninja ..

    ninja "$@" &&
        echo \"success!\"

) &&
    clang-tidy src/*.cpp

ln -sf "$BUILD_DIR/compile_commands.json" .
