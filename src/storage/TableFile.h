#ifndef TABLEFILE_H
#define TABLEFILE_H

//Table File Abstraction
//Each TableFile represents a file on disk that stores table data.
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <BPlusTree.h>
using namespace std;

class Page; //forward declaration

struct RID {
    uint32_t pageID;
    uint16_t slotID;
};

class TableFile {
public:
    TableFile(const string& filename);
    ~TableFile();
    BPlusTree index;
    RID insertRow(const vector<string>& row);
    vector<string> getRow(const RID& rid);
    vector<vector<string>> scanAll();
    vector<string> findByKey(Key k);
    vector<vector<string>> rangeQuery(Key low, Key high);
private:
    fstream file;
    Page* getLastPage();
    Page* createNewPage();
    void writePageToDisk(Page* page);
    Page readPageFromDisk(uint32_t pageID);

    vector<Page> pages;
};

#endif // TABLEFILE_H