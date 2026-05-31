#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <atomic>

using namespace std;

class WAL {
    private:
        string wal_filename;
        ofstream walFile;
        atomic<int> txnCounter;

        string getCurrentTimestamp();
    
    public:
        WAL(string filename);
        ~WAL();

        //Write single line directly to WAL (for non-transaction commands)
        void logDirect(const string& command);

        //Transaction methods
        int beginTransaction(); // returns txn ID
        void logTransaction(int txnId, const string& command); // log a command under a txn
        void commitTransaction(int txnId); //write commit to WAL
        void rollbackTransaction(int txnId); // write ROLLBACK to WAL

        //For .wal display command
        string readWAL();
};