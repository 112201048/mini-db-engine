#pragma once
#include "NodePage.h"
#include <fstream>

using namespace std;

struct IndexMeta {
    uint32_t rootNodeID;
};

constexpr uint32_t INVALID_NODE = UINT32_MAX;

class BPlusDiskTree {
    fstream file;
public:
    BPlusDiskTree(const string& filename);

    uint32_t allocateNode();
    void writeNode(const NodePage& node);
    NodePage readNode(uint32_t nodeID);
    uint32_t readRootID();
    void writeRootID(uint32_t id);
};