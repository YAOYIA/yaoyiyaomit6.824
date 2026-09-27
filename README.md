# 分布式系统架构 客户端--服务端模型
在MapReduce和raft共识算法中，所有节点之间的交互都离不开服务端--客户端的通信模型。掌握客户端如何发起请求，服务端如何应答、处理请求是构建分布式系统的第一步。

核心概念：

客户端和服务端将系统的功能分为两个部分。服务器是被动的，它持续监听网络端口、等待外部请求，然后执行响应的业务逻辑并返回结果。客户端的请求是是主动的，它在需要资源或者服务的时候的构建请求，通过网络发送给服务器，等待并且接受响应。
在这个交互的模型中，底层的网络协议负责数据的可靠性。

客户端请求---->TCP网络协议---->服务端---->数据库。


## RPC 远程过程调用
让调用远端函数像本地调用一样简单。

RPC框架中，客户端只需调用本地的一个存根stub，存根负责将函数名和参数序列化成二进制的字节流发送给服务器；服务器接收到数据后进行反序列化，并且进行真正执行，然后将结果返回到客户端。

example
```
1.调用本地存根 stub
客户端程序调用本地环境中的存根函数，这与普通函数调用看起来一模一样。
2.数据序列化 serialization
存根将目标函数名称和传递参数打包，转换成可以在网络中传输的二进制字节流。
3.网络传输与等待响应
底层的网络库将字节流发送到服务端指定的ip和port。此时，采用同步模式的客户端线程会挂起并等待服务器返回结果。
4.反序列化并返回结果
当服务器处理完毕并返回结果字节流之后，客户端的网络接受数据并交由存根进行发序列化，并且将结果返回给调用方，整个RPC过程结束。
```

### RPC实现
使用socket和protobuf实现RPC，是要将本地的请求序列化通过网络协议转发。
核心是三个步骤：数据接口定义、序列化、网络通信。

* 定义接口与数据结构

使用protobuf定义客户端和服务端之间传输的数据结构
```
syntax = "proto3";

//请求数据包
message EchoRequest {
    string message = 1;
}

//响应数据包
message EchoResponse {
    string reply = 1;
}
```
通过 proto --cpp_out=. message.proto编译生成message.pb.h和message.pb.cc 这两个文件提供了数据的序列化和反序列化方法。

* 设计消息封包协议

TCP是流式协议，没有消息边界。通过socket发送protobuf序列后的字节流前，必须在头部添加一个固定长度的字段，通常来表示消息体长度，以解决“粘包”问题。
发送数据格式：
```
【4字节消息长度】+ 【protobuf 序列化后的字节流】
```

* 实现服务端。

服务端需要持续监听端口，接收请求，反序列化数据，执行本地逻辑，最后将结果打包返回。

### 编译与运行

用 CMake 构建。本机 protobuf 是用 `CMAKE_INSTALL_PREFIX=/` 装的（头文件在 `/include`，库在 `/lib`，CMake config 在 `/lib/cmake/protobuf`），所以 `CMakeLists.txt` 里把 `/` 补进了 `CMAKE_PREFIX_PATH`；`find_package(protobuf CONFIG)` 会自动带上 absl、utf8_range 这些静态库依赖，不用手写一长串 `-labsl_*`。

`message.proto` 由 `protobuf_generate` 在构建时调用 `protoc` 生成，产物落在 `build/` 目录，不会污染源码目录。

```bash
cd rpc
cmake -S . -B build     # 配置
cmake --build build -j  # 编译，生成 build/server 和 build/client

./build/server 8080           # 终端 A：启动服务端，默认端口 8080
./build/client "hello, rpc"   # 终端 B：发起一次调用

rm -rf build            # 清理
```


