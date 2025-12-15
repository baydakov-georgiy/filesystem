#pragma once

#include <string>
#include <utility>

using namespace std;

struct RopeNode {
    int weight;
    int height;
    string text;
    RopeNode* left;
    RopeNode* right;

    RopeNode(const string& s);
    RopeNode();
    bool isLeaf() const;
};

class Rope {
    private:
        RopeNode* root;
        const int MAX_LEAF_SIZE = 8;

        void deleteNode(RopeNode* node);
        int getHeight(RopeNode* n) const;
        int getBalance(RopeNode* node) const;
        void updateHeight(RopeNode* node);
        int getLength(RopeNode* node) const;
        void updateWeight(RopeNode* node);
        RopeNode* copyNode(RopeNode* node);
        RopeNode* rightRotate(RopeNode* P);
        RopeNode* leftRotate(RopeNode* P);
        RopeNode* balance(RopeNode* node);
        RopeNode* buildFromString(const string& s, int start, int end);
        string getString(RopeNode* node) const;
        char charAt(RopeNode* node, int index) const;
        RopeNode* concat(RopeNode* left, RopeNode* right);
        pair<RopeNode*, RopeNode*> splitNode(RopeNode* node, int index);

    public:
        Rope();
        Rope(const string& s);
        Rope(RopeNode* node);
        Rope(const Rope& other);
        ~Rope();
        Rope& operator=(const Rope& other);

        void insert(int pos, const string& str);
        int find(const string& substr) const;
        bool deleteSubstring(const string& substr);
        void append(const string& str);
        string toString() const;
        int length() const;
        bool empty() const;
};