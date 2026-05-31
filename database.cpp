#include "database.h"

void Database::load_from_disk(){
    ifstream inFile(db_filename);
    if (inFile.is_open()){
        string fileLine;

        while(getline(inFile, fileLine)){
            stringstream ss(fileLine);
            string command, key, value;
            ss >> command >> key;
            if(command == "SET"){
                getline(ss >> ws, value);
                store[key] = value;
            } else if(command == "DELETE"){
                store.erase(key);
            }
        }
        inFile.close();
        cout<< "Restored database state from disk. \n";
    }
}

Database::Database(string filename){
    db_filename = filename;
    load_from_disk();
    logFile.open(db_filename, ios::app);
    if(!logFile.is_open()){
        cout<< "(err) Failed to open log file for writing!\n";
    }
}

Database::~Database(){
    if(logFile.is_open()){
        logFile.close();
        cout<<"Database safely closed. \n";
    }
}

void Database::set(string key, string value){
    store[key] = value;
    logFile << "SET " << key << " " << value<< "\n";
    logFile.flush();
}

string Database::get(string key){
    if(store.find(key)!= store.end()){
        return store[key];
    }
    return "";
}

bool Database::remove(string key){
    if(store.erase(key)){
        logFile << "DELETE " << key << "\n";
        logFile.flush();
        return true;
    }
    return false;
}

bool Database::update(string key, string value){
    if(store.find(key)!=store.end()){
        store[key] = value;
        logFile << "SET " << key << " " << value << "\n";
        logFile.flush();
        return true;
    }
    return false;
}

void Database::compact(){
    cout << "Starting log compaction...\n";
    if(logFile.is_open()) logFile.close();

    string temp_filename = "temp.log";
    ofstream tempFile(temp_filename);

    for(auto const& [key,value]:store){
        tempFile << "SET " << key << " " << value << "\n";
    }
    tempFile.close();

    std::remove(db_filename.c_str());
    std::rename(temp_filename.c_str(), db_filename.c_str());
    logFile.open(db_filename, ios::app);
    cout<< "Compaction complete! Log file optimized. \n";
}

unordered_map<string, string>& Database::getStore(){
    return store;
}