#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "service.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using mypackage::MyService;
using mypackage::HelloRequest;
using mypackage::HelloReply;

class MyClient {
public:
    MyClient(std::shared_ptr<Channel> channel)
        : stub_(MyService::NewStub(channel)) {}

    std::string SayHello(const std::string& name) {
        HelloRequest request;
        request.set_name(name);
        HelloReply reply;
        ClientContext context;

        Status status = stub_->SayHello(&context, request, &reply);
        if (status.ok()) {
            return reply.message();
        } else {
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<MyService::Stub> stub_;
};

int main() {
    MyClient client(grpc::CreateChannel("server:50051", grpc::InsecureChannelCredentials()));
    std::string response = client.SayHello("Alice");
    std::cout << "Server response: " << response << std::endl;
    return 0;
}
