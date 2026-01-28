#include "BPlusTree.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <vector>

using namespace std;

BPlusTree::BPlusTree(int order) : order(order){
    root = new BPlusNode(true);//start with a leaf
}

BPlusNode* BPlusTree::findLeaf(Key key, vector<BPlusNode*>& path) {
    BPlusNode* node = root;
    while (!node->isLeaf) {
        path.push_back(node);
        int i = 0;
        while (i < node->keys.size() && key >= node->keys[i]) {
            i++;
        }
        node = node->children[i];
    }
    return node;
}

void BPlusTree::insertInternal(BPlusNode* node, Key key, BPlusNode* rightChild, vector<BPlusNode*>& path) {
    auto it = upper_bound(node->keys.begin(), node->keys.end(), key);
    size_t index = distance(node->keys.begin(), it);
    node->keys.insert(it, key);
    node->children.insert(node->children.begin() + index + 1, rightChild);

    if (node->keys.size() > order) {
        splitInternal(node, path);
    }
}

void BPlusTree::splitInternal(BPlusNode* node, vector<BPlusNode*>& path) {
    int mid = order / 2;
    Key promotedKey = node->keys[mid];

    BPlusNode* newInternal = new BPlusNode(false);

    newInternal->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
    newInternal->children.assign(node->children.begin() + mid + 1, node->children.end());

    node->keys.resize(mid);
    node->children.resize(mid + 1);

    // If node is root
    if (node == root) {
        BPlusNode* newRoot = new BPlusNode(false);
        newRoot->keys.push_back(promotedKey);
        newRoot->children.push_back(node);
        newRoot->children.push_back(newInternal);
        root = newRoot;
        return;
    }

    // Insert into parent
    BPlusNode* parent = path.back();
    path.pop_back();
    insertInternal(parent, promotedKey, newInternal, path);
}

void BPlusTree::splitLeaf(BPlusNode* leaf, vector<BPlusNode*>& path) {
    int mid = (order + 1) / 2;
    BPlusNode *newLeaf = new BPlusNode(true);
    newLeaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
    newLeaf->rids.assign(leaf->rids.begin() + mid, leaf->rids.end());
    leaf->keys.resize(mid);
    leaf->rids.resize(mid);

    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    // Promote the first key of the new leaf to the parent
    Key promotedKey = newLeaf->keys[0];

    // If leaf is root
    if (leaf == root) {
        BPlusNode* newRoot = new BPlusNode(false);
        newRoot->keys.push_back(promotedKey);
        newRoot->children.push_back(leaf);
        newRoot->children.push_back(newLeaf);
        root = newRoot;
        return;
    }

    // Insert into parent
    BPlusNode* parent = path.back();
    path.pop_back();
    insertInternal(parent, promotedKey, newLeaf, path);
}

bool BPlusTree::search(Key key, RID& out) {
    vector<BPlusNode*> dummy;
    BPlusNode* node = findLeaf(key, dummy);
    auto it = lower_bound(node->keys.begin(), node->keys.end(), key);
    if (it != node->keys.end() && *it == key) {
        size_t index = distance(node->keys.begin(), it);
        out = node->rids[index];
        return true;
    }
    return false;
}

void BPlusTree::insert(Key key, const RID& rid) {
    vector<BPlusNode*> path;
    BPlusNode* leaf = findLeaf(key, path);
    auto it = lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    size_t index = distance(leaf->keys.begin(), it);
    leaf->keys.insert(it, key);
    leaf->rids.insert(leaf->rids.begin() + index, rid);
    if (leaf->keys.size() > order) {
        splitLeaf(leaf, path);
    }
}

vector<RID> BPlusTree::rangeScan(Key low, Key high){
    vector<RID> result;
    vector<BPlusNode*> dummy;
    BPlusNode* node = findLeaf(low, dummy);
    auto it = lower_bound(node->keys.begin(), node->keys.end(), low);
    size_t index = distance(node->keys.begin(), it);
    while (node) {
        while (index < node->keys.size()) {
            if (node->keys[index] > high) {
                return result;
            }
            result.push_back(node->rids[index]);
            index++;
        }
        node = node->next;
        index = 0;
    }
    return result;
}
