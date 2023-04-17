// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include <filesystem>
#include "client.h"
#include "../server/logger.h"
#include "../server/server.h"

constexpr auto grpcErrorLogFile = "grpc-client.log";
ChatClient::ChatClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(chat::ChatService::NewStub(channel))
{

}

bool ChatClient::Register(const std::string& login, const std::string& username, const std::string& password)
{
    return 0;
}

bool ChatClient::Authenticate(const std::string& username, const std::string& password)
{
    return 0;
}

bool ChatClient::Message(const std::string& sender, const std::string& receiver, const std::string& content)
{
    return 0;
}

void ChatClient::RetrieveMessageStream(const std::string& username)
{

}
