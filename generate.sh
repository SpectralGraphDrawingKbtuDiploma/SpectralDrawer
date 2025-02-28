#!/bin/bash

echo "Генерация gRPC-кода..."
mkdir -p /app/generated

protoc --proto_path=/app/proto --cpp_out=/app/generated --grpc_out=/app/generated --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) /app/proto/calculator.proto

echo "Генерация завершена!"
ls -l /app/generated/
