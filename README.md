# AOLDB - Custom C++ Database Engine

A lightweight, custom-built database engine developed entirely from scratch in C++. This project demonstrates core database concepts, including interactive command parsing and persistent disk storage to ensure data integrity.

## 🚀 Features

* **Custom REPL Interface:** A built-in Read-Eval-Print Loop (REPL) that allows users to interactively enter and execute queries directly from the command line.
* **Write-Ahead Logging (WAL):** Ensures reliable persistent disk storage and crash recovery by logging all modifications before they are applied to the database state.
* **Modular Architecture:** Built using clean Object-Oriented Programming (OOP) principles, separating the storage engine, query parser, and interface into distinct, maintainable classes.
* **Zero External Dependencies:** Built using standard C++ libraries and raw file I/O operations.

## 🛠️ Tech Stack

* **Language:** C++
* **Concepts:** Object-Oriented Design, REPL, Database Persistence
* **Storage Mechanism:** Custom File I/O & Write-Ahead Log

## ⚙️ Getting Started

### Prerequisites
You will need a C++ compiler (like GCC or Clang) installed on your system. 

### Installation & Build
1. Clone the repository:
   ```bash
   git clone [https://github.com/yourusername/aoldb.git](https://github.com/yourusername/aoldb.git)
   cd aoldb
2. Compile the source code: g++ -std=c++17 src/*.cpp -o aoldb
3. Running the Database: Execute the compiled binary to start the REPL: ./aoldb

## 💻 Usage

Once the REPL is running, you can interact directly with the database. 

```bash
aoldb> INSERT 1 "user_data"
Executed.
aoldb> SELECT *
...
aoldb> EXIT

