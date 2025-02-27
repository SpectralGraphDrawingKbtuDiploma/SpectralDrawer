FROM ubuntu:22.04

RUN apt update && apt install -y \
    build-essential \
    cmake \
    g++ \
    protobuf-compiler \
    libprotobuf-dev \
    grpc-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir -p build && cd build && cmake .. && make

CMD ["./build/server"]
