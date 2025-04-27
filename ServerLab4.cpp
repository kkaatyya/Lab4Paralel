#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

std::mutex coutMutex;

void sendLine(SOCKET client, const std::string& msg) 
{
    std::string data = msg + "\n";
    send(client, data.c_str(), data.size(), 0);
}

std::string recvLine(SOCKET client) 
{
    char buffer[1024];
    std::string result;
    int bytes;
    while ((bytes = recv(client, buffer, 1, 0)) > 0) 
    {
        if (buffer[0] == '\n') break;
        result += buffer[0];
    }
    return result;
}

void processClient(SOCKET clientSocket) 
{
    while (true) 
    {
        std::string cmd = recvLine(clientSocket);

        if (cmd == "EXIT") 
        {
            break;
        }
        else 
        {
            sendLine(clientSocket, "UNKNOWN_COMMAND");
        }
    }

    closesocket(clientSocket);
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "Client disconnected\n";
}

int main() 
{
    
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(5555);

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "Server started on port 5555...\n";

    while (true) 
    {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "New client connected\n";
        std::thread(processClient, clientSocket).detach();
    }

    WSACleanup();
    return 0;
}
