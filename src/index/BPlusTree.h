#pragma once
#include "include/Common.h"
#include <string>
#include <vector>

using namespace std;

class BPlusDiskTree; // forward declaration
class BPlusNode; // forward declaration

class BPlusTree {
public:
    BPlusTree(int order, string filename);

    void insert(Key key, const RID& rid);
    bool search(Key key, RID& rid);
    vector<RID> rangeScan(Key low, Key high);
private:
    BPlusDiskTree* file;
    BPlusNode* root;
    int order;

    void insertInternal(BPlusNode* node, Key key, BPlusNode* rightChild, vector<BPlusNode*>& path);
    void persistNode(BPlusNode* node);
    BPlusNode* findLeaf(Key key, vector<BPlusNode*>& path);
    void splitLeaf(BPlusNode* leaf, vector<BPlusNode*>& path);
    void splitInternal(BPlusNode* node, vector<BPlusNode*>& path);
    BPlusNode* loadNode(uint32_t nodeID);
};