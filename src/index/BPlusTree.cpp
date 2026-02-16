#include "BPlusTree.h"
#include "BPlusNode.h"
#include "BPlusDiskTree.h"

#include <algorithm>
#include <stdexcept>
#include <iostream>

using namespace std;

BPlusTree::BPlusTree(int order, string filename)
    : order(order) {
    
    file = new BPlusDiskTree(filename);
    uint32_t rootID = file->readRootID();

    if (rootID == INVALID_NODE) {
        // First time creation
        root = new BPlusNode(true);
        root->nodeID = file->allocateNode();
        persistNode(root);
        file->writeRootID(root->nodeID);
    } else {
        // Reload existing tree
        root = loadNode(rootID);
    }
}

int BPlusTree::minKeys() const {
    return (order + 1) / 2 - 1;
}

BPlusNode* getLeftSibling(BPlusNode* node, BPlusNode* parent, int& index) {
    for (int i = 0; i < parent->children.size(); i++) {
        if (parent->children[i] == node) {
            index = i;
            if (i > 0)
                return parent->children[i - 1];
        }
    }
    return nullptr;
}

BPlusNode* getRightSibling(BPlusNode* node, BPlusNode* parent, int& index) {
    for (int i = 0; i < parent->children.size(); i++) {
        if (parent->children[i] == node) {
            index = i;
            if (i < parent->children.size() - 1)
                return parent->children[i + 1];
        }
    }
    return nullptr;
}


BPlusNode* BPlusTree::loadNode(uint32_t nodeID) {
    NodePage page = file->readNode(nodeID);

    BPlusNode* node = new BPlusNode(page.header.isLeaf);
    node->nodeID = page.header.nodeID;
    node->keys = page.keys;

    if (node->isLeaf) {
        node->rids = page.rids;
        if (page.header.nextLeaf != INVALID_NODE) {
            node->next = loadNode(page.header.nextLeaf);
        }
    } else {
        for (auto childID : page.children) {
            node->children.push_back(loadNode(childID));
        }
    }

    return node;
}

void BPlusTree::persistNode(BPlusNode* n) {
    NodePage page{};
    page.header.nodeID = n->nodeID;
    page.header.isLeaf = n->isLeaf;
    page.header.numKeys = n->keys.size();
    page.header.nextLeaf =
        n->isLeaf && n->next ? n->next->nodeID : INVALID_NODE;

    page.keys = n->keys;

    if (n->isLeaf)
        page.rids = n->rids;
    else {
        for (auto c : n->children)
            page.children.push_back(c->nodeID);
    }

    file->writeNode(page);
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
    persistNode(node);
}

void BPlusTree::splitInternal(BPlusNode* node, vector<BPlusNode*>& path) {
    int mid = order / 2;
    Key promotedKey = node->keys[mid];

    BPlusNode* newInternal = new BPlusNode(false);
    newInternal->nodeID = file->allocateNode();

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
        newRoot->nodeID = file->allocateNode();
        persistNode(newRoot);
        persistNode(node);
        persistNode(newInternal);
        root = newRoot;
        file->writeRootID(root->nodeID);
        return;
    }

    // Insert into parent
    BPlusNode* parent = path.back();
    path.pop_back();
    insertInternal(parent, promotedKey, newInternal, path);
    persistNode(node);
    persistNode(newInternal);
    persistNode(parent);
}

void BPlusTree::splitLeaf(BPlusNode* leaf, vector<BPlusNode*>& path) {
    int mid = (order + 1) / 2;
    BPlusNode *newLeaf = new BPlusNode(true);
    newLeaf->nodeID = file->allocateNode();
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
        newRoot->nodeID = file->allocateNode();
        root = newRoot;
        persistNode(leaf);
        persistNode(newLeaf);
        persistNode(newRoot);
        file->writeRootID(root->nodeID);
        return;
    }

    // Insert into parent
    BPlusNode* parent = path.back();
    path.pop_back();
    insertInternal(parent, promotedKey, newLeaf, path);
    persistNode(leaf);
    persistNode(newLeaf);
    persistNode(parent); // if exists
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
    persistNode(leaf);
}

