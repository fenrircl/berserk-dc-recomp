FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Toolchain, Vulkan, shader compiler, and the SDL3 build dependencies. SDL3 is
# built from source because Ubuntu 24.04 does not package it.
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake ninja-build git curl ca-certificates pkg-config \
        libvulkan-dev glslc \
        libfreetype-dev libx11-dev libxext-dev libxrandr-dev libxi-dev \
        libxcursor-dev libxfixes-dev libxss-dev libxtst-dev \
        libwayland-dev wayland-protocols libxkbcommon-dev libdecor-0-dev \
        libasound2-dev libpulse-dev libudev-dev libdbus-1-dev \
        libegl1-mesa-dev libgl1-mesa-dev libgbm-dev imagemagick \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL -o /tmp/sdl3.tar.gz \
        https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-3.4.18.tar.gz \
    && tar -xzf /tmp/sdl3.tar.gz -C /tmp \
    && cmake -S /tmp/SDL3-3.4.18 -B /tmp/sdl3-build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TESTS=OFF \
        -DCMAKE_INSTALL_PREFIX=/opt/sdl3 \
    && cmake --build /tmp/sdl3-build --parallel 6 \
    && cmake --install /tmp/sdl3-build \
    && rm -rf /tmp/SDL3-3.4.18 /tmp/sdl3-build /tmp/sdl3.tar.gz

ENV CMAKE_PREFIX_PATH=/opt/sdl3
ENV LD_LIBRARY_PATH=/opt/sdl3/lib

WORKDIR /work
