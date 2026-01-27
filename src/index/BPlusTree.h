#include "BPlusNode.h"

class BPlusTree {
public:
    BPlusTree(int order);

    void insert(Key key, const RID& rid);
    bool search(Key key, RID& rid);
private:
    BPlusNode* root;
    int order;

    void insertInternal(BPlusNode* node, Key key, BPlusNode* rightChild, vector<BPlusNode*>& path);
    BPlusNode* findLeaf(Key key, vector<BPlusNode*>& path);
    void splitLeaf(BPlusNode* leaf, vector<BPlusNode*>& path);
    void splitInternal(BPlusNode* node, vector<BPlusNode*>& path);
};