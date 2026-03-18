#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<cstdio>
#include<unordered_map>
#include<winsock2.h>
#include<ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class Database {
    private:
        unordered_map<string, string> store;
        string db_filename;
        ofstream logFile;

        //Private helper to load the file when db starts
        void load_from_disk(){
         // ---STARTUP: Rebuild state from log---
        ifstream inFile(db_filename);
        if(inFile.is_open()){
            string fileLine;
            while(getline(inFile, fileLine)){
                stringstream ss(fileLine);
                string command, key, value;
                ss >> command >> key;
                if(command=="SET"){
                    getline(ss>>ws, value);
                    store[key]= value;
                } else if(command=="DELETE"){
                    store.erase(key);
                }
            }
            inFile.close();
            cout<< "Restored database state from disk.\n";
        }
            }

    public:
        //Constructor: Runs when u create a db object
        Database(string filename){
            db_filename = filename;
            load_from_disk();

            logFile.open(db_filename, ios::app);
            if(!logFile.is_open()){
                cout << "(err) Failed to open log file for writing! \n";
            }
        }
        ~Database(){
            if(logFile.is_open()){
                logFile.close();
                cout << "Database safely close. \n";
            }
        }

        void compact(){
            cout<< "Starting log compaction.. \n";

            //1. Close the current log file so the os lets us delete it
            if(logFile.is_open()){
                logFile.close();
            }

            //2. Open a brand new temp file
            string temp_filename = "temp.log";
            ofstream tempFile(temp_filename);

            //3. Iterate through our map and write ONLY the newest, curr values
            for(auto const& [key,val] : store){
                tempFile << "SET " << key << " " << val << "\n";
            }

            //4. Clost the temporary file to save it
            tempFile.close();

            //5. rename the old bloated file and rename the temp file
            std::remove(db_filename.c_str());
            std::rename(temp_filename.c_str(), db_filename.c_str());

            logFile.open(db_filename, ios::app);

            cout<< "Compaction complete! Log file optimized. \n";
        }


        //database operations
        void set(string key, string value){
            store[key] = value;

            //Save to disk
            logFile << "SET " << key << " " << value << "\n"; //Log it
            logFile.flush(); //force save to disk
        }

        string get(string key){
            if(store.find(key)!=store.end()){
                return store[key];
            } else {
                return ""; //Return empty string if not exists
            }
        }

        bool remove(string key){
            // if the key was successfully deleted from memory
            if(store.erase(key)){
                logFile << "DELETE " << key<< "\n";
                logFile.flush();
                return true;
            }
            return false;
        }

        bool update(string key, string value){
            if(store.find(key)!=store.end()){
                store[key] = value;
                logFile << "SET " << key << " " << value<<"\n";
                logFile.flush();
                return true;
            }
            return false; // key doesn't exist
        }
};

int main(){
   // 1. create myDB object from DATABASE class
   Database myDB("database.log");

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
        string networkBuffer = "";

        char buffer[1024];
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

            if (command == "SET") {
                ss >> key;
                getline(ss >> ws, value);
                myDB.set(key, value);
                networkResponse = "OK\r\n";
            } 
            else if (command == "GET") {
                ss >> key;
                string result = myDB.get(key);
                if (result != "") {
                    networkResponse = "\"" + result + "\"\r\n";
                } else {
                    networkResponse = "{nil}\r\n";
                }
            } 
            else if (command == "DELETE") {
                ss >> key;
                if(myDB.remove(key)){
                    networkResponse = "OK\r\n";
                } else {
                    networkResponse = "(err) Key not found\r\n";
                }
            }
            else if (command == "UPDATE") {
                ss >> key;
                string prevValue = myDB.get(key);
                if (prevValue != "") {
                    getline(ss >> ws, value);
                    myDB.update(key, value);
                    networkResponse = "OK (Value updated from " + prevValue + ")\r\n";
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
                cout << "Client requested to exit.\n";
                break; // Break the inner loop to disconnect them
            }
            else {
                networkResponse = "(err) Unknown command: " + command + "\r\n";
            }
              // 3. Send the final response back over the network!
            send(clientSocket, networkResponse.c_str(), networkResponse.length(), 0);

            }
        }

        //close connection after inner loop breaks
        closesocket(clientSocket);
        cout << "Connection close. Waiting for new client...\n";
    }

    closesocket(serverSocket);
    WSACleanup();


    string inputLine;
    cout << "\n";
    cout<< "-----------------------------------------------------------------------------\n";
    cout<< "Welcome to your custom KV-pair DATABASE. Type EXIT to quit \n";
    cout<< "Commands: SET<key><value> | GET<key> | DELETE<key> | UPDATE<key><value> \n";
    cout<< "COMPACT (removes old redundant logs and makes db faster) \n";
    cout<< "-----------------------------------------------------------------------------\n";

    //The REPL (Read Eval Print Loop)

    // while(true){
    //     cout<< "db> ";
    //     getline(cin, inputLine);

    //     if(inputLine == "EXIT" || inputLine == "exit"){
    //         cout<< "Shutting down database....\n";
    //         break;
    //     }

    //     //streamstring for reading words seperated by space
    //     stringstream ss(inputLine);
    //     string command, key, value;

    //     ss >> command;
    //     if(command=="SET"){
    //         ss >> key;
    //         getline(ss >> ws, value);
    //         myDB.set(key, value);
    //         cout<< "OK\n";
    //     }
    //     else if(command=="GET"){
    //         ss >> key;
    //         string value = myDB.get(key);
    //         if(value == ""){
    //             cout << "(err) Key doesn't exist! \n";
    //         } else{
    //             cout << key << " " << value << "\n";
    //         }

    //     }
    //     else if(command=="DELETE"){
    //         ss>>key;
    //         if(myDB.remove(key)){
    //             cout<<"OK \n";
    //         } else {
    //             cout<< "(err) Key not found\n";
    //         }
    //     }
    //     else if(command =="UPDATE"){
    //         ss>> key;
    //         string prevValue = myDB.get(key);

    //         if(prevValue != ""){
    //             cout<< "(prev value): " << prevValue << "\n";

    //             getline(ss >> ws, value);
    //            bool newValueSuccess = myDB.update(key, value);
    //             if(newValueSuccess== true){
    //                 cout << "(new value): " << value << "\n";
    //             } else {
    //                 cout << "Error in updating value \n";
    //             }
    //         } else {
    //             cout << "(err) Key not found \n";
    //         }
    //     }
    //     else if(command=="COMPACT"){
    //         myDB.compact();
    //     }
    //     else{
    //         cout<< "(err) Unknown command: "<< command <<"\n";
    //     }
    // }
    return 0;
}