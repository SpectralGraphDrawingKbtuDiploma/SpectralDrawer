FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
  autoconf \
  automake \
  build-essential \
  cmake \
  curl \
  g++ \
  git \
  libtool \
  make \
  pkg-config \
  unzip \
  && apt-get clean

ENV GRPC_RELEASE_TAG latest

RUN git clone -b ${GRPC_RELEASE_TAG} https://github.com/grpc/grpc /var/local/git/grpc && \
    cd /var/local/git/grpc && \
    git submodule update --init --recursive

RUN echo "-- installing protobuf" && \
cd /var/local/git/grpc/third_party/protobuf && \
./autogen.sh && ./configure --enable-shared && \
make -j$(nproc) && make -j$(nproc) check && make install && make clean && ldconfig

RUN echo "-- installing grpc" && \
    cd /var/local/git/grpc && \
    make -j$(nproc) && make install && make clean && ldconfig

WORKDIR /app

COPY . .

RUN mkdir -p build && cd build && cmake .. && make

CMD ["./build/server"]
