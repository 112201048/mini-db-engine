#include "BPlusTree.h"
#include <algorithm>
#include <stdexcept>

using namespace std;

BPlusTree::BPlusTree(int order) : order(order){
    root = new BPlusNode(true);//start with a leaf
}

BPlusNode* BPlusTree::findLeaf(Key key){
    BPlusNode* node = root;
    while (!node->isLeaf) {
        int i = 0;
        while (i < node->keys.size() && key >= node->keys[i]) {
            i++;
        }
        node = node->children[i];
    }
    return node;
}


bool BPlusTree::search(Key key, RID& out) {
    BPlusNode* node = findLeaf(key);
    auto it = lower_bound(node->keys.begin(), node->keys.end(), key);
    if (it != node->keys.end() && *it == key) {
        size_t index = distance(node->keys.begin(), it);
        out = node->rids[index];
        return true;
    }
    return false;
}

//simple insert without splitting
void BPlusTree::insert(Key key, const RID& rid) {
    BPlusNode* leaf = findLeaf(key);
    auto it = lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    size_t index = distance(leaf->keys.begin(), it);
    leaf->keys.insert(it, key);
    leaf->rids.insert(leaf->rids.begin() + index, rid);
}