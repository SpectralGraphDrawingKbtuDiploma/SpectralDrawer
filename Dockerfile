FROM grpc/cxx:latest

WORKDIR /app

# Копируем protobuf-схему
COPY proto/helloworld.proto /app/proto/

# Генерируем gRPC-код внутри контейнера
RUN mkdir -p /app/build && \
    protoc -I /app/proto --grpc_out=/app --cpp_out=/app \
           --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` /app/proto/helloworld.proto

# Копируем исходный код и CMakeLists.txt
COPY server.cpp /app/
COPY CMakeLists.txt /app/

# Сборка проекта
RUN cd build && \
    cmake .. && \
    make

# Запускаем сервер
CMD ["./build/server"]
