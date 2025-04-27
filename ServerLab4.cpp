#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

std::string recvLine(SOCKET socket) {
    char buffer[1024];
    std::string result;
    int bytes;
    while ((bytes = recv(socket, buffer, 1, 0)) > 0) {
        if (buffer[0] == '\n') break;
        result += buffer[0];
    }
    return result;
}

void sendLine(SOCKET socket, const std::string& msg) {
    std::string data = msg + "\n";
    send(socket, data.c_str(), data.size(), 0);
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

void handleClient(SOCKET clientSocket) {
    while (true) {
        std::string command = trim(recvLine(clientSocket));
        if (command.empty()) {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        std::cout << "Received command: '" << command << "'" << std::endl;

        if (command == "HELLO") {
            sendLine(clientSocket, "Hello, client!");
        }
        else if (command == "EXIT") {
            sendLine(clientSocket, "Goodbye!");
            break;
        }
        else {
            sendLine(clientSocket, "Unknown command");
        }
    }

    closesocket(clientSocket);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(5555);

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "Server started on port 5555..." << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        std::cout <<
