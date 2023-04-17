// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <random>
#include <grpc++/grpc++.h>
#include <utility>
#include "proto/chat.pb.h"
#include "proto/chat.grpc.pb.h"
class SQLcon;

struct Message
{
    Message() = default;
    ~Message() = default;
    std::string content;
    int64_t sender;
    int64_t reciever;
};
struct User
{
    User() = default;
    ~User() = default;
    std::string username;
    std::string name;
    std::string surname;
    std::string email;
    std::string password;
    std::string token;
};
class ChatServer : public chat::ChatService::Service
{

};

inline std::string generateToken(std::size_t length = 64)
{
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, int(CHARACTERS.size()) - 1);

    std::string random_string;

    for (size_t i = 0; i < length; ++i)
    {
        random_string += CHARACTERS[distribution(generator)];
    }
    return random_string;
}
inline void server()
{
    std::string server_address("localhost:55777");
    ChatServer service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}