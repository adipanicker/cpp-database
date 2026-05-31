#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <unordered_map>

using namespace std;

class Database{
    private:
        unordered_map<string,string> store;
        string db_filename;
        ofstream logFile;

        void load_from_disk();

    public:
        Database(string filename);
        ~Database();

        void set(string key, string value);
        string get(string key);
        bool remove(string key);
        bool update(string key, string value);
        void compact();

        unordered_map<string, string>& getStore();
};