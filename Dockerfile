FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

RUN apt-get update && \
    apt-get install -y \
    cmake \
    libopencv-dev \
    g++ \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Установка nlohmann/json
RUN wget https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp -O /usr/include/json.hpp

RUN useradd -m aruser && \
    mkdir -p /ARChessCube && \
    chown aruser:aruser /ARChessCube

USER aruser
WORKDIR /ARChessCube

COPY --chown=aruser:aruser . .

RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make