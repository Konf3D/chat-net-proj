
#pragma once
#include "client.h"
class ChatGUI
{
public:
    ChatGUI();
    ~ChatGUI() = default;
public:
    void start();
private:
    void trySignIn();
    void trySignUp();
    void logged();
    ChatClient client_;
    std::string user_;
};
