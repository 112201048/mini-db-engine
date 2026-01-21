# Mini DB Engine

A minimal database storage engine written in C++ to understand how real databases store data on disk.
This project focuses on the **storage layer** and is being built incrementally, one concept at a time.

## Current Status

Implemented features:
- Fixed size pages (4 KB)
- Variable length row storage
- Slot directory
- Row Identifiers (RID)
- Page based inserts and table scans
- In memory page management
- Binary serialization
- Pages written to disk

At this stage, pages are kept in memory during execution. Pages in the disk do not get reloaded on startup. Deletes, updates, and indexing are not yet supported.

## Storage Layout

Each page is organised as:

```
+---------------------+
|      Page Header    |
+---------------------+
|      Stored Rows    |
+---------------------+
|      Free Space     |
+---------------------+
|     Slot Directory  |
+---------------------+
```

Rows are identified by a Row Identifier (RID) consisting of a page ID and slot ID.
Detailed storage design can be found in [docs/storage.md](docs/storage.md).

## Build and run

```bash
cd src
g++ -std=c++17 main.cpp src/storage/Page.cpp src/storage/TableFile.cpp -o main
./main
```