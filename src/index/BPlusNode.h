#pragma once
#include <vector>
#include <cstdint>
#include "include/Common.h"

using namespace std;

struct BPlusNode {
    uint32_t nodeID;
    bool isLeaf;
    vector<Key> keys;
    vector<BPlusNode*> children; // For internal nodes
    vector<RID> rids;             // For leaf nodes
    BPlusNode* next;        // Pointer to next leaf node

    BPlusNode(bool leaf) : isLeaf(leaf), next(nullptr) {}
};