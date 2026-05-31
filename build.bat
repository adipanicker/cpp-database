@echo off
g++ main.cpp database.cpp wal.cpp display.cpp -o cpp-db -lws2_32 -std=c++17
echo Build complete!