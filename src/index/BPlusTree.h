#include "BPlusNode.h"

class BPlusTree {
public:
    BPlusTree(int order);

    void insert(Key key, const RID& rid);
    bool search(Key key, RID& rid);
private:
    BPlusNode* root;
    int order;

    void insertInternal(BPlusNode* node, Key key, const RID& rid, Key& promotedKey, BPlusNode*& newChild);
    BPlusNode* findLeaf(Key key);
};