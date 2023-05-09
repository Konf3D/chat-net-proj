
#pragma once
#include <fstream>
#include <iostream>
#include <random>
#include "server.h"
constexpr auto dbFileName = "server.db";
constexpr auto maxTries = 5;
ChatServer::ChatServer()
	:sql(dbFileName)
{

}

::grpc::Status ChatServer::AuthenticateUser(::grpc::ServerContext* context, const::chat::User* request, ::chat::Token* response)
{
	const auto userID = sql.getUser(request->username());
	const auto user = sql.getUser(userID);
	if (!user || user->password != request->password())
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Invalid login/password");
	}
	std::string token;
	int attempt = 0;
	bool condition;
	do
	{
		condition = (sql.insertToken(token, userID) || attempt > maxTries);
		++attempt;
		token = generateToken();
	} while (condition = false);
	if (attempt > maxTries)
		return grpc::Status(grpc::StatusCode::INTERNAL, "Server failed");
	return grpc::Status::OK;
}

::grpc::Status ChatServer::RegisterUser(::grpc::ServerContext* context, const::chat::User* request, ::chat::Token* response)
{
	if(sql.getUser(request->username()))
		return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "Username taken");
	const auto separator = request->email().find_first_of("@");
	const auto user = sql.getUser(request->email().substr(0,separator-1),request->email().substr(separator));
	if (separator != request->email().find_last_of("@") || separator == std::string::npos)
		return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Email invalid");
	if(user)
		return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "email taken");

	User usr = { request->username(),//username
	std::string(),//name
	std::string(),//surname
	request->email().substr(0,separator - 1),//emailname
	request->email().substr(separator + 1),//emaildomain
	request->password(),
	};
	sql.insertUser(usr);
	const auto userID = sql.getUser(request->username());

	std::string token;
	int attempt = 0;
	bool condition;
	do
	{
		condition = (sql.insertToken(token, userID) || attempt > maxTries);
		++attempt;
		token = generateToken();
	} while (condition = false);
	if (attempt > maxTries)
		return grpc::Status(grpc::StatusCode::INTERNAL, "Server failed");
	return grpc::Status::OK;

}

::grpc::Status ChatServer::SendMessage(::grpc::ServerContext* context, const::chat::Message* request, ::chat::Token* response)
{
	auto senderUsernameTokenMeta = context->client_metadata().begin();
	const int usernameMeta_UserID = sql.getUser(std::string(senderUsernameTokenMeta->first.begin(), senderUsernameTokenMeta->first.end()));
	const int tokenMeta_UserID = sql.getTokenUser(std::string(senderUsernameTokenMeta->second.begin(), senderUsernameTokenMeta->second.end()));
	const int sender_UserID = sql.getUser(request->sender());
	const int reciever_UserID = sql.getUser(request->receiver());
	if(usernameMeta_UserID * tokenMeta_UserID * sender_UserID == 0)
		return grpc::Status(grpc::StatusCode::NOT_FOUND, "Sender credentials invalid");
	if(usernameMeta_UserID * tokenMeta_UserID * sender_UserID == 0 || usernameMeta_UserID!=tokenMeta_UserID ||	tokenMeta_UserID != sender_UserID)
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Sender credentials invalid");
	if(reciever_UserID == 0)
		return grpc::Status(grpc::StatusCode::NOT_FOUND, "Reciever not found");

	if(!sql.insertMessage({ request->content(),sender_UserID,reciever_UserID }))
		return grpc::Status(grpc::StatusCode::INTERNAL, "Message database failed");

	return grpc::Status::OK;
}

::grpc::Status ChatServer::GetMessageStream(::grpc::ServerContext* context, const::chat::Token* request, ::grpc::ServerWriter<::chat::Message>* writer)
{
	const int userID = sql.getTokenUser(request->message());
	if(userID == 0)
		return grpc::Status(grpc::StatusCode::NOT_FOUND, "User access token invalid");
	auto messages = sql.getMessages(userID);
	auto write = [&](const Message& msg)
	{
		chat::Message message;
		message.set_content(msg.content);
		message.set_sender(sql.getUser(msg.sender)->username);
		message.set_receiver(sql.getUser(msg.reciever)->username);
		writer->Write(message);
	};
	return grpc::Status::OK;
}
