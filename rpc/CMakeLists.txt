cmake_minimum_required(VERSION 3.16)
project(rpc CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 本机 protobuf 是用 CMAKE_INSTALL_PREFIX=/ 装的：头文件在 /include，库在 /lib，
# CMake config 在 /lib/cmake/protobuf。把 "/" 补进搜索路径，find_package 才定位得到。
if(NOT "/" IN_LIST CMAKE_PREFIX_PATH)
  list(APPEND CMAKE_PREFIX_PATH "/")
endif()

# CONFIG 模式会加载 protobuf 自己的 protobuf-config.cmake，
# 它会把 absl / utf8_range 等静态库依赖一并带上，不用像手写命令行那样列一长串。
find_package(protobuf CONFIG REQUIRED)
find_package(Threads REQUIRED)

# 把 message.proto 编成一个静态库，server 和 client 共用同一份生成代码。
# protobuf_generate 会在构建时调用 protoc，生成 message.pb.cc / message.pb.h，
# 并把生成的文件加进这个 target 的源码列表。
add_library(message_proto STATIC message.proto)
protobuf_generate(TARGET message_proto LANGUAGE cpp)

target_link_libraries(message_proto PUBLIC protobuf::libprotobuf)

# 生成的 message.pb.h 落在构建目录里，需要用它的 target 得能 include 到
target_include_directories(message_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_executable(server server.cpp)
target_link_libraries(server PRIVATE message_proto Threads::Threads)

add_executable(client client.cpp)
target_link_libraries(client PRIVATE message_proto Threads::Threads)
