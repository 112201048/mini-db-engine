#include <vector>
#include <cstdint>
#include "storage/TableFile.h"

using namespace std;

using Key = int32_t;

struct BPlusNode {
    bool isLeaf;
    vector<Key> keys;
    vector<BPlusNode*> children; // For internal nodes
    vector<RID> rids;             // For leaf nodes
    BPlusNode* next;        // Pointer to next leaf node

    BPlusNode(bool leaf) : isLeaf(leaf), next(nullptr) {}
};