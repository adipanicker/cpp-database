# cpp-db

A lightweight key-value database engine built from scratch in C++. No external libraries — just raw C++, file I/O, and TCP networking.

---

## Features

- **Persistent Storage** — data survives restarts via a Write-Ahead Log (WAL)
- **ACID-style Transactions** — `BEGIN` / `COMMIT` / `ROLLBACK` support
- **Log Compaction** — removes redundant WAL entries to keep the database lean
- **TCP Server** — remote clients connect and run commands over the network
- **Multi-client Support** — multiple clients handled simultaneously via multithreading
- **Custom CLI Client** — clean `cpp-db>` prompt for interacting with the server
- **Execution Timing** — every command reports how long it took to execute

---

## Project Structure

```
cpp-db/
├── main.cpp          # Entry point, TCP server loop
├── database.h        # Database class declaration
├── database.cpp      # Database implementation (store, WAL, compaction)
├── wal.h             # WAL & Transaction class declaration
├── wal.cpp           # WAL implementation (logging, transactions, timestamps)
├── display.h         # Display class declaration
├── display.cpp       # Pretty terminal output formatting
└── client.cpp        # CLI client with cpp-db> prompt
```

---

## Getting Started

### Prerequisites
- Windows (uses Winsock2 for networking)
- MinGW / g++ with C++17 support

### Build

**Server:**
```bash
g++ main.cpp database.cpp wal.cpp display.cpp -o cpp-db -lws2_32 -std=c++17
```

**Client:**
```bash
g++ client.cpp -o client -lws2_32 -std=c++17
```

Or use the build script:
```bash
build.bat
```

### Run

Terminal 1 — start the server:
```bash
./cpp-db
```

Terminal 2 — connect with the client:
```bash
./client
```

---

## Commands

| Command            | Description                          |
|--------------------|--------------------------------------|
| `SET key value`    | Store a key-value pair               |
| `GET key`          | Retrieve value by key                |
| `DELETE key`       | Delete a key                         |
| `UPDATE key value` | Update an existing key               |
| `COMPACT`          | Optimize the database log            |
| `BEGIN`            | Start a transaction                  |
| `COMMIT`           | Commit current transaction           |
| `ROLLBACK`         | Rollback current transaction         |
| `.keys`            | Show all stored keys in a table      |
| `.wal`             | Show Write-Ahead Log                 |
| `.help`            | Show help menu                       |
| `EXIT`             | Disconnect                           |

---

## How It Works

### Write-Ahead Log (WAL)
Every write operation is logged to `wal.log` before being applied to memory. On startup, the database replays the log to restore its last known state. This ensures data durability — if the server crashes mid-operation, no committed data is lost.

### Transactions
Commands between `BEGIN` and `COMMIT` are buffered in memory and only applied to the database atomically on `COMMIT`. A `ROLLBACK` discards the buffer entirely. Each transaction gets a unique ID and timestamped entries in the WAL.

```
cpp-db> BEGIN
OK (TXN 1 started)

cpp-db> SET name Aditya
OK (buffered in TXN 1)

cpp-db> SET city Bangalore
OK (buffered in TXN 1)

cpp-db> COMMIT
OK (committed)
```

### Log Compaction
Over time the WAL grows with redundant entries (e.g. a key SET multiple times). `COMPACT` rewrites the log keeping only the current value of each key, reducing file size and improving startup time.

### Multi-client Threading
Each incoming TCP connection is handed off to a new `std::thread`, leaving the main loop free to immediately accept the next client. Each thread maintains its own transaction state independently.

---

## Example Session

```
cpp-db v1.0.0
Connected to server on port 8080
Type .help for available commands

cpp-db> SET name Aditya
OK
Executed in 2.59ms

cpp-db> GET name
+---------+--------+
| Key     | Value  |
+---------+--------+
| name    | Aditya |
+---------+--------+
Executed in 0.02ms

cpp-db> .keys
+--------+--------+
| Key    | Value  |
+--------+--------+
| name   | Aditya |
| city   | Bangalore|
+--------+--------+
[2 key(s)]
Executed in 0.03ms

cpp-db> .wal
+-------------------------------------------------------+
| WAL - Write Ahead Log                                 |
+-------------------------------------------------------+
| 2026-06-03 10:05:29 SET name Aditya                   |
| 2026-06-03 10:05:40 [TXN 1] BEGIN                     |
| 2026-06-03 10:05:48 [TXN 1] SET city Bangalore         |
| 2026-06-03 10:06:00 [TXN 1] COMMIT                    |
+-------------------------------------------------------+
Executed in 0.48ms
```

---

## What I Learned

- Why databases flush to disk **before** acknowledging a write — lose power after the ACK but before the flush and your data is gone. WAL solves exactly this.
- How TCP servers manage concurrent connections using threading
- How log compaction works in systems like LevelDB and RocksDB
- Implementing atomic transactions without an existing framework

---

## Roadmap

- [ ] TTL (Time To Live) — auto-expiring keys
- [ ] AUTH command — password-protected connections  
- [ ] Persistence format upgrade — binary encoding for faster reads
- [ ] Cross-platform support (Linux/macOS via POSIX sockets)
