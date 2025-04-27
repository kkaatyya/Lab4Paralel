#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

void sendLine(SOCKET socket, const std::string& msg) 
{
    std::string data = msg + "\n";
    send(socket, data.c_str(), data.size(), 0);
}

std::string recvLine(SOCKET socket) 
{
    char buffer[1024];
    std::string result;
    int bytes;
    while ((bytes = recv(socket, buffer, 1, 0)) > 0) 
    {
        if (buffer[0] == '\n') break;
        result += buffer[0];
    }
    return result;
}

int main() 
{
    
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = htons(5555);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        std::cerr << "Connection failed!" << std::endl;
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;

    sendLine(clientSocket, "HELLO");

    std::string response = recvLine(clientSocket);
    std::cout << "Server response: '" << response << "'" << std::endl;

    sendLine(clientSocket, "EXIT");

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
