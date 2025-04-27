#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

std::mutex coutMutex;

void sendLine(SOCKET client, const std::string& msg) {
    std::string data = msg + "\n";
    send(client, data.c_str(), data.size(), 0);
}

std::string recvLine(SOCKET client) {
    char buffer[1024];
    std::string result;
    int bytes;
    while ((bytes = recv(client, buffer, 1, 0)) > 0) {
        if (buffer[0] == '\n') break;
        result += buffer[0];
    }
    return result;
}

void configureClient(SOCKET clientSocket) {
    int size, threadsCount;

    std::cout << "Enter matrix size: ";
    std::cin >> size;

    std::cout << "Enter number of threads: ";
    std::cin >> threadsCount;

    std::string configCommand = "CONFIG " + std::to_string(size) + " " + std::to_string(threadsCount);
    sendLine(clientSocket, configCommand);

    std::string response = recvLine(clientSocket);
    std::cout << "Server response: '" << response << "'" << std::endl;

    if (response == "CONFIG_OK") {
        std::cout << "Configuration successful!" << std::endl;
    }
    else {
        std::cout << "Failed to configure!" << std::endl;
    }
}

void startComputation(SOCKET clientSocket) {
    sendLine(clientSocket, "START");
    std::string response = recvLine(clientSocket);
    if (response == "ERROR: CONFIG FIRST") {
        std::cout << "Please configure first!" << std::endl;
        return;
    }

    std::cout << "Computation started!" << std::endl;

    while (true) {
        response = recvLine(clientSocket);
        if (response.rfind("STATUS", 0) == 0) {
            std::cout << response << std::endl;
        }
        else if (response == "DONE") {
            std::cout << "Computation completed!" << std::endl;
            break;
        }
    }

    std::cout << "Receiving result..." << std::endl;
    while (true) {
        std::string row = recvLine(clientSocket);
        if (row.empty()) break;
        std::cout << row << std::endl;
    }
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = htons(5555);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed!" << std::endl;
        return 1;
    }

    std::cout << "Connected to the server!" << std::endl;

    configureClient(clientSocket);
    startComputation(clientSocket);

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