void BPlusTree::rebalanceInternal(
    BPlusNode* node,
    vector<BPlusNode*>& path)
{
    if (node == root) {
        if (node->keys.empty()) {
            root = node->children[0];
            file->writeRootID(root->nodeID);
        }
        return;
    }

    BPlusNode* parent = path.back();
    path.pop_back();

    int index = 0;
    BPlusNode* left = getLeftSibling(node, parent, index);
    BPlusNode* right = getRightSibling(node, parent, index);

    // CASE 1 — Borrow from left
    if (left && left->keys.size() > minKeys()) {

        // Pull separator from parent
        node->keys.insert(node->keys.begin(),
            parent->keys[index - 1]);

        // Move last child of left
        node->children.insert(node->children.begin(),
            left->children.back());

        // Move key up from left to parent
        parent->keys[index - 1] = left->keys.back();

        left->keys.pop_back();
        left->children.pop_back();

        persistNode(left);
        persistNode(node);
        persistNode(parent);
        return;
    }

    // CASE 2 — Borrow from right
    if (right && right->keys.size() > minKeys()) {

        node->keys.push_back(parent->keys[index]);

        node->children.push_back(right->children.front());

        parent->keys[index] = right->keys.front();

        right->keys.erase(right->keys.begin());
        right->children.erase(right->children.begin());

        persistNode(right);
        persistNode(node);
        persistNode(parent);
        return;
    }

    // CASE 3 — Merge

    if (left) {

        // Pull separator down
        left->keys.push_back(parent->keys[index - 1]);

        // Merge keys
        left->keys.insert(left->keys.end(),
            node->keys.begin(), node->keys.end());

        // Merge children
        left->children.insert(left->children.end(),
            node->children.begin(), node->children.end());

        parent->keys.erase(parent->keys.begin() + index - 1);
        parent->children.erase(parent->children.begin() + index);

        persistNode(left);
        persistNode(parent);

        if (parent->keys.size() < minKeys())
            rebalanceInternal(parent, path);

    } else if (right) {

        node->keys.push_back(parent->keys[index]);

        node->keys.insert(node->keys.end(),
            right->keys.begin(), right->keys.end());

        node->children.insert(node->children.end(),
            right->children.begin(), right->children.end());

        parent->keys.erase(parent->keys.begin() + index);
        parent->children.erase(parent->children.begin() + index + 1);

        persistNode(node);
        persistNode(parent);

        if (parent->keys.size() < minKeys())
            rebalanceInternal(parent, path);
    }
}

void BPlusTree::rebalanceLeaf(BPlusNode* leaf,
                              vector<BPlusNode*>& path) {

    BPlusNode* parent = path.back();
    path.pop_back();

    int index = 0;
    BPlusNode* left = getLeftSibling(leaf, parent, index);
    BPlusNode* right = getRightSibling(leaf, parent, index);

    // CASE 1 — Borrow from left
    if (left && left->keys.size() > minKeys()) {
        cout << "Borrowing from left\n";
        leaf->keys.insert(leaf->keys.begin(),
                          left->keys.back());
        leaf->rids.insert(leaf->rids.begin(),
                          left->rids.back());

        left->keys.pop_back();
        left->rids.pop_back();

        parent->keys[index - 1] = leaf->keys.front();

        persistNode(left);
        persistNode(leaf);
        persistNode(parent);
        return;
    }

    // CASE 2 — Borrow from right
    if (right && right->keys.size() > minKeys()) {
        cout << "Borrowing from right\n";
        leaf->keys.push_back(right->keys.front());
        leaf->rids.push_back(right->rids.front());

        right->keys.erase(right->keys.begin());
        right->rids.erase(right->rids.begin());

        parent->keys[index] = right->keys.front();

        persistNode(right);
        persistNode(leaf);
        persistNode(parent);
        return;
    }

    // CASE 3 — Merge

    if (left) {
        cout << "Merging leaves\n";
        // merge into left
        left->keys.insert(left->keys.end(),
                          leaf->keys.begin(), leaf->keys.end());
        left->rids.insert(left->rids.end(),
                          leaf->rids.begin(), leaf->rids.end());

        left->next = leaf->next;

        parent->keys.erase(parent->keys.begin() + index - 1);
        parent->children.erase(parent->children.begin() + index);

        persistNode(left);
        persistNode(parent);

        if (parent == root && parent->keys.empty()) {
            root = left;
            file->writeRootID(root->nodeID);
        }

    } else if (right) {
        cout << "Merging leaves\n";
        // merge right into leaf
        leaf->keys.insert(leaf->keys.end(),
                          right->keys.begin(), right->keys.end());
        leaf->rids.insert(leaf->rids.end(),
                          right->rids.begin(), right->rids.end());

        leaf->next = right->next;

        parent->keys.erase(parent->keys.begin() + index);
        parent->children.erase(parent->children.begin() + index + 1);

        persistNode(leaf);
        persistNode(parent);

        if (parent == root && parent->keys.empty()) {
            root = leaf;
            file->writeRootID(root->nodeID);
        }
    }

    // Parent may now underflow
    if (parent != root && parent->keys.size() < minKeys()) {
        rebalanceInternal(parent, path);
    }
}


bool BPlusTree::remove(Key key) {
    vector<BPlusNode*> path;
    BPlusNode* leaf = findLeaf(key, path);

    auto it = lower_bound(leaf->keys.begin(), leaf->keys.end(), key);

    if (it == leaf->keys.end() || *it != key)
        return false;

    size_t index = distance(leaf->keys.begin(), it);

    leaf->keys.erase(it);
    leaf->rids.erase(leaf->rids.begin() + index);

    if (leaf == root){
        persistNode(leaf);
        return true;
    }

    if (leaf->keys.size() < minKeys()) {
        rebalanceLeaf(leaf, path);
    }

    persistNode(leaf);
    return true;
}

vector<RID> BPlusTree::rangeScan(Key low, Key high){
    vector<RID> result;
    vector<BPlusNode*> dummy;
    BPlusNode* node = findLeaf(low, dummy);
    if (!node) return result;
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
