#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "message.pb.h"

void StartServer(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);

    while (true) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        
        // 1. 读取 4 字节消息长度 (简化了错误处理和循环读取)
        uint32_t msg_length = 0;
        read(client_socket, &msg_length, sizeof(msg_length));
        
        // 2. 读取 Protobuf 数据
        char buffer[1024] = {0};
        read(client_socket, buffer, msg_length);
        
        // 3. 反序列化
        EchoRequest request;
        request.ParseFromArray(buffer, msg_length);
        
        // 4. 执行业务逻辑
        EchoResponse response;
        response.set_reply("Server received: " + request.message());
        
        // 5. 序列化响应并发送
        std::string response_data;
        response.SerializeToString(&response_data);
        uint32_t res_length = response_data.size();
        
        write(client_socket, &res_length, sizeof(res_length));
        write(client_socket, response_data.c_str(), res_length);
        
        close(client_socket);
    }
}

int main(int argc, char* argv[]) {
    int port = (argc > 1) ? std::stoi(argv[1]) : 8080;
    std::cout << "Server listening on port " << port << std::endl;
    StartServer(port);
    return 0;
}
