FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

RUN apt-get update && \
    apt-get install -y \
    cmake \
    libopencv-dev \
    g++ \
#    v4l-utils \
#    libv4l-dev \
#    libgtk2.0-dev \
#    libcanberra-gtk-module \
    && rm -rf /var/lib/apt/lists/*

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

CMD ["/ARChessCube/build/ARChessCube"]