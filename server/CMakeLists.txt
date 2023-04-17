cmake_minimum_required(VERSION 3.16)

project(chat-net-srv)

add_subdirectory(proto)

add_executable(chat-net-server chat-net-serv.cpp server.cpp ./proto/chat.grpc.pb.cc ./proto/chat.pb.cc sqlcon.cpp)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
target_link_libraries(chat-net-server PRIVATE unofficial::sqlite3::sqlite3)

find_package(gRPC CONFIG REQUIRED)
target_link_libraries(chat-net-server PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)

find_package(protobuf REQUIRED)
target_link_libraries(chat-net-server PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
