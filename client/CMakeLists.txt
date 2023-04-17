cmake_minimum_required(VERSION 3.16)

project(chat-client)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chat-net-client chat-net-client.cpp client.cpp gui.cpp ../server/proto/chat.grpc.pb.cc ../server/proto/chat.pb.cc ../server/logger.h)
find_package(gRPC CONFIG REQUIRED)
target_link_libraries(chat-net-client PRIVATE gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)

find_package(protobuf REQUIRED)
#target_link_libraries(chat-net-client )