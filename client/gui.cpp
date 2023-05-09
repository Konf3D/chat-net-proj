
#include "gui.h"

ChatGUI::ChatGUI()
    :client_(grpc::CreateChannel("localhost:55777", grpc::InsecureChannelCredentials()))
{
}

void ChatGUI::start()
{
    std::cout << "1. SignIn\n2. SignUp\n3. Close\n";
    char choice;
    std::cin >> choice;
    switch (choice)
    {
    case '1':
        trySignIn();
        break;
    case '2':
        trySignUp();
        break;
    case '3':
        return;
    default:
        std::cout << "\nInput not recognized... ";
        break;
    }
    start();
}

void ChatGUI::trySignIn()
{
    std::cout << "Please, enter your credentials (username,password)\n";
    std::cout << "login: ";
    std::string username;
    std::getline(std::cin, username);
    std::getline(std::cin, username);// because first time call gets empty string autimatically
    std::cout << "password: ";
    std::string password;
    std::getline(std::cin, password);
    //std::cin >> login >> password;
    if (client_.Authenticate(username, password))
    {
        user_ = std::move(username);
        client_.RetrieveMessageStream(user_);
        logged();
    }
    else
    {
        std::cout << "Failed! Try again.\n";
    }
    return;
}

void ChatGUI::trySignUp()
{
    std::cout << "Please, enter your credentials (email,password,username)\n";
    std::string login = "";
    do
    {
        std::cout << "email: ";
        std::cin >> login;
        if (login.size() > 4)
            break;
        std::cout << "\nEmail is too short. Try again!\n";
    } while (true);
    std::string username = "";
    do
    {
        std::cout << "username: ";
        std::cin >> username;
        if (username.size() > 4)
            break;
        std::cout << "\nUsername is too short. Try again!\n";
    } while (true);
    std::string password = "";
    do
    {
        std::cout << "password: ";
        std::cin >> password;// because first time call gets empty string autimatically
        if (password.size() > 4)
            break;
        std::cout << "\nPassword is too short. Try again!\n";
    } while (true);
    {
        if (!client_.Register(login, username, password))
        {
            std::cout << "Signup failed!\n";
            return;
        }
        user_ = std::move(username);
        client_.RetrieveMessageStream(user_);
        logged();
    }
    return;
}

void ChatGUI::logged()
{
    static char choice;
    static std::string message;
    std::cout << "\nWhat to do:\n1.Write a message to a user\n2.Display messages\n3.Logout\n";
    std::cin >> choice;
    switch (choice)
    {
    case '1':
    {
        std::cout << "Enter your message:";
        std::getline(std::cin, message);
        std::getline(std::cin, message);// because first time call gets empty string autimatically
        std::cout << "Enter the reciever's username:";
        std::string reciever;
        std::getline(std::cin, reciever);
        client_.Message(user_, reciever, message);
        client_.RetrieveMessageStream(user_);
        break;
    }
    case '2':
    {
        std::ifstream messagesDBFile(dbMessagesFileName,std::ios::beg);
        std::string readbuff[3];
        while (!messagesDBFile.eof())
        {
            std::getline(messagesDBFile, readbuff[0]);
            std::getline(messagesDBFile, readbuff[1]);
            std::getline(messagesDBFile, readbuff[2]);
            if (messagesDBFile.fail())
            {
                //std::cout << "Failed to read data\n";
                break;
            }
            if (readbuff[0] == user_)
            {
                //if (readbuff[1].empty() || readbuff[1] == "\n")
                //    std::cout << "You sent: " << readbuff[2] << '\n';
                //else
                    std::cout << "You sent to user " << readbuff[1] << ":" << readbuff[2] << '\n';
            }
            else
            {
                //if (readbuff[1]==user_)
                //    std::cout << readbuff[0] <<" sent you: " << readbuff[2] << '\n';
                //else
                    std::cout << "You sent user " << readbuff[1] << ":" << readbuff[2] << '\n';
            }
        }
        client_.RetrieveMessageStream(user_);
    }
    break;
    case '3':
        return;
    default:
        std::cout << "\nInput not recognized... ";
        break;
    }
    logged();
}
