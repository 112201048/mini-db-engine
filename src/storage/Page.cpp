#include "Page.h"
#include <cstring>
#include <stdexcept>
//Structure of the page in memory:
//Header → rows → free space ← slots

//Constructor with a member initializer list
Page::Page(uint32_t id) : buffer(PAGE_SIZE, 0) {
    // Treat the raw bytes at the start of the buffer as if they are PageHeader struct
    PageHeader* header = reinterpret_cast<PageHeader*>(buffer.data());
    header->numSlots = 0;
    header->freeSpaceOffset = sizeof(PageHeader);
    header->pageID = id;
}

bool Page::canFit(uint32_t rowSize) const {
    const PageHeader* header =
        reinterpret_cast<const PageHeader*>(buffer.data());

    uint32_t freeSpace =
        PAGE_SIZE
        - header->freeSpaceOffset
        - header->numSlots * sizeof(Slot);

    return freeSpace >= (rowSize + sizeof(Slot));
}

uint16_t Page::insertRow(const std::vector<char>& rowData) {
    PageHeader* header = reinterpret_cast<PageHeader*>(buffer.data());
    uint32_t rowSize = rowData.size();

    // Insert the row data at the free space offset
    uint16_t rowOffset = header->freeSpaceOffset;
    memcpy(buffer.data() + rowOffset, rowData.data(), rowSize);

    // Update the header
    header->freeSpaceOffset += rowSize;

    // Create a new slot for this row
    Slot* slot = reinterpret_cast<Slot*>(buffer.data() + PAGE_SIZE - (header->numSlots + 1) * sizeof(Slot));
    slot->offset = rowOffset;
    slot->length = rowSize;
    slot->isOccupied = true;

    header->numSlots += 1;

    return header->numSlots - 1;
}

void Page::deleteRow(uint16_t slotID) {
    const PageHeader* header = reinterpret_cast<const PageHeader*>(buffer.data());
    if (slotID >= header->numSlots)
        throw runtime_error("Invalid slot ID");

    Slot* slot = reinterpret_cast<Slot*>(buffer.data() + PAGE_SIZE - (slotID + 1) * sizeof(Slot));

    if (!slot->isOccupied)
        throw runtime_error("Row already deleted");

    slot->isOccupied = false;
}


vector<string> Page::readRow(uint16_t slotID) const {
    const PageHeader* header = reinterpret_cast<const PageHeader*>(buffer.data());
    
    if (slotID >= header->numSlots) {
        throw runtime_error("Invalid slot ID");
    }

    const Slot* slot = reinterpret_cast<const Slot*>(buffer.data() + PAGE_SIZE - (slotID + 1) * sizeof(Slot));
    if (!slot->isOccupied)
        throw runtime_error("Attempt to read deleted row");
    const char* rowData = buffer.data() + slot->offset;
    uint16_t rowLength = slot->length;

    // Parse the row data
    vector<string> row;
    size_t bytesRead = 0;
    while (bytesRead < rowLength) {
        uint32_t colSize;
        memcpy(&colSize, rowData + bytesRead, sizeof(uint32_t));
        bytesRead += sizeof(uint32_t);

        string colData(rowData + bytesRead, colSize);
        bytesRead += colSize;

        row.push_back(colData);
    }

    return row;
}

vector<vector<string>> Page::readAllRows() const {
    const PageHeader* header = reinterpret_cast<const PageHeader*>(buffer.data());
    vector<vector<string>> allRows;

    for (uint16_t i = 0; i < header->numSlots; ++i) {
        const Slot* slot = reinterpret_cast<const Slot*>(buffer.data() + PAGE_SIZE - (i + 1) * sizeof(Slot));
        const char* rowData = buffer.data() + slot->offset;
        uint16_t rowLength = slot->length;

        // Parse the row data
        vector<string> row;
        size_t bytesRead = 0;
        while (bytesRead < rowLength) {
            uint32_t colSize;
            memcpy(&colSize, rowData + bytesRead, sizeof(uint32_t));
            bytesRead += sizeof(uint32_t);

            string colData(rowData + bytesRead, colSize);
            bytesRead += colSize;

            row.push_back(colData);
        }
        allRows.push_back(row);
    }

    return allRows;
}
