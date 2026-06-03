#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    // Setup Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "(err) Winsock failed to start.\n";
        return 1;
    }

    // Create socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cout << "(err) Error creating socket.\n";
        WSACleanup();
        return 1;
    }

    // Connect to server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(8080);

    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "(err) Could not connect to server. Is cpp-db running?\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "cpp-db v1.0.0\n";
    cout << "Connected to server on port 8080\n";
    cout << "Type .help for available commands\n\n";

    string input;
    char buffer[4096];

    while (true) {
        // Show prompt
        cout << "cpp-db> ";
        getline(cin, input);

        if (input.empty()) continue;

        // Send to server with newline
        string message = input + "\n";
        send(clientSocket, message.c_str(), message.length(), 0);

        // If exit, break before waiting for response
        if (input == "EXIT" || input == "exit") {
            cout << "Goodbye!\n";
            break;
        }

        // Receive response
        string response = "";
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived <= 0) {
                cout << "(err) Server disconnected.\n";
                goto cleanup;
            }
            response += buffer;
            // Stop reading when we get the execution time line
            if (response.find("Executed in") != string::npos) break;
            // Or if it ends with \r\n and no more data coming
            if (bytesReceived < sizeof(buffer) - 1) break;
        }

        cout << response << "\n";
    }

cleanup:
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}