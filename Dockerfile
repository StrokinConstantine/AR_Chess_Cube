FROM ubuntu:25.04

RUN apt update && \
    apt install -y \
    cmake \
    libopencv-dev \
	g++ \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ARChessCube
COPY . .

RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make

CMD ["/ARChessCube/build/ARChessCube"]