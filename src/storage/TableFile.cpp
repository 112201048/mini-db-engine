//Implementation of the TableFile class that manages table data storage on disk.

#include "TableFile.h"
#include "Page.h"
#include <stdexcept>
#include <cstdint>
#include <vector>
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

vector<char> serializeRow(const vector<string>& row) {
    // Calculate total size
    uint32_t totalSize = 0;
    for (const auto& col : row) {
        totalSize += sizeof(uint32_t); // for column size
        totalSize += col.size();     // for column data
    }

    vector<char> buffer(totalSize);
    size_t offset = 0;
    for (const auto& col : row) {
        uint32_t colSize = col.size();
        memcpy(buffer.data() + offset, &colSize, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(buffer.data() + offset, col.data(), colSize);
        offset += colSize;
    }
    return buffer;
}

RID TableFile::insertRow(const vector<string>& row) {
    auto rowData = serializeRow(row);

    Page* page = getLastPage();
    if (!page || !page->canFit(rowData.size())) {
        // Need to create a new page
        page = createNewPage();
    }
    uint16_t slotID = page->insertRow(rowData);
    writePageToDisk(page);
    return {page->getPageID(), slotID};
}

vector<vector<string>> TableFile::scanAll() {
    vector<vector<string>> result;

    for(auto& page : pages) {
        auto rows = page.readAllRows();
        result.insert(result.end(), rows.begin(), rows.end());
    }
    return result;
}

Page* TableFile::getLastPage() {
    if (pages.empty()) return nullptr;
    return &pages.back();
}

Page* TableFile::createNewPage() {
    uint32_t newPageID = pages.size();
    pages.emplace_back(newPageID);
    return &pages.back();
}

void TableFile::writePageToDisk(Page* page) {
    uint32_t pageID = page->getPageID();
    file.seekp(pageID * PAGE_SIZE, ios::beg);
    file.write(page->data(), PAGE_SIZE);
    file.flush();
}