#pragma once

#include "Rope.h"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

using namespace std;

enum class NodeType {
    DIRECTORY,
    FILE
};

struct Permissions {
    int owner;
    int group;
    int others;

    Permissions();
    string toRWX(int perm) const;
    string toString() const;
};

class FSNode;

class HashFunction {
    public:
        static uint32_t hash(const string& str);
};

struct AVLHashNode {
    uint32_t hash;
    string name;
    shared_ptr<FSNode> node;
    AVLHashNode* left;
    AVLHashNode* right;
    int height;

    AVLHashNode(uint32_t h, const string& n, shared_ptr<FSNode> nd);
};

class HTreeIndex {
    private:
        AVLHashNode* root;
        int nodeCount;

        int getHeight(AVLHashNode* n) const;
        int getBalance(AVLHashNode* n) const;
        void updateHeight(AVLHashNode* n);
        AVLHashNode* rightRotate(AVLHashNode* y);
        AVLHashNode* leftRotate(AVLHashNode* x);
        AVLHashNode* balance(AVLHashNode* node);
        AVLHashNode* insertNode(AVLHashNode* node, uint32_t hash, 
                const string& name, shared_ptr<FSNode> fsnode);
        shared_ptr<FSNode> findNode(AVLHashNode* node, uint32_t hash, 
                const string& name) const;
        AVLHashNode* findMin(AVLHashNode* node);
        pair<AVLHashNode*, bool> removeNode(AVLHashNode* node, uint32_t hash, const string& name);
        void collectNodes(AVLHashNode* node, vector<shared_ptr<FSNode>>& result) const;
        int countNodes(AVLHashNode* node) const;
        void deleteTree(AVLHashNode* node);

    public:
        HTreeIndex();
        ~HTreeIndex();

        void insert(const string& name, shared_ptr<FSNode> node);
        shared_ptr<FSNode> find(const string& name) const;
        bool remove(const string& name);
        vector<shared_ptr<FSNode>> getAllNodes() const;
        size_t size() const;
        bool empty() const;
        void printStats() const;
};

class FSNode {
 public:
     string name;
     NodeType type;
     Permissions permissions;
     Rope content;
     HTreeIndex htree;
     FSNode* parent;
     
     FSNode(const string& name, NodeType type, FSNode* parent = nullptr);
     bool isDirectory() const;
     bool isFile() const;
     shared_ptr<FSNode> findChild(const string& childName);
     void addChild(shared_ptr<FSNode> child, bool silent = false);
     bool removeChild(const string& childName);
     vector<shared_ptr<FSNode>> getChildren() const;
};