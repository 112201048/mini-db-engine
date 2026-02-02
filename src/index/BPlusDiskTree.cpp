#include "BPlusDiskTree.h"
#include <stdexcept>
#include <cstring>
#include <vector>

using namespace std;

BPlusDiskTree::BPlusDiskTree(const string& filename) {
    file.open(filename, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        file.clear();
        file.open(filename, ios::out | ios::binary);
        file.close();
        file.open(filename, ios::in | ios::out | ios::binary);

        // Initialize metadata page
        IndexMeta meta{INVALID_NODE};
        file.write(reinterpret_cast<char*>(&meta), sizeof(meta));

        // Pad rest of first page
        vector<char> pad(INDEX_PAGE_SIZE - sizeof(meta), 0);
        file.write(pad.data(), pad.size());
        file.flush();
    }
}

uint32_t BPlusDiskTree::readRootID() {
    IndexMeta meta{};
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(&meta), sizeof(meta))) {
        return INVALID_NODE;
    }
    return meta.rootNodeID;
}


void BPlusDiskTree::writeRootID(uint32_t id) {
    IndexMeta meta{id};
    file.seekp(0);
    file.write(reinterpret_cast<char*>(&meta), sizeof(meta));
    file.flush();
}

uint32_t BPlusDiskTree::allocateNode() {
    file.seekp(0, ios::end);
    uint32_t nodeID = (file.tellp() / INDEX_PAGE_SIZE) - 1;
    vector<char> emptyPage(INDEX_PAGE_SIZE, 0);
    file.write(emptyPage.data(), INDEX_PAGE_SIZE);
    file.flush();
    return nodeID;
}

void BPlusDiskTree::writeNode(const NodePage& node) {
    char buffer[INDEX_PAGE_SIZE]{};
    size_t offset = 0;

    memcpy(buffer, &node.header, sizeof(NodeHeader));
    offset += sizeof(NodeHeader);

    memcpy(buffer + offset, node.keys.data(),
           node.keys.size() * sizeof(Key));
    offset += node.keys.size() * sizeof(Key);

    if (node.header.isLeaf) {
        memcpy(buffer + offset, node.rids.data(),
               node.rids.size() * sizeof(RID));
    } else {
        memcpy(buffer + offset, node.children.data(),
               node.children.size() * sizeof(uint32_t));
    }

    file.seekp((node.header.nodeID + 1) * INDEX_PAGE_SIZE);
    file.write(buffer, INDEX_PAGE_SIZE);
    file.flush();
}

NodePage BPlusDiskTree::readNode(uint32_t nodeID) {
    file.seekg((nodeID + 1) * INDEX_PAGE_SIZE);
    char buffer[INDEX_PAGE_SIZE]{};
    file.read(buffer, INDEX_PAGE_SIZE);

    NodePage node;
    size_t offset = 0;

    memcpy(&node.header, buffer, sizeof(NodeHeader));
    offset += sizeof(NodeHeader);

    node.keys.resize(node.header.numKeys);
    memcpy(node.keys.data(), buffer + offset,
           node.header.numKeys * sizeof(Key));
    offset += node.header.numKeys * sizeof(Key);

    if (node.header.isLeaf) {
        node.rids.resize(node.header.numKeys);
        memcpy(node.rids.data(), buffer + offset,
               node.header.numKeys * sizeof(RID));
    } else {
        node.children.resize(node.header.numKeys + 1);
        memcpy(node.children.data(), buffer + offset,
               (node.header.numKeys + 1) * sizeof(uint32_t));
    }

    return node;
}