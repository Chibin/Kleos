#! /bin/bash

time (
docker build -t kleos:testing .
docker run --rm -it -v"$(pwd)":"$(pwd)":Z -w "$(pwd)" kleos:testing /bin/sh -c "BUILD_DIR=./docker-build ./compile-and-lint.sh; chown -R \"$(stat -c \"%u:%g\" .)\" \"$(pwd)\""
# docker run --rm -it -v"$(pwd)":"$(pwd)":Z -w "$(pwd)" -it --rm kleos:testing
)
