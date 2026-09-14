# Multi-stage build for cgminer.
#
# The build stage compiles the miner with the same toolchain and options the
# CI workflow uses (no OpenCL SDK required). The runtime stage keeps only the
# binary, the OpenCL kernel sources and the shared libraries it needs.

FROM ubuntu:22.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        autoconf \
        automake \
        libtool \
        pkg-config \
        libcurl4-openssl-dev \
        libncurses-dev \
        libudev-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

# --disable-opencl keeps the build free of the AMD/OpenCL SDK, and
# --enable-avalon satisfies configure's requirement that at least one mining
# driver is enabled when OpenCL is unavailable.
RUN sh autogen.sh --disable-opencl --enable-avalon \
    && make -j"$(nproc)"

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        libcurl4 \
        libncurses6 \
        libudev1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /src/cgminer /app/cgminer
COPY --from=build /src/*.cl /app/

# The OpenCL kernels live next to the binary; point --kernel-path at them.
ENTRYPOINT ["/app/cgminer", "--kernel-path", "/app"]