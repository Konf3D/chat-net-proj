
#pragma once
#include <mutex>
#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>
struct Message
{
    std::string content;
    int sender;
    int reciever;
};
struct User
{
    std::string username;
    std::string name;
    std::string surname;
    std::string emailname;
    std::string emaildomain;
    std::string password;
};
inline int callbackMsg(void* data, int argc, char** argv, char** azColName) {
    auto messages = static_cast<std::vector<Message>*>(data);
    Message m{
        argv[0] ? argv[0] : "",
        std::stoi(argv[1]),
        std::stoi(argv[2])
    };
    messages->emplace_back(std::move(m));
    return 0;
}
inline int callbackUser(void* data, int argc, char** argv, char** azColName) {
    auto users = static_cast<std::vector<User>*>(data);
    User m{
        argv[1] ? argv[1] : "",
        argv[2] ? argv[2] : "",
        argv[3] ? argv[3] : "",
        argv[4] ? argv[4] : "",
        argv[5] ? argv[5] : "",
        argv[6] ? argv[6] : "",
    };
    users->emplace_back(std::move(m));
    return 0;
}
class SQLcon
{
public:
    SQLcon(const std::string& dbFileName);
    ~SQLcon();
    bool initTable();
    bool insertUser(const User& record);
    bool insertMessage(const Message& record);
    bool insertToken(const std::string& token, int userID);
    void deleteTokenByUserId(int userId);
    std::vector<Message> getMessages(int UserID);
    std::unique_ptr<User> getUser(int UserID);
    int getUser(const std::string& emailname, const std::string& emaildomain);
    int getUser(const std::string& username);
    int getTokenUser(const std::string& token);
private:
    int rc = 0;
    sqlite3* db;
    char* err_msg = nullptr;
};

enum StatusMsg
{
    SENT = 1,
    RECIEVED = 2,
    READ = 3,
};