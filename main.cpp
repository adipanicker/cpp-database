#include<iostream>
#include<string>
#include<sstream>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<thread>
#include<vector>
#include<chrono>
#include "database.h"
#include "wal.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;



int main(){
   // 1. create myDB object from DATABASE class
   Database myDB("database.log");
   WAL wal("wal.log");

    //--- SETUP WINSOCK --- 
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData)!= 0){
        cout << "(err) Winsock failed to start. \n";
        return 1;
    }

    //--- 2. CREATE The socket (The Door) ---
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket==INVALID_SOCKET){
        cout<<"(err) Error creating socket. \n";
        WSACleanup();
        return 1;
    }

    //3. Bind Socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Localhost
    serverAddr.sin_port = htons(8080); //PORT 8080

    if(bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr))==SOCKET_ERROR){
        cout << "(err) Bind failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    //4 Listen for Visitors
    listen(serverSocket, 1);
    cout << "Database Server is RUNNING on port 8080...\n";
    cout << "Waiting for connections..\n";

    // The Server Loop
    while(true){
        // Accept an incoming connection
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET){
            continue;
        }
        cout << "Client connected!\n";

        //Read the message the client sent
        thread([ clientSocket, &myDB, &wal](){

        string networkBuffer = "";
        char buffer[1024];
        bool inTransaction = false;
        int currentTxnId = -1;
        vector<pair<string,string>> txnBuffer; //To store {command, args} fr each txn

        while(true){
            //Clear out old message from buffer
            memset(buffer, 0, sizeof(buffer));

            //Wait for them to type
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

            //if bytesReceived is 0 or less, it means client closed their terminal
            if(bytesReceived<=0){
                cout<< "Client disconnected \n";
                break;
            }
            //convert message to string
            networkBuffer += buffer;

            if(networkBuffer.find('\n')!= string::npos){
                string inputLine = networkBuffer;

                networkBuffer = "";

            //clean up invisible newline chars
            inputLine.erase(inputLine.find_last_not_of(" \n\r\t")+1);
            cout<< "Received command: "<< inputLine << "\n";

            // 1. Set up the string stream to read the network message
            stringstream ss(inputLine);
            string command, key, value, networkResponse;

            // 2. Parse the command
            ss >> command;
            
            auto start = chrono::high_resolution_clock::now();
            //Transaction Commands
            if(command == "BEGIN"){
                if(inTransaction){
                    networkResponse = "(err) Already in a transaction\r\n";
                } else{
                      currentTxnId = wal.beginTransaction();
                    inTransaction = true;
                    txnBuffer.clear();
                    networkResponse = "OK (TXN " + to_string(currentTxnId) + " started)\r\n";
                }
            }
            else if (command == "COMMIT") {
                if (!inTransaction) {
                    networkResponse = "(err) No active transaction\r\n";
                } else {
                    // Apply all buffered commands to the database
                    for (auto& [cmd, args] : txnBuffer) {
                        stringstream argss(args);
                        string k, v;
                        argss >> k;
                        getline(argss >> ws, v);
                        if (cmd == "SET") myDB.set(k, v);
                        else if (cmd == "DELETE") myDB.remove(k);
                        else if (cmd == "UPDATE") myDB.update(k, v);
                    }
                    wal.commitTransaction(currentTxnId);
                    inTransaction = false;
                    currentTxnId = -1;
                    txnBuffer.clear();
                    networkResponse = "OK (committed)\r\n";
                }
            }
            else if (command == "ROLLBACK") {
                if (!inTransaction) {
                    networkResponse = "(err) No active transaction\r\n";
                } else {
                    wal.rollbackTransaction(currentTxnId);
                    inTransaction = false;
                    currentTxnId = -1;
                    txnBuffer.clear();
                    networkResponse = "OK (rolled back)\r\n";
                }
            }

            //Regular Commands
            else if (command == "SET") {
                ss >> key;
                getline(ss >> ws, value);
                if (inTransaction) {
                    wal.logTransaction(currentTxnId, "SET " + key + " " + value);
                    txnBuffer.push_back({"SET", key + " " + value});
                    networkResponse = "OK (buffered in TXN " + to_string(currentTxnId) + ")\r\n";
                } else {
                    myDB.set(key, value);
                    wal.logDirect("SET " + key + " " + value);
                    networkResponse = "OK\r\n";
                }
            }
            else if (command == "GET") {
                ss >> key;
                string result = myDB.get(key);
                networkResponse = result != "" ? "\"" + result + "\"\r\n" : "{nil}\r\n";
            }
            else if (command == "DELETE") {
                ss >> key;
                if (inTransaction) {
                    wal.logTransaction(currentTxnId, "DELETE " + key);
                    txnBuffer.push_back({"DELETE", key});
                    networkResponse = "OK (buffered in TXN " + to_string(currentTxnId) + ")\r\n";
                } else {
                    networkResponse = myDB.remove(key) ? "OK\r\n" : "(err) Key not found\r\n";
                    if (myDB.remove(key)) wal.logDirect("DELETE " + key);
                }
            }
            else if (command == "UPDATE") {
                ss >> key;
                string prevValue = myDB.get(key);
                if (prevValue != "") {
                    getline(ss >> ws, value);
                    if (inTransaction) {
                        wal.logTransaction(currentTxnId, "UPDATE " + key + " " + value);
                        txnBuffer.push_back({"UPDATE", key + " " + value});
                        networkResponse = "OK (buffered in TXN " + to_string(currentTxnId) + ")\r\n";
                    } else {
                        myDB.update(key, value);
                        wal.logDirect("UPDATE " + key + " " + value);
                        networkResponse = "OK (updated from " + prevValue + ")\r\n";
                    }
                } else {
                    networkResponse = "(err) Key not found\r\n";
                }
            }
            else if (command == "COMPACT") {
                myDB.compact();
                networkResponse = "OK (Log compacted)\r\n";
            }
            else if (command == "EXIT" || command == "exit") {
                networkResponse = "Goodbye!\r\n";
                send(clientSocket, networkResponse.c_str(), networkResponse.length(), 0);
                break;
            }
            else {
                networkResponse = "(err) Unknown command: " + command + "\r\n";
            }

            auto end = chrono::high_resolution_clock::now();
            double ms = chrono::duration<double, milli>(end-start).count();

            networkResponse += "Executed in " + to_string(ms) + "ms\r\n";

              // 3. Send the final response back over the network!
            send(clientSocket, networkResponse.c_str(), networkResponse.length(), 0);

            }
        }

        //close connection after inner loop breaks
        closesocket(clientSocket);
        cout << "Connection close. Waiting for new client...\n";
    

     }).detach();
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
    
};