#pragma once
#include <vector>
#include <cstdint>
#include "include/Common.h"

using namespace std;

constexpr uint32_t INDEX_PAGE_SIZE = 4096;

struct NodeHeader {
    uint32_t nodeID;
    bool isLeaf;
    uint16_t numKeys;
    uint32_t nextLeaf;
};

struct NodePage {
    NodeHeader header;
    vector<Key> keys;
    vector<uint32_t> children; // internal nodes
    vector<RID> rids;          // leaf nodes
};
