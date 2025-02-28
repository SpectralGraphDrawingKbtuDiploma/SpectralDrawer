#include <iostream>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "generated/calculator.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using calculator::Calculator;
using calculator::AddRequest;
using calculator::AddResponse;

class CalculatorClient {
public:
    CalculatorClient(std::shared_ptr<Channel> channel)
        : stub_(Calculator::NewStub(channel)) {}

    int Add(int a, int b) {
        AddRequest request;
        request.set_a(a);
        request.set_b(b);

        AddResponse response;
        ClientContext context;

        Status status = stub_->Add(&context, request, &response);
        if (status.ok()) {
            return response.result();
        } else {
            std::cerr << "RPC failed" << std::endl;
            return -1;
        }
    }

private:
    std::unique_ptr<Calculator::Stub> stub_;
};

int main() {
    CalculatorClient client(grpc::CreateChannel("server:50051", grpc::InsecureChannelCredentials()));

    int result = client.Add(10, 20);
    std::cout << "Result: " << result << std::endl;

    return 0;
}
