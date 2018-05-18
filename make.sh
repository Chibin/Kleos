#! /bin/bash

docker build -t jobless:testing .
docker run --rm -it -v"$(pwd)":$(pwd):Z jobless:testing /bin/sh -c "\
    cd $(pwd) || exit 1; \
    mkdir -p docker-build; \
    clang-format -i src/*.cpp src/*.h; \
    (cd docker-build && \
        cmake -D USE_OPENGL_ES=1 ..; \
        run-clang-tidy-3.8.py && \
        make $* && \
        echo \"success!\") && \
    clang-format -i src/*.cpp src/*.h; \
    chown -R $(stat -c "%u:%g" .) $(pwd) \
    "
