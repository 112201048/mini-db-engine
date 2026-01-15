//Implementation of the TableFile class that manages table data storage on disk.

#include "TableFile.h"
#include <cstdint>
#include <cstring>

TableFile::TableFile(const string& filename) {
    file.open(filename, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        // If the file does not exist, create it
        file.clear();
        file.open(filename, ios::out | ios::binary);
        file.close();
        file.open(filename, ios::in | ios::out | ios::binary);
    }
}

TableFile::~TableFile() {
    if (file.is_open()) {
        file.close();
    }
}

// Serialize the row into a buffer and write to file
// overall layout of the file looks like it [row1][row2][row3][row4]...
// each row is stored as [rowSize] [col1_size][col1_data] [col2_size][col2_data] [col3_size][col3_data]
void TableFile::insertRow(const vector<string>& row) {
    if (file.is_open()) {
        //moves the put pointer to the end of the file
        file.seekp(0, std::ios::end);
        // Calculate total row size
        uint32_t rowSize = 0;
        for (const auto& col : row) {
            rowSize += sizeof(uint32_t); // for column size
            rowSize += col.size();     // for column data
        }
        // Write row size
        file.write(reinterpret_cast<const char*>(&rowSize), sizeof(uint32_t));
        // Write each column
        for (const auto& col : row) {
            uint32_t colSize = col.size();
            file.write(reinterpret_cast<const char*>(&colSize), sizeof(uint32_t));
            file.write(col.data(), colSize);
        }
        file.flush(); // Ensure data is written to disk
    }
    else{
        // Handle error: file not open
        throw runtime_error("File not open");
    }
}

vector<vector<string>> TableFile::readAllRows() {
    vector<vector<string>> allRows;
    if (file.is_open()) {
        // moves the get pointer to the beginning of the file
        file.seekg(0, ios::beg);
        //while the next character is not end of file
        while (true) {
            uint32_t rowSize = 0;
            if (!file.read(reinterpret_cast<char*>(&rowSize), sizeof(uint32_t))) {
                break; // EOF or read failure
            }
            // total size of the row has been read, now read each column
            // read rowSize bytes and store to a buffer
            vector<char> rowBuffer(rowSize);
            if (!file.read(rowBuffer.data(), rowSize)) {
                throw runtime_error("Corrupted row data");
            }
            // parse the row buffer
            vector<string> row;
            size_t bytesReadFromBuffer = 0;
            while (bytesReadFromBuffer < rowSize) {
                uint32_t colSize;
                memcpy(&colSize, rowBuffer.data() + bytesReadFromBuffer, sizeof(uint32_t));
                bytesReadFromBuffer += sizeof(uint32_t);

                string colData(rowBuffer.data() + bytesReadFromBuffer, colSize);
                bytesReadFromBuffer += colSize;

                row.push_back(colData);
            }
            allRows.push_back(row);
        }
    }
    else {
        // Handle error: file not open
        throw runtime_error("File not open");
    }
    return allRows;
}
