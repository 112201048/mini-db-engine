#pragma once
#include <vector>
#include <string>
#include <cstdint>
using namespace std;

//constants that are evaluated at compile time
constexpr uint32_t PAGE_SIZE = 4096; //4KB page size

struct Slot {
    uint16_t offset; // Offset of the record within the page
    uint16_t length; // Length of the record    
};

struct PageHeader {
    uint32_t pageID;       // Unique identifier for the page
    uint16_t numSlots;      // Number of slots in the page
    uint16_t freeSpaceOffset; // Offset to the start of free space
};

class Page {
public:
    Page(uint32_t id);
    bool canFit(uint32_t rowSize) const;
    //get page id
    uint32_t getPageID() const {
        const PageHeader* header = reinterpret_cast<const PageHeader*>(buffer.data());
        return header->pageID;
    }
    uint16_t insertRow(const std::vector<char>& rowData);
    vector<vector<string>> readAllRows() const;
    vector<string> readRow(uint16_t slotID) const;

    const char* data() const { return buffer.data(); }
    char* data() { return buffer.data(); }
private:
    vector<char> buffer;
};