#include "wal.h"

string WAL::getCurrentTimestamp(){
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm);
    return string(buf);
}

WAL::WAL(string filename){
    wal_filename = filename;
    txnCounter = 0;
    walFile.open(wal_filename, ios::app);
    if (!walFile.is_open()){
        cout << "(err) Failed to open WAL file for writing! \n";
    } else {
        cout << "WAL initialized: " << wal_filename << "\n";
    }
}

WAL::~WAL() {
    if(walFile.is_open()){
        walFile.close();
    }
}

void WAL::logDirect(const string& command){
    if(walFile.is_open()){
        walFile << getCurrentTimestamp() << " " << command << "\n";
        walFile.flush();
    }
}

int WAL::beginTransaction(){
    int txnId = ++txnCounter;
    if(walFile.is_open()){
        walFile << getCurrentTimestamp() << " [TXN " << txnId << "] BEGIN\n";
        walFile.flush();
    }
    cout<< "Transaction " << txnId << " started. \n";
    return txnId;
}

void WAL::logTransaction(int txnId, const string& command){
    if(walFile.is_open()){
        walFile << getCurrentTimestamp() << " [TXN " << txnId << "] " << command << "\n";
        walFile.flush();
    }
    cout << "Transaction " <<txnId << " commited. \n";
}

void WAL::commitTransaction(int txnId) {
    if (walFile.is_open()) {
        walFile << getCurrentTimestamp() << " [TXN " << txnId << "] COMMIT\n";
        walFile.flush();
    }
    cout << "Transaction " << txnId << " committed.\n";
}

void WAL::rollbackTransaction(int txnId){
    if(walFile.is_open()){
        walFile << getCurrentTimestamp() << " [TXN " << txnId << "] ROLLBACK\n";
        walFile.flush();
    }
    cout << "Transaction " << txnId << " rolled back. \n";
}

string WAL::readWAL(){
    ifstream inFile(wal_filename);
    if(!inFile.is_open()){
        return "(err) Could not open WAL file. \n";
    }
    stringstream ss;
    string line;
    int lineCount = 0;
    while(getline(inFile, line)){
        ss << line << "\n";
        lineCount++;
    }
    inFile.close();
    if(lineCount == 0) return "(empty)\n";
    return ss.str();
}

