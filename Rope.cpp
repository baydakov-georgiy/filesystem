#include "Rope.h"
#include <iostream>

using namespace std;

void printMessage(const string& who, const string& msg) {
    if (!who.empty())
        cout << "[" << who << "] "; 
    cout << msg << endl;
}

void print_error(const string& who, const string& msg) {
    if (!who.empty())
        cout << "[" << who << "] "; 
    cout << msg << endl;
}

RopeNode::RopeNode(const string& s)
    : weight(s.size()), height(1), text(s),
    left(nullptr), right(nullptr) {}

RopeNode::RopeNode()
    : weight(0), height(1), text(""),
    left(nullptr), right(nullptr) {}

bool RopeNode::isLeaf() const {
    return left == nullptr && right == nullptr;
}

int Rope::getHeight(RopeNode* node) const {
    return node ? node->height : 0;
}

int Rope::getBalance(RopeNode* node) const {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

void Rope::updateHeight(RopeNode* node) {
    if (!node) return; 
    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
}

int Rope::getLength(RopeNode* node) const {
    if (!node) return 0;
    if (node->isLeaf()) return node->text.size();
    return node->weight + getLength(node->right);
}

void Rope::updateWeight(RopeNode* node) {
    if (node && !node->isLeaf()) {
        node->weight = getLength(node->left);
    }
}

RopeNode* Rope::copyNode(RopeNode* node) {
    if (!node) return nullptr;

    RopeNode* newNode = new RopeNode();
    newNode->weight = node->weight;
    newNode->height = node->height;
    newNode->text = node->text;
    newNode->left = copyNode(node->left);
    newNode->right = copyNode(node->right);

    return newNode;
}

void Rope::deleteNode(RopeNode* node) {
    if (!node) return;
    deleteNode(node->left);
    deleteNode(node->right);
    delete node;
}

RopeNode* Rope::rightRotate(RopeNode* P) {
    if (!P || !P->left) return P; 

    RopeNode* Q = P->left;

    RopeNode* newP = copyNode(P);
    RopeNode* newQ = copyNode(Q);

    newP->left = newQ->right;
    newQ->right = newP;

    updateHeight(newP);
    updateWeight(newP);
    updateHeight(newQ);
    updateWeight(newQ);

    return newQ;
}

RopeNode* Rope::leftRotate(RopeNode* P) {
    if (!P || !P->right) return P;

    RopeNode* Q = P->right;

    RopeNode* newP = copyNode(P);
    RopeNode* newQ = copyNode(Q);

    newP->right = newQ->left;
    newQ->left = newP;

    updateHeight(newP);
    updateWeight(newP);
    updateHeight(newQ);
    updateWeight(newQ);

    return newQ;
}

RopeNode* Rope::balance(RopeNode* node) {
    if (!node) return node;

    updateHeight(node);
    updateWeight(node);

    int bf = getBalance(node);

    if (bf > 1) {
        if (getBalance(node->left) < 0) {
            node->left = leftRotate(node->left);
        }
        return rightRotate(node);
    }

    if (bf < -1) {
        if (getBalance(node->right) > 0) {
            node->right = rightRotate(node->right);
        }
        return leftRotate(node);
    }

    return node;
}

RopeNode* Rope::buildFromString(const string& s, int start, int end) {
    if (start >= end) return nullptr;

    if (end - start <= MAX_LEAF_SIZE) {
        return new RopeNode(s.substr(start, end - start));
    }

    int mid = (start + end) / 2;
    RopeNode* node = new RopeNode();
    node->left = buildFromString(s, start, mid);
    node->right = buildFromString(s, mid, end);
    node->weight = getLength(node->left);
    updateHeight(node);

    return balance(node);
}

string Rope::getString(RopeNode* node) const {
    if (!node) return "";
    if (node->isLeaf()) return node->text;
    return getString(node->left) + getString(node->right);
}

char Rope::charAt(RopeNode* node, int index) const {
    if (!node) return '\0';

    if (node->isLeaf()) {
        return (size_t)index < node->text.size() ? node->text[index] : '\0';
    }

    if (index < node->weight) {
        return charAt(node->left, index);
    }

    return charAt(node->right, index - node->weight);
}

RopeNode* Rope::concat(RopeNode* left, RopeNode* right) {
    if (!left) return right;
    if (!right) return left;

    RopeNode* node = new RopeNode();
    node->left = left;
    node->right = right;
    node->weight = getLength(left);
    updateHeight(node);

    return balance(node);
}

pair<RopeNode*, RopeNode*> Rope::splitNode(RopeNode* node, int index) {
    if (!node) return {nullptr, nullptr};

    if (node->isLeaf()) {
        if (index <= 0) return { nullptr, copyNode(node) };
        if ((size_t)index >= node->text.size()) return { copyNode(node), nullptr };

        RopeNode* left = new RopeNode(node->text.substr(0, index));
        RopeNode* right = new RopeNode(node->text.substr(index));

        return {left, right};
    }

    if (index <= node->weight) {
        auto [l1, l2] = splitNode(node->left, index);
        RopeNode* right = concat(l2, copyNode(node->right));
        return {l1, right};
    } else {
        auto [r1, r2] = splitNode(node->right, index - node->weight);
        RopeNode* left = concat(copyNode(node->left), r1);
        return { left, r2 };
    }
}

Rope::Rope() : root(nullptr) {}

Rope::Rope(const string& s) {
    root = buildFromString(s, 0, s.size());
}

Rope::Rope(RopeNode* node) : root(node) {}

Rope::Rope(const Rope& other) : root(copyNode(other.root)) {}

Rope::~Rope() {
    deleteNode(root);
}

Rope& Rope::operator=(const Rope& other) {
    if (this != &other) {
        deleteNode(root);
        root = copyNode(other.root);
    }
    return *this;
}

void Rope::insert(int pos, const string& str) {
    if (str.empty()) return;

    int len = length();
    if (pos < 0 || pos > len) {
        print_error("Rope", "Неверная позиция для вставки");
        return;
    }

    auto [l, r] = splitNode(root, pos);
    RopeNode* mid = buildFromString(str, 0, str.size());

    deleteNode(root);
    
    root = concat(concat(l, mid), r);
    printMessage("Rope", "Вставлено <" + str + "> на позицию" + to_string(pos));
}

int Rope::find(const string& substr) const {
    string current = toString();
    size_t pos = current.find(substr);

    if (pos == string::npos) {
        return -1;
    }

    return static_cast<int>(pos);
}

bool Rope::deleteSubstring(const string& substr) {
    string current = toString();
    size_t pos = current.find(substr);

    if (pos == string::npos) {
        printMessage("Rope", "Подстрока <" + substr + "> не найдена");
        return false;
    }

    RopeNode* oldRoot = root;
    auto [l, tmp] = splitNode(oldRoot, pos);
    auto [mid, r] = splitNode(tmp, substr.length());

    deleteNode(oldRoot);
    deleteNode(tmp);
    deleteNode(mid);
    
    root = concat(l, r);

    printMessage("Rope", "Удалена подстрока <" + substr + "> с позиции " + to_string(pos));
    return true;
}

void Rope::append(const string& str) {
    if (str.empty()) return;
    RopeNode* mid = buildFromString(str, 0, str.size());
    root = concat(root, mid);
}

string Rope::toString() const {
    return getString(root);
}

int Rope::length() const {
    return getLength(root);
}

bool Rope::empty() const {
    return root == nullptr || length() == 0;
}
