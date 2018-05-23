#! /bin/bash

time (
docker build -t jobless:testing .
docker run --rm -it -v"$(pwd)":"$(pwd)":Z -w "$(pwd)" jobless:testing /bin/sh -c "BUILD_DIR=./docker-build ./compile-and-lint.sh; chown -R \"$(stat -c \"%u:%g\" .)\" \"$(pwd)\""
# docker run --rm -it -v"$(pwd)":"$(pwd)":Z -w "$(pwd)" -it --rm jobless:testing
)
