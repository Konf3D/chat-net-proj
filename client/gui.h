// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
