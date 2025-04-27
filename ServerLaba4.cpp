#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <random>
#include <climits>

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

void generateAndComputeMatrix(SOCKET clientSocket, int size, int threadsCount) {
    std::vector<std::vector<int>> matrix(size, std::vector<int>(size));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 99);

    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            matrix[i][j] = dis(gen);

    std::vector<int> columnMins(size, INT_MAX);
    std::mutex progressMutex;
    int completedCols = 0;

    auto worker = [&](int start, int end) {
        for (int j = start; j < end; ++j) {
            int minVal = matrix[0][j];
            for (int i = 1; i < size; ++i) {
                if (matrix[i][j] < minVal) minVal = matrix[i][j];
            }
            columnMins[j] = minVal;

            {
                std::lock_guard<std::mutex> lock(progressMutex);
                completedCols++;
                int percent = (completedCols * 100) / size;
                sendLine(clientSocket, "STATUS " + std::to_string(percent));
                Sleep(10);
            }
        }
        };

    std::vector<std::thread> threads;
    int colsPerThread = size / threadsCount;
    for (int t = 0; t < threadsCount; ++t) {
        int start = t * colsPerThread;
        int end = (t == threadsCount - 1) ? size : start + colsPerThread;
        threads.emplace_back(worker, start, end);
    }

    for (auto& t : threads) t.join();

    for (int i = 0; i < size; ++i)
        matrix[i][size - 1 - i] = columnMins[size - 1 - i];

    sendLine(clientSocket, "DONE");

    for (int i = 0; i < size; ++i) {
        std::ostringstream row;
        for (int j = 0; j < size; ++j)
            row << matrix[i][j] << " ";
        sendLine(clientSocket, row.str());
    }
}

void processClient(SOCKET clientSocket) {
    int size = 0;
    int threadsCount = 1;

    while (true) {
        std::string cmd = recvLine(clientSocket);

        if (cmd.rfind("CONFIG", 0) == 0) {
            std::istringstream iss(cmd);
            std::string temp;
            iss >> temp >> size >> threadsCount;
            sendLine(clientSocket, "CONFIG_OK");
            std::cout << "Received CONFIG: Size = " << size << ", Threads = " << threadsCount << std::endl;
        }
        else if (cmd == "START") {
            if (size <= 0) {
                sendLine(clientSocket, "ERROR: CONFIG FIRST");
                continue;
            }
            generateAndComputeMatrix(clientSocket, size, threadsCount);
        }
        else if (cmd == "EXIT") {
            break;
        }
        else {
            sendLine(clientSocket, "UNKNOWN_COMMAND");
        }
    }

    closesocket(clientSocket);
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "Client disconnected\n";
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

    std::cout << "Server started on port 5555...\n";

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "New client connected\n";
        std::thread(processClient, clientSocket).detach();
    }

    WSACleanup();
    return 0;
}
