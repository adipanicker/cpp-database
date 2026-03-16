#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<unordered_map>

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

    string inputLine;
    cout << "\n";
    cout<< "-----------------------------------------------------------------------------\n";
    cout<< "Welcome to your custom KV-pair DATABASE. Type EXIT to quit \n";
    cout<< "Commands: SET<key><value> | GET<key> | DELETE<key> | UPDATE<key><value> \n";
    cout<< "-----------------------------------------------------------------------------\n";

    //The REPL (Read Eval Print Loop)
    while(true){
        cout<< "db> ";
        getline(cin, inputLine);

        if(inputLine == "EXIT" || inputLine == "exit"){
            cout<< "Shutting down database....\n";
            break;
        }

        //streamstring for reading words seperated by space
        stringstream ss(inputLine);
        string command, key, value;

        ss >> command;
        if(command=="SET"){
            ss >> key;
            getline(ss >> ws, value);
            myDB.set(key, value);
            cout<< "OK\n";
        }
        else if(command=="GET"){
            ss >> key;
            string value = myDB.get(key);
            if(value == ""){
                cout << "(err) Key doesn't exist! \n";
            } else{
                cout << key << " " << value << "\n";
            }

        }
        else if(command=="DELETE"){
            ss>>key;
            if(myDB.remove(key)){
                cout<<"OK \n";
            } else {
                cout<< "(err) Key not found\n";
            }
        }
        else if(command =="UPDATE"){
            ss>> key;
            string prevValue = myDB.get(key);

            if(prevValue != ""){
                cout<< "(prev value): " << prevValue << "\n";

                getline(ss >> ws, value);
               bool newValueSuccess = myDB.update(key, value);
                if(newValueSuccess== true){
                    cout << "(new value): " << value << "\n";
                } else {
                    cout << "Error in updating value \n";
                }
            } else {
                cout << "(err) Key not found \n";
            }
        }
        else{
            cout<< "(err) Unknown command: "<< command <<"\n";
        }
    }
    return 0;
}