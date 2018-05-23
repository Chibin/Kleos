FROM debian:stretch

RUN apt-get update && \
    apt-get install -y \
    build-essential \
    cmake \
    libglm-dev \
    libglew-dev \
    libsdl2-dev \
    libsdl2-ttf-dev

RUN apt-get update && \
    apt-get install -y \
    cppcheck \
    git

RUN apt-get update && \
    apt-get install -y \
    ninja-build \
    wget

RUN wget https://releases.llvm.org/6.0.0/clang+llvm-6.0.0-x86_64-linux-gnu-debian8.tar.xz

RUN tar -xvf clang+llvm-6.0.0-x86_64-linux-gnu-debian8.tar.xz

RUN cd clang+llvm-6.0.0-x86_64-linux-gnu-debian8 && cp -r * ..
