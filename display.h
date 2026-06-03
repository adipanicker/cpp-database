#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <sstream>

using namespace std;

class Display {
public:
    // Formats the entire key-value store as a bordered table
    static string formatStore(unordered_map<string, string>& store);

    // Formats WAL log contents nicely
    static string formatWAL(const string& walContents);

    // Formats a single GET result as a small table
    static string formatGet(const string& key, const string& value);

    // Prints the help menu
    static string formatHelp();
};