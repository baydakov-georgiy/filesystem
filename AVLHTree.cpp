#include "AVLHTree.h"
#include <iostream>
#include <cmath>

using namespace std;

Permissions::Permissions() : owner(7), group(5), others(5) {}

string Permissions::toRWX(int perm) const {
    string result;
    result += (perm & 4) ? 'r' : '-';
    result += (perm & 2) ? 'w' : '-';
    result += (perm & 1) ? 'x' : '-';
    return result;
}

string Permissions::toString() const {
    return toRWX(owner) + toRWX(group) + toRWX(others);
}

FSNode::FSNode(const string& name, NodeType type, FSNode* parent)
    : name(name), type(type), parent(parent) {}

bool FSNode::isDirectory() const { return type == NodeType::DIRECTORY; }
bool FSNode::isFile() const { return type == NodeType::FILE; }

shared_ptr<FSNode> FSNode::findChild(const string& childName) {
    if (!isDirectory()) return nullptr;
    return htree.find(childName);
}

void FSNode::addChild(shared_ptr<FSNode> child, bool silent) {
    if (!isDirectory()) return;
    htree.insert(child->name, child);
    if (!silent) {
        cout << "  [AVL H-Tree] Добавлен '" << child->name 
             << "' (hash: " << HashFunction::hash(child->name) << ")" << endl;
    }
}

bool FSNode::removeChild(const string& childName) {
    if (!isDirectory()) return false;
    return htree.remove(childName);
}

vector<shared_ptr<FSNode>> FSNode::getChildren() const {
    if (!isDirectory()) return {};
    return htree.getAllNodes();
}

uint32_t HashFunction::hash(const string& str) {
    uint32_t hash = 0;
    for (char c : str) {
        hash = hash * 31 + static_cast<uint32_t>(c);
    }
    return hash;
}

AVLHashNode::AVLHashNode(uint32_t h, const string& n, shared_ptr<FSNode> nd)
    : hash(h), name(n), node(nd), left(nullptr), right(nullptr), height(1) {}

HTreeIndex::HTreeIndex() : root(nullptr) {}

HTreeIndex::~HTreeIndex() {
    deleteTree(root);
}

int HTreeIndex::getHeight(AVLHashNode* n) const {
    return n ? n->height : 0;
}

int HTreeIndex::getBalance(AVLHashNode* n) const {
    return n ? getHeight(n->left) - getHeight(n->right) : 0;
}

void HTreeIndex::updateHeight(AVLHashNode* n) {
    if (n) {
        n->height = 1 + max(getHeight(n->left), getHeight(n->right));
    }
}

AVLHashNode* HTreeIndex::rightRotate(AVLHashNode* y) {
    AVLHashNode* x = y->left;
    AVLHashNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    updateHeight(y);
    updateHeight(x);

    return x;
}

AVLHashNode* HTreeIndex::leftRotate(AVLHashNode* x) {
    AVLHashNode* y = x->right;
    AVLHashNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    updateHeight(x);
    updateHeight(y);

    return y;
}

AVLHashNode* HTreeIndex::balance(AVLHashNode* node) {
    if (!node) return node;

    updateHeight(node);
    int bal = getBalance(node);

    if (bal > 1 && getBalance(node->left) >= 0) {
        return rightRotate(node);
    }

    if (bal > 1 && getBalance(node->left) < 0) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    if (bal < -1 && getBalance(node->right) <= 0) {
        return leftRotate(node);
    }

    if (bal < -1 && getBalance(node->right) > 0) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

AVLHashNode* HTreeIndex::insertNode(AVLHashNode* node, uint32_t hash, 
        const string& name, shared_ptr<FSNode> fsnode) {
    if (!node) {
        return new AVLHashNode(hash, name, fsnode);
    }

    if (hash < node->hash) {
        node->left = insertNode(node->left, hash, name, fsnode);
    } else if (hash > node->hash) {
        node->right = insertNode(node->right, hash, name, fsnode);
    } else {
        if (name < node->name) {
            node->left = insertNode(node->left, hash, name, fsnode);
        } else {
            node->right = insertNode(node->right, hash, name, fsnode);
        }
    }

    return balance(node);
}

shared_ptr<FSNode> HTreeIndex::findNode(AVLHashNode* node, uint32_t hash, 
        const string& name) const {
    if (!node) return nullptr;

    if (hash < node->hash) {
        return findNode(node->left, hash, name);
    } else if (hash > node->hash) {
        return findNode(node->right, hash, name);
    } else {
        if (node->name == name) {
            return node->node;
        }
        auto left_result = findNode(node->left, hash, name);
        if (left_result) return left_result;
        return findNode(node->right, hash, name);
    }
}

AVLHashNode* HTreeIndex::findMin(AVLHashNode* node) {
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

AVLHashNode* HTreeIndex::removeNode(AVLHashNode* node, uint32_t hash, const string& name) {
    if (!node) return nullptr;

    if (hash < node->hash) {
        node->left = removeNode(node->left, hash, name);
    } else if (hash > node->hash) {
        node->right = removeNode(node->right, hash, name);
    } else if (node->name == name) {
        if (!node->left || !node->right) {
            AVLHashNode* temp = node->left ? node->left : node->right;
            delete node;
            return temp;
        } else {
            AVLHashNode* temp = findMin(node->right);
            node->hash = temp->hash;
            node->name = temp->name;
            node->node = temp->node;
            node->right = removeNode(node->right, temp->hash, temp->name);
        }
    } else {
        AVLHashNode* newLeft = removeNode(node->left, hash, name);
        if (newLeft != node->left) {
            node->left = newLeft;
            return balance(node);
        }

        node->right = removeNode(node->right, hash, name);
    }

    return balance(node);
}

void HTreeIndex::collectNodes(AVLHashNode* node, vector<shared_ptr<FSNode>>& result) const {
    if (!node) return;
    collectNodes(node->left, result);
    result.push_back(node->node);
    collectNodes(node->right, result);
}

int HTreeIndex::countNodes(AVLHashNode* node) const {
    if (!node) return 0;
    return 1 + countNodes(node->left) + countNodes(node->right);
}

void HTreeIndex::deleteTree(AVLHashNode* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

void HTreeIndex::insert(const string& name, shared_ptr<FSNode> node) {
    uint32_t hashValue = HashFunction::hash(name);
    root = insertNode(root, hashValue, name, node);
}

shared_ptr<FSNode> HTreeIndex::find(const string& name) const {
    uint32_t hashValue = HashFunction::hash(name);
    return findNode(root, hashValue, name);
}

bool HTreeIndex::remove(const string& name) {
    uint32_t hashValue = HashFunction::hash(name);
    int oldSize = countNodes(root);
    root = removeNode(root, hashValue, name);
    int newSize = countNodes(root);
    return newSize < oldSize;
}

vector<shared_ptr<FSNode>> HTreeIndex::getAllNodes() const {
    vector<shared_ptr<FSNode>> result;
    collectNodes(root, result);
    return result;
}

size_t HTreeIndex::size() const {
    return countNodes(root);
}

bool HTreeIndex::empty() const {
    return root == nullptr;
}

void HTreeIndex::printStats() const {
    if (!root) return;

    int nodes = countNodes(root);
    int height = getHeight(root);
    int minHeight = nodes > 0 ? (int)ceil(log2(nodes + 1)) : 0;

    cout << "  [AVL H-Tree Stats] Узлов: " << nodes 
        << ", Высота: " << height
        << " (мин: " << minHeight << ")"
        << ", Баланс: " << (height <= 1.44 * log2(nodes + 1) ? "OK" : "Warning")
        << endl;
}
