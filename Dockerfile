FROM debian:stretch

RUN apt-get update && \
    apt-get install -y \
    build-essential \
    clang \
    cmake \
    libglm-dev \
    libglew-dev \
    libsdl2-dev \
    libsdl2-ttf-dev

RUN apt-get update && \
    apt-get install -y \
    clang-tidy \
    clang-format \
    cppcheck \
    git \
