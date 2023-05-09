
#pragma once
#include <grpc++/grpc++.h>
#include <fstream>
#include "../server/server.h"
constexpr auto dbMessagesFileName = "usermessages.db";
class ChatClient
{
public:
	ChatClient(std::shared_ptr<grpc::Channel> channel);
	bool Register(const std::string& email, const std::string& username, const std::string& password);
	bool Authenticate(const std::string& username, const std::string& password);
	bool Message(const std::string& sender, const std::string& receiver, const std::string& content);
	void RetrieveMessageStream(const std::string& username);
private:
	std::unique_ptr<chat::ChatService::Stub> stub_;
	std::string token_;
};