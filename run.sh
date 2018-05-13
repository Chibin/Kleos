#! /bin/bash

docker run --rm -it -v"$(pwd)":/build:Z jobless:testing /bin/sh -c "\
    cd /build || exit 1; \
    cmake -D USE_OPENGL_ES=1 .; \
    make; \
    chown -R $(stat -c "%u:%g" .) /build \
    "
