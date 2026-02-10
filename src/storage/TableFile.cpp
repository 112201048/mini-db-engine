//Implementation of the TableFile class that manages table data storage on disk.

#include "TableFile.h"
#include "index/BPlusTree.h"
#include "Page.h"
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <cstring>

TableFile::TableFile(const string& filename){
    index = new BPlusTree(3, filename + "_index.db");
    file.open(filename, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        // If the file does not exist, create it
        file.clear();
        file.open(filename, ios::out | ios::binary);
        file.close();
        file.open(filename, ios::in | ios::out | ios::binary);
    }
    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    size_t numPages = fileSize / PAGE_SIZE;
    if (fileSize % PAGE_SIZE != 0) throw runtime_error("Corrupt table file: partial page detected");
    file.seekg(0, ios::beg);
    for (uint32_t i = 0; i < numPages; ++i) {
        Page page = readPageFromDisk(i);
        pages.push_back(page);
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

vector<string> TableFile::getRow(const RID& rid) {
    if (rid.pageID >= pages.size()) {
        throw runtime_error("Invalid RID: pageID out of bounds");
    }
    Page& page = pages[rid.pageID];
    return page.readRow(rid.slotID);
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
    RID rid = {page->getPageID(), slotID};
    Key key = extractKeyFromRow(row);   // decide which column is indexed
    index->insert(key, rid);
    return rid;
}

void TableFile::deleteByKey(Key k) {
    RID rid;

    if (!index->search(k, rid))
        throw runtime_error("Key not found");
    pages[rid.pageID].deleteRow(rid.slotID);

    writePageToDisk(&pages[rid.pageID]);

    index->remove(k);
}


Key TableFile::extractKeyFromRow(const vector<string>& row) {
    int indexedColumn = 0;      // for now, hardcode
    return stoi(row[indexedColumn]);
}

vector<string> TableFile::findByKey(Key k) {
    RID rid;
    if (index->search(k, rid))
        return getRow(rid);
    throw runtime_error("Key not found");
}

vector<vector<string>> TableFile::rangeQuery(Key low, Key high) {
    auto rids = index->rangeScan(low, high);

    vector<vector<string>> result;
    for (auto &r : rids)
        result.push_back(getRow(r));

    return result;
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

Page TableFile::readPageFromDisk(uint32_t pageID) {
    Page page(pageID);
    file.seekg(pageID * PAGE_SIZE, ios::beg);
    if (!file.read(page.data(), PAGE_SIZE)) {
        throw runtime_error("Failed to read page from disk");
    }
    return page;
}