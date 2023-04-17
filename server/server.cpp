// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#pragma once
#include <fstream>
#include <iostream>
#include <random>
#include "server.h"

constexpr auto dbUsersFileName = "users.db";
constexpr auto dbMessagesFileName = "messages.db";
//Complete

ChatServer::ChatServer()
{
#ifdef WIN32
    /* check if database files are present*/
    system("powershell ni users.db");
    system("powershell ni messages.db");
#elif unix
    system("touch users.db && chmod 600 users.db");
    system("touch messages.db && chmod 600 messages.db");
#endif
    /* LOADING DATA FROM LOCAL DATABASE*/
    std::ifstream usersDBFile(dbUsersFileName);
    if (usersDBFile.is_open())
    {
        std::string login;
        std::string username;
        std::string password;
        std::string email;
        while (std::getline(usersDBFile, login) && std::getline(usersDBFile, username) && std::getline(usersDBFile, password) && std::getline(usersDBFile, email))
        {
            if (username.empty() || password.empty() || login.empty())
                //throw std::ios_base::failure("Database corrupted");
                break;

            users_.push_back({ login, username, password,std::string(), email });
        }
    }

    /* LOADING MESSAGES*/
    std::ifstream messageDBFile(dbMessagesFileName);
    if (messageDBFile.is_open())
    {
        std::string message;
        std::string sender;
        std::string reciever;
        while (std::getline(messageDBFile, sender) && std::getline(messageDBFile, reciever) && std::getline(messageDBFile, message))
        {
            if (message.empty() || sender.empty() || reciever.empty())
                //throw std::ios_base::failure("Database corrupted");
                break;

            messages_.push_back({ sender, reciever, message });
        }
    }

}

//Complete
grpc::Status ChatServer::AuthenticateUser(grpc::ServerContext* context, const chat::User* request, chat::ServerResponse* response)
{    
    // Check if user exists and password is correct
    auto isLoginPresent = [&request](const User& user)
    {
        return (request->username() == user.login && request->password()==user.password);
    };
    auto it = std::find_if(users_.begin(), users_.end(), isLoginPresent);
    if (it == users_.end()) {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Invalid credentials");
    }
    else {
        std::string token = generateToken();
        it->token = token;
        response->set_message(token);
        return grpc::Status::OK;
    }
    return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "Server failed");
}
//Complete
grpc::Status ChatServer::RegisterUser(grpc::ServerContext* context, const chat::User* request, chat::ServerResponse* response)
{

    auto isLoginPresent = [&request](const User& user)
    {
        return (request->username() == user.username || request->login() == user.login);
    };
    auto it = std::find_if(users_.begin(), users_.end(), isLoginPresent);
    // Check if user already exists
    if (it != users_.end()) {
        return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "Username/login taken");
    }
    else {
        // Add user to user list
        std::string token = generateToken();
        users_.push_back({ request->login(),request->username(),request->password(),token,request->email() });
        std::ofstream usersDBFile(dbUsersFileName, std::ios::app);
        usersDBFile << request->login() << '\n' << request->username() << '\n' << request->password() << '\n' << request->email() << '\n';
        response->set_message(token);
        return grpc::Status::OK;
    }
    return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "Server failed");
}
//Complete
grpc::Status ChatServer::SendMessage(grpc::ServerContext* context, const chat::Message* request, chat::SendMessageResponse* response)
{
    
    const auto& senderToken = context->client_metadata();

    auto isSenderPresent = [&request](const User& user)
    {
        return (request->sender() == user.username);
    };
    auto isRecieverPresent = [&request](const User& user)
    {
        return (request->receiver() == user.username);
    };
    const auto& sender_it = std::find_if(users_.begin(),users_.end(),isSenderPresent);
    const auto& receiver_it = std::find_if(users_.begin(), users_.end(), isRecieverPresent);
    if (sender_it == users_.end())
    {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Sender credentials invalid");
    }
    if (sender_it->token != senderToken.begin()->second || sender_it->username != senderToken.begin()->first)
    {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Sender signature invalid");
    }
    messages_.push_back({ request->content(),request->sender(),request->receiver() });
    std::ofstream messagesDBFile(dbMessagesFileName, std::ios::app);
    if (messagesDBFile.is_open())
    {
        messagesDBFile << request->content() << '\n' << request->sender() << '\n' << request->receiver() << '\n';

        if(messagesDBFile.bad())
            return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "Server failed");

        return grpc::Status::OK;
    }

    return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "Server failed");
}
//Complete
grpc::Status ChatServer::GetMessageStream(grpc::ServerContext* context, const chat::ServerResponse* request, grpc::ServerWriter< chat::Message>* writer)
{
    auto isTokenPresent = [&request](const User& user)
    {
        return (request->message() == user.token);
    };
    const auto& it = std::find_if(users_.begin(), users_.end(), isTokenPresent);
    if (it == users_.end()) {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "User signature invalid");
    }
    for (const auto& message_data : messages_) 
    {
        chat::Message msg;
        msg.set_sender(message_data.sender);
        msg.set_receiver(message_data.reciever);
        msg.set_content(message_data.content);
        writer->Write(msg);
    }

    return grpc::Status::OK;
}
