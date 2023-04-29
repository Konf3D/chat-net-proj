// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include <filesystem>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include "client.h"
#include "../server/logger.h"
#include "../server/server.h"
std::string sha256(const std::string& password) {
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256();
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;

    EVP_DigestInit_ex(mdctx, md, nullptr);
    EVP_DigestUpdate(mdctx, password.c_str(), password.size());
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < md_len; ++i) {
        ss << std::setw(2) << static_cast<int>(md_value[i]);
    }

    return ss.str();
}
constexpr auto grpcErrorLogFile = "grpc-client.log";
ChatClient::ChatClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(chat::ChatService::NewStub(channel))
{

}

bool ChatClient::Register(const std::string& email, const std::string& username, const std::string& password)
{
    chat::User user;
    user.set_email(email);
    user.set_username(username);
    user.set_password(sha256(password));
    chat::Token response;

    grpc::ClientContext context;
    grpc::Status status = stub_->RegisterUser(&context, user, &response);

    if (status.ok())
    {
        token_ = response.message();
        return true;
    }
    else
    {
        grpcLog(grpcErrorLogFile, status);
        return false;
    }
    return 0;
}

bool ChatClient::Authenticate(const std::string& username, const std::string& password)
{
    chat::User user;
    user.set_username(username);
    user.set_password(sha256(password));
    chat::Token response;

    grpc::ClientContext context;
    grpc::Status status = stub_->AuthenticateUser(&context, user, &response);
    if (status.ok())
    {
        token_ = response.message();
        return true;
    }
    else
    {
        grpcLog(grpcErrorLogFile, status);
        return false;
    }
}

bool ChatClient::Message(const std::string& sender, const std::string& receiver, const std::string& content)
{
    chat::Message message;
    message.set_sender(sender);
    message.set_receiver(receiver);
    message.set_content(content);
    chat::Token response;

    grpc::ClientContext context;
    context.AddMetadata(sender, token_);
    grpc::Status status = stub_->SendMessage(&context, message, &response);
    if (!status.ok())
    {
        grpcLog(grpcErrorLogFile, status);
        return false;
    }
    return true;
}

void ChatClient::RetrieveMessageStream(const std::string& username)
{
    chat::Token token;
    token.set_message(token_);
    grpc::ClientContext context;
    std::unique_ptr<grpc::ClientReader<chat::Message>> reader(stub_->GetMessageStream(&context, token));

    chat::Message message;

    std::ofstream messagesDBFile(dbMessagesFileName, std::ios::app);
    while (reader->Read(&message)) {
        messagesDBFile << message.sender() << '\n' << message.receiver() << '\n' << message.content() << '\n';
    }

    grpc::Status status = reader->Finish();
    if (!status.ok()) {
        grpcLog(grpcErrorLogFile, status);
    }
    return;
}
