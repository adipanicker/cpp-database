#include "display.h"

string Display::formatStore(unordered_map<string, string>& store) {
    if (store.empty()) {
        return "(empty) No keys found.\r\n";
    }

    // Find the longest key and value for column width
    size_t maxKey = 3;   // minimum "Key"
    size_t maxVal = 5;   // minimum "Value"
    for (auto const& [key, val] : store) {
        if (key.length() > maxKey) maxKey = key.length();
        if (val.length() > maxVal) maxVal = val.length();
    }

    // Build the border line
    string border = "+-" + string(maxKey, '-') + "-+-" + string(maxVal, '-') + "-+\r\n";

    string result = "";
    result += border;
    string headerKey = string("Key") + string(maxKey - 3, ' ');
    string headerVal = string("Value") + string(maxVal - 5, ' ');
    result += "| " + headerKey + " | " + headerVal + " |\r\n";

   for (auto const& [key, val] : store) {
    string paddedKey = key + string(maxKey - key.length(), ' ');
    string paddedVal = val + string(maxVal - val.length(), ' ');
    result += "| " + paddedKey + " | " + paddedVal + " |\r\n";
}

    result += border;
    result += "[" + to_string(store.size()) + " key(s)]\r\n";

    return result;
}

string Display::formatGet(const string& key, const string& value) {
    size_t maxKey = max(key.length(), (size_t)3);
    size_t maxVal = max(value.length(), (size_t)5);

    string border = "+-" + string(maxKey, '-') + "-+-" + string(maxVal, '-') + "-+\r\n";

    string result = "";
    result += border;
    result += "| " + string("Key") + string(maxKey - 3, ' ') + " | " + string("Value") + string(maxVal - 5, ' ') + " |\r\n";
    result += border;
    result += "| " + key + string(maxKey - key.length(), ' ') + " | " + value + string(maxVal - value.length(), ' ') + " |\r\n";
    result += border;

    return result;
}

string Display::formatWAL(const string& walContents) {
    if (walContents == "(empty)\n") {
        return "(empty) WAL has no entries yet.\r\n";
    }

    string result = "";
    result += "+-----------------------------------------------+\r\n";
    result += "| WAL - Write Ahead Log                         |\r\n";
    result += "+-----------------------------------------------+\r\n";

    stringstream ss(walContents);
    string line;
    while (getline(ss, line)) {
        if (line.empty()) continue;
        // pad or trim to fit
        if (line.length() > 53) line = line.substr(0, 53);
        result += "| " + line + string(53 - line.length(), ' ') + " |\r\n";
    }

    result += "+-----------------------------------------------+\r\n";
    return result;
}

string Display::formatHelp() {
    string result = "";
    result += "+------------------+----------------------------------------+\r\n";
    result += "| Command          | Description                            |\r\n";
    result += "+------------------+----------------------------------------+\r\n";
    result += "| SET key value    | Store a key-value pair                 |\r\n";
    result += "| GET key          | Retrieve value by key                  |\r\n";
    result += "| DELETE key       | Delete a key                           |\r\n";
    result += "| UPDATE key value | Update an existing key                 |\r\n";
    result += "| COMPACT          | Optimize the database log              |\r\n";
    result += "| BEGIN            | Start a transaction                    |\r\n";
    result += "| COMMIT           | Commit current transaction             |\r\n";
    result += "| ROLLBACK         | Rollback current transaction           |\r\n";
    result += "| .keys            | Show all keys in a table               |\r\n";
    result += "| .wal             | Show Write-Ahead Log                   |\r\n";
    result += "| .help            | Show this help menu                    |\r\n";
    result += "| EXIT             | Disconnect                             |\r\n";
    result += "+------------------+----------------------------------------+\r\n";
    return result;
}