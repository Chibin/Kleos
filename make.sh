#! /bin/bash

docker run --rm -it -v"$(pwd)":$(pwd):Z jobless:testing /bin/sh -c "\
    cd $(pwd) || exit 1; \
    cmake -D USE_OPENGL_ES=1 .; \
    clang-format -i *.cpp *.h
        make $* && \
        run-clang-tidy-3.8.py; \
    chown -R $(stat -c "%u:%g" .) $(pwd) \
    "

# git diff -U0 --no-color HEAD^ | clang-format-diff -i -p1 && \
