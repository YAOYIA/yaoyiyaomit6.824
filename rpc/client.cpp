#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "message.pb.h"

void CallRpc(const std::string& msg) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    // 1. 准备请求并序列化
    EchoRequest request;
    request.set_message(msg);
    std::string request_data;
    request.SerializeToString(&request_data);
    
    // 2. 发送长度和数据
    uint32_t req_length = request_data.size();
    write(sock, &req_length, sizeof(req_length));
    write(sock, request_data.c_str(), req_length);
    
    // 3. 接收响应长度
    uint32_t res_length = 0;
    read(sock, &res_length, sizeof(res_length));
    
    // 4. 接收响应数据并反序列化
    char buffer[1024] = {0};
    read(sock, buffer, res_length);
    
    EchoResponse response;
    response.ParseFromArray(buffer, res_length);
    
    std::cout << "RPC Result: " << response.reply() << std::endl;
    close(sock);
}

int main(int argc, char* argv[]) {
    std::string msg = (argc > 1) ? argv[1] : "hello, rpc";
    CallRpc(msg);
    return 0;
}