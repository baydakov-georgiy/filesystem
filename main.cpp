#include <functional>
#include <iostream>
#include <string>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>

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

struct RopeNode {
    int weight;
    int height;
    string text;
    RopeNode* left;
    RopeNode* right;

    RopeNode(const string& s)
        : weight(s.size()), height(1), text(s),
        left(nullptr), right(nullptr) {}

    RopeNode()
        : weight(0), height(1), text(""),
        left(nullptr), right(nullptr) {}

    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

class Rope {
    private:
        RopeNode* root;
        const int MAX_LEAF_SIZE = 8;

        int getHeight(RopeNode* node) const {
            return node ? node->height : 0;
        }

        int getBalance(RopeNode* node) const {
            return node ? getHeight(node->left) - getHeight(node->right) : 0;
        }

        void updateHeight(RopeNode* node) {
            if (!node) return; 
            node->height = 1 + max(getHeight(node->left), getHeight(node->right));
        }

        int getLength(RopeNode* node) const {
            if (!node) return 0;
            if (node->isLeaf()) return node->text.size();
            return node->weight + getLength(node->right);
        }

        void updateWeight(RopeNode* node) {
            if (node && !node->isLeaf()) {
                node->weight = getLength(node->left);
            }
        }

        RopeNode* copyNode(RopeNode* node) {
            if (!node) return nullptr;

            RopeNode* newNode = new RopeNode();
            newNode->weight = node->weight;
            newNode->height = node->height;
            newNode->text = node->text;
            newNode->left = copyNode(node->left);
            newNode->right = copyNode(node->right);

            return newNode;
        }

        void deleteNode(RopeNode* node) {
            if (!node) return;
            deleteNode(node->left);
            deleteNode(node->right);
            delete node;
        }

        RopeNode* rightRotate(RopeNode* P) {
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

        RopeNode* leftRotate(RopeNode* P) {
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

        RopeNode* balance(RopeNode* node) {
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

        RopeNode* buildFromString(const string& s, int start, int end) {
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

        string getString(RopeNode* node) const {
            if (!node) return "";
            if (node->isLeaf()) return node->text;
            return getString(node->left) + getString(node->right);
        }

        char charAt(RopeNode* node, int index) const {
            if (!node) return '\0';

            if (node->isLeaf()) {
                return (size_t)index < node->text.size() ? node->text[index] : '\0';
            }

            if (index < node->weight) {
                return charAt(node->left, index);
            }

            return charAt(node->right, index - node->weight);
        }

        RopeNode* concat(RopeNode* left, RopeNode* right) {
            if (!left) return right;
            if (!right) return left;

            RopeNode* node = new RopeNode();
            node->left = left;
            node->right = right;
            node->weight = getLength(left);
            updateHeight(node);

            return balance(node);
        }

        pair<RopeNode*, RopeNode*> splitNode(RopeNode* node, int index) {
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

    public:
        Rope() : root(nullptr) {}

        Rope (const string& s) {
            root = buildFromString(s, 0, s.size());
        }

        Rope(RopeNode* node) : root(node) {}

        Rope(const Rope& other) : root(copyNode(other.root)) {}

        ~Rope() {
            deleteNode(root);
        }

        Rope& operator=(const Rope& other) {
            if (this != &other) {
                deleteNode(root);
                root = copyNode(other.root);
            }
            return *this;
        }

        void insert(int pos, const string& str) {
            if (str.empty()) return;

            int len = length();
            if (pos < 0 || pos > len) {
                print_error("Rope", "Неверная позиция для вставки");
                return;
            }

            auto [l, r] = splitNode(root, pos);
            RopeNode* mid = buildFromString(str, 0, str.size());
            
            // Удаляем старое дерево, так как splitNode создает копии
            deleteNode(root);
            
            root = concat(concat(l, mid), r);
            printMessage("Rope", "Вставлено <" + str + "> на позицию" + to_string(pos));
        }

        int find(const string& substr) const {
            string current = toString();
            size_t pos = current.find(substr);

            if (pos == string::npos) {
                return -1;
            }

            return static_cast<int>(pos);
        }

        bool deleteSubstring(const string& substr) {
            string current = toString();
            size_t pos = current.find(substr);

            if (pos == string::npos) {
                printMessage("Rope", "Подстрока <" + substr + "> не найдена");
                return false;
            }

            RopeNode* oldRoot = root;
            auto [l, tmp] = splitNode(oldRoot, pos);
            auto [mid, r] = splitNode(tmp, substr.length());
            
            // Удаляем старое дерево и средний кусок
            deleteNode(oldRoot);
            deleteNode(tmp);
            deleteNode(mid);
            
            root = concat(l, r);

            printMessage("Rope", "Удалена подстрока <" + substr + "> с позиции " + to_string(pos));
            return true;
        }

        void append(const string& str) {
            if (str.empty()) return;
            RopeNode* oldRoot = root;
            RopeNode* mid = buildFromString(str, 0, str.size());
            root = concat(root, mid);
            // concat не удаляет старые узлы, только создает новый родительский
            // Старый root становится дочерним узлом, поэтому НЕ удаляем
        }

        string toString() const {
            return getString(root);
        }

        int length() const {
            return getLength(root);
        }

        bool empty() const {
            return root == nullptr || length() == 0;
        }
};

enum class NodeType {
    DIRECTORY,
    FILE
};

enum class Permission {
    NONE = 0,
    READ = 4,  
    WRITE = 2,  
    EXECUTE = 1 
};

struct Permissions {
    int owner;
    int group;
    int others;

    Permissions() : owner(7), group(5), others(5) {}

    string toRWX(int perm) const {
        string result;
        result += (perm & 4) ? 'r' : '-';
        result += (perm & 2) ? 'w' : '-';
        result += (perm & 1) ? 'x' : '-';
        return result;
    }

    string toString() const {
        return toRWX(owner) + toRWX(group) + toRWX(others);
    }
};

class FSNode;

class HashFunction {
    public:
        static uint32_t hash(const string& str) {
            uint32_t hash = 0;
            for (char c : str) {
                hash = hash * 31 + static_cast<uint32_t>(c);
            }
            return hash;
        }
};

struct AVLHashNode {
    uint32_t hash;
    string name;
    shared_ptr<FSNode> node;
    AVLHashNode* left;
    AVLHashNode* right;
    int height;

    AVLHashNode(uint32_t h, const string& n, shared_ptr<FSNode> nd)
        : hash(h), name(n), node(nd), left(nullptr), right(nullptr), height(1) {}
};

class HTreeIndex {
    private:
        AVLHashNode* root;

        int getHeight(AVLHashNode* n) const {
            return n ? n->height : 0;
        }

        int getBalance(AVLHashNode* n) const {
            return n ? getHeight(n->left) - getHeight(n->right) : 0;
        }

        void updateHeight(AVLHashNode* n) {
            if (n) {
                n->height = 1 + max(getHeight(n->left), getHeight(n->right));
            }
        }

        AVLHashNode* rightRotate(AVLHashNode* y) {
            AVLHashNode* x = y->left;
            AVLHashNode* T2 = x->right;

            x->right = y;
            y->left = T2;

            updateHeight(y);
            updateHeight(x);

            return x;
        }

        AVLHashNode* leftRotate(AVLHashNode* x) {
            AVLHashNode* y = x->right;
            AVLHashNode* T2 = y->left;

            y->left = x;
            x->right = T2;

            updateHeight(x);
            updateHeight(y);

            return y;
        }

        AVLHashNode* balance(AVLHashNode* node) {
            if (!node) return node;

            updateHeight(node);
            int bal = getBalance(node);

            // Left-Left
            if (bal > 1 && getBalance(node->left) >= 0) {
                return rightRotate(node);
            }

            // Left-Right
            if (bal > 1 && getBalance(node->left) < 0) {
                node->left = leftRotate(node->left);
                return rightRotate(node);
            }

            // Right-Right
            if (bal < -1 && getBalance(node->right) <= 0) {
                return leftRotate(node);
            }

            // Right-Left
            if (bal < -1 && getBalance(node->right) > 0) {
                node->right = rightRotate(node->right);
                return leftRotate(node);
            }

            return node;
        }

        AVLHashNode* insertNode(AVLHashNode* node, uint32_t hash, 
                const string& name, shared_ptr<FSNode> fsnode) {
            if (!node) {
                return new AVLHashNode(hash, name, fsnode);
            }

            if (hash < node->hash) {
                node->left = insertNode(node->left, hash, name, fsnode);
            } else if (hash > node->hash) {
                node->right = insertNode(node->right, hash, name, fsnode);
            } else {
                // Коллизия - сравниваем имена
                if (name < node->name) {
                    node->left = insertNode(node->left, hash, name, fsnode);
                } else {
                    node->right = insertNode(node->right, hash, name, fsnode);
                }
            }

            return balance(node);
        }

        shared_ptr<FSNode> findNode(AVLHashNode* node, uint32_t hash, 
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

        AVLHashNode* findMin(AVLHashNode* node) {
            while (node && node->left) {
                node = node->left;
            }
            return node;
        }

        AVLHashNode* removeNode(AVLHashNode* node, uint32_t hash, const string& name) {
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
                // Коллизия: hash совпадает, но имя другое
                // Пытаемся удалить из левого поддерева
                AVLHashNode* newLeft = removeNode(node->left, hash, name);
                if (newLeft != node->left) {
                    // Удаление произошло в левом поддереве
                    node->left = newLeft;
                    return balance(node);
                }
                // Не нашли слева, ищем справа
                node->right = removeNode(node->right, hash, name);
            }

            return balance(node);
        }

        void collectNodes(AVLHashNode* node, vector<shared_ptr<FSNode>>& result) const {
            if (!node) return;
            collectNodes(node->left, result);
            result.push_back(node->node);
            collectNodes(node->right, result);
        }

        int countNodes(AVLHashNode* node) const {
            if (!node) return 0;
            return 1 + countNodes(node->left) + countNodes(node->right);
        }

        void deleteTree(AVLHashNode* node) {
            if (!node) return;
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }

public:
        HTreeIndex() : root(nullptr) {}

        ~HTreeIndex() {
            deleteTree(root);
        }

        void insert(const string& name, shared_ptr<FSNode> node) {
            uint32_t hashValue = HashFunction::hash(name);
            root = insertNode(root, hashValue, name, node);
        }

        shared_ptr<FSNode> find(const string& name) const;

        bool remove(const string& name);

        vector<shared_ptr<FSNode>> getAllNodes() const {
            vector<shared_ptr<FSNode>> result;
            collectNodes(root, result);
            return result;
        }

        size_t size() const {
            return countNodes(root);
        }

        bool empty() const {
            return root == nullptr;
        }

        void printStats() const {
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
};

class FSNode {
 public:
     string name;
     NodeType type;
     Permissions permissions;
     Rope content;
     HTreeIndex htree;
     FSNode* parent;
     
     FSNode(const string& name, NodeType type, FSNode* parent = nullptr)
         : name(name), type(type), parent(parent) {}
     
     bool isDirectory() const { return type == NodeType::DIRECTORY; }
     bool isFile() const { return type == NodeType::FILE; }
     
     shared_ptr<FSNode> findChild(const string& childName) {
         if (!isDirectory()) return nullptr;
         return htree.find(childName);
     }
     
     void addChild(shared_ptr<FSNode> child, bool silent = false) {
         if (!isDirectory()) return;
         htree.insert(child->name, child);
         if (!silent) {
             cout << "  [AVL H-Tree] Добавлен '" << child->name 
                  << "' (hash: " << HashFunction::hash(child->name) << ")" << endl;
         }
     }
     
     bool removeChild(const string& childName) {
         if (!isDirectory()) return false;
         return htree.remove(childName);
     }
     
     vector<shared_ptr<FSNode>> getChildren() const {
         if (!isDirectory()) return {};
         return htree.getAllNodes();
     }
 };
 
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

class FileSystem {
    shared_ptr<FSNode> root;
    shared_ptr<FSNode> currentDir;  // Текущая директория для cd
    bool debugMode;  // Режим отладки для вывода подробной информации

    vector<string> splitPath(const string& path) const {
        vector<string> components;
        stringstream ss(path);
        string component;

        while (getline(ss, component, '/' )) {
            if (!component.empty() && component != ".") {
                components.push_back(component);
            }
        
        }

        return components;
    }

    shared_ptr<FSNode> findNode(const string& path) {
        if (path == "/" || path.empty()) {
            return root;
        }

        vector<string> components = splitPath(path);
        shared_ptr<FSNode> current = root;

        for (const auto& comp : components) {
            if (!current->isDirectory()) {
                return nullptr;
            }

            current = current->findChild(comp);
            if (!current) {
                return nullptr;
            }
        }

        return current;
    }
    
    // Разрешить путь (абсолютный или относительный) относительно текущей директории
    shared_ptr<FSNode> resolvePath(const string& path) {
        if (path.empty()) {
            return nullptr;
        }
        
        if (path[0] == '/') {
            // Абсолютный путь
            return findNode(path);
        }
        
        // Относительный путь - может содержать несколько компонентов
        vector<string> components;
        stringstream ss(path);
        string component;
        while (getline(ss, component, '/')) {
            if (!component.empty()) {
                components.push_back(component);
            }
        }
        
        shared_ptr<FSNode> current = currentDir;
        for (const auto& comp : components) {
            if (!current->isDirectory()) {
                return nullptr;
            }
            
            if (comp == "..") {
                // Вверх на уровень
                if (!current->parent) {
                    // Уже в корне
                    continue;
                }
                
                FSNode* parentRaw = current->parent;
                
                if (parentRaw == root.get()) {
                    current = root;
                } else {
                    // Нужно найти shared_ptr для родителя
                    // Получаем текущий путь и поднимаемся на уровень вверх
                    string currentPath = getPathToNode(current);
                    size_t lastSlash = currentPath.find_last_of('/');
                    
                    if (lastSlash == 0) {
                        current = root;
                    } else {
                        string parentPath = currentPath.substr(0, lastSlash);
                        current = findNode(parentPath);
                        
                        if (!current) {
                            return nullptr;
                        }
                    }
                }
            } else if (comp == ".") {
                // Текущая директория - ничего не делаем
                continue;
            } else {
                // Обычный компонент
                current = current->findChild(comp);
                if (!current) {
                    return nullptr;
                }
            }
        }
        
        return current;
    }
    
    // Получить путь к узлу от корня
    string getPathToNode(shared_ptr<FSNode> node) {
        if (node == root) {
            return "/";
        }
        
        vector<string> components;
        FSNode* current = node.get();
        
        while (current && current != root.get()) {
            components.push_back(current->name);
            current = current->parent;
        }
        
        string path = "";
        for (auto it = components.rbegin(); it != components.rend(); ++it) {
            path += "/" + *it;
        }
        
        return path;
    }
    
    // Проверка прав доступа (упрощённая модель: owner=7, group=5, others=5 по умолчанию)
    bool checkReadPermission(shared_ptr<FSNode> node) {
        if (!node) return false;
        // Упрощённо: проверяем права owner (всегда считаем что мы owner)
        return (node->permissions.owner & 4) != 0;
    }
    
    bool checkWritePermission(shared_ptr<FSNode> node) {
        if (!node) return false;
        return (node->permissions.owner & 2) != 0;
    }
    
    bool checkExecutePermission(shared_ptr<FSNode> node) {
        if (!node) return false;
        return (node->permissions.owner & 1) != 0;
    }

    void searchRecursive(shared_ptr<FSNode> node, const string& name, 
                        const string& currentPath, vector<string>& results) {
        if (!node) return;
        
        // Формируем путь правильно, избегая двойных слешей
        string nodePath;
        if (currentPath == "/") {
            nodePath = "/" + node->name;
        } else {
            nodePath = currentPath + "/" + node->name;
        }
        
        if (node->name.find(name) != string::npos) {
            results.push_back(nodePath);
        }
        
        if (node->isDirectory()) {
            auto children = node->getChildren();
            for (auto& child : children) {
                searchRecursive(child, name, nodePath, results);
            }
        }
    }

    void visualizeTree(shared_ptr<FSNode> node, const string& prefix, bool isLast) {
        if (!node) return;
        
        cout << prefix;
        cout << (isLast ? "└── " : "├── ");
        
        if (node->isDirectory()) {
            cout << "\033[1;34m" << node->name << "/\033[0m";
        } else {
            cout << node->name;
        }
        
        cout << " [" << node->permissions.toString() << "]";
        
        cout << " {hash:" << HashFunction::hash(node->name) << "}";
        
        cout << endl;
        
        if (node->isDirectory()) {
            string newPrefix = prefix + (isLast ? "    " : "│   ");
            auto children = node->getChildren();
            for (size_t i = 0; i < children.size(); i++) {
                bool childIsLast = (i == children.size() - 1);
                visualizeTree(children[i], newPrefix, childIsLast);
            }
        }
    }

public:
    FileSystem() : debugMode(false) {
        root = make_shared<FSNode>("", NodeType::DIRECTORY, nullptr);
        currentDir = root;
        cout << "[ФС] Файловая система инициализирована" << endl;
    }
    
    // Переключить режим отладки
    void toggleDebug() {
        debugMode = !debugMode;
        cout << "Режим отладки: " << (debugMode ? "ВКЛ" : "ВЫКЛ") << endl;
    }
    
    bool isDebugMode() const {
        return debugMode;
    }

    // Получить текущую директорию
    string getCurrentPath() const {
        if (currentDir == root) return "/";
        
        string path;
        FSNode* node = currentDir.get();
        
        while (node && node->parent) {
            path = "/" + node->name + path;
            node = node->parent;
        }
        
        return path.empty() ? "/" : path;
    }

    // Изменить текущую директорию
    bool changeDirectory(const string& path) {
        if (path == "/") {
            currentDir = root;
            return true;
        }
        
        shared_ptr<FSNode> target;
        
        if (path[0] == '/') {
            // Абсолютный путь
            target = findNode(path);
        } else if (path == "..") {
            // Вверх на уровень
            if (!currentDir->parent) {
                // Уже в корне
                return true;
            }
            
            FSNode* parentRaw = currentDir->parent;
            
            if (parentRaw == root.get()) {
                currentDir = root;
                return true;
            }
            
            // Нужно найти shared_ptr для родителя
            // Идём от корня и ищем родителя
            string currentPath = getCurrentPath();
            size_t lastSlash = currentPath.find_last_of('/');
            
            if (lastSlash == 0) {
                // Родитель - корень
                currentDir = root;
            } else {
                string parentPath = currentPath.substr(0, lastSlash);
                currentDir = findNode(parentPath);
                
                if (!currentDir) {
                    // Откат на корень в случае ошибки
                    currentDir = root;
                }
            }
            
            return true;
        } else {
            // Относительный путь (может содержать несколько компонентов)
            target = resolvePath(path);
        }
        
        if (!target) {
            cout << "cd: " << path << ": Нет такого файла или каталога" << endl;
            return false;
        }
        
        if (!target->isDirectory()) {
            cout << "cd: " << path << ": Это не каталог" << endl;
            return false;
        }
        
        // Проверка прав execute (нужно для входа в директорию)
        if (!checkExecutePermission(target)) {
            cout << "cd: " << path << ": Отказано в доступе" << endl;
            return false;
        }
        
        currentDir = target;
        return true;
    }

    // Создать директорию
    bool mkdir(const string& name) {
        if (name.empty()) {
            cout << "mkdir: отсутствует операнд" << endl;
            return false;
        }
        
        // Если путь содержит / - используем createDirectory
        if (name.find('/') != string::npos) {
            bool result = createDirectory(name, !debugMode);
            if (!result) {
                cout << "mkdir: невозможно создать каталог '" << name << "': Отказано в доступе" << endl;
            }
            return result;
        }
        
        // Проверка прав write в текущей директории
        if (!checkWritePermission(currentDir)) {
            cout << "mkdir: невозможно создать каталог '" << name << "': Отказано в доступе" << endl;
            return false;
        }
        
        // Простое имя директории
        if (currentDir->findChild(name)) {
            cout << "mkdir: невозможно создать каталог '" << name << "': Файл существует" << endl;
            return false;
        }
        
        auto newDir = make_shared<FSNode>(name, NodeType::DIRECTORY, currentDir.get());
        currentDir->addChild(newDir, !debugMode);
        return true;
    }

    // Создать файл
    bool touch(const string& name) {
        if (name.empty()) {
            cout << "touch: отсутствует операнд" << endl;
            return false;
        }
        
        // Если путь содержит / - используем createFile для правильной обработки
        if (name.find('/') != string::npos) {
            bool result = createFile(name, "", !debugMode);
            if (!result) {
                cout << "touch: невозможно создать '" << name << "': Отказано в доступе" << endl;
            }
            return result;
        }
        
        // Простое имя файла
        if (currentDir->findChild(name)) {
            return true; // В Linux touch обновляет время, мы просто игнорируем
        }
        
        // Проверка прав write в текущей директории
        if (!checkWritePermission(currentDir)) {
            cout << "touch: невозможно создать файл '" << name << "': Отказано в доступе" << endl;
            return false;
        }
        
        auto newFile = make_shared<FSNode>(name, NodeType::FILE, currentDir.get());
        currentDir->addChild(newFile, !debugMode);
        return true;
    }

    // Вывести содержимое файла
    void cat(const string& name) {
        auto file = resolvePath(name);
        
        if (!file) {
            cout << "cat: " << name << ": Нет такого файла или каталога" << endl;
            return;
        }
        
        if (!file->isFile()) {
            cout << "cat: " << name << ": Это каталог" << endl;
            return;
        }
        
        // Проверка прав read
        if (!checkReadPermission(file)) {
            cout << "cat: " << name << ": Отказано в доступе" << endl;
            return;
        }
        
        cout << file->content.toString();
    }

    // Записать в файл (перезапись)
    bool writeFile(const string& name, const string& content) {
        auto file = resolvePath(name);
        
        if (!file) {
            // Нужно создать файл - находим родителя и имя файла
            shared_ptr<FSNode> parent;
            string fileName;
            
            size_t lastSlash = name.find_last_of('/');
            if (lastSlash != string::npos) {
                // Есть путь к родителю
                string parentPath = name.substr(0, lastSlash);
                fileName = name.substr(lastSlash + 1);
                
                if (parentPath.empty()) {
                    parent = root;
                } else {
                    parent = resolvePath(parentPath);
                }
            } else {
                // Просто имя файла в текущей директории
                parent = currentDir;
                fileName = name;
            }
            
            if (!parent || !parent->isDirectory()) {
                cout << "Ошибка: Путь не существует" << endl;
                return false;
            }
            
            // Создаём файл
            auto newFile = make_shared<FSNode>(fileName, NodeType::FILE, parent.get());
            newFile->content = Rope(content);
            parent->addChild(newFile, !debugMode);
            return true;
        }
        
        if (!file->isFile()) {
            cout << "Ошибка: " << name << " является каталогом" << endl;
            return false;
        }
        
        // Проверка прав write
        if (!checkWritePermission(file)) {
            cout << "Ошибка: " << name << ": Отказано в доступе" << endl;
            return false;
        }
        
        file->content = Rope(content);
        return true;
    }

    // Добавить в конец файла
    bool appendFile(const string& name, const string& content) {
        auto file = resolvePath(name);
        
        if (!file) {
            return writeFile(name, content);
        }
        
        if (!file->isFile()) {
            cout << "Ошибка: " << name << " является каталогом" << endl;
            return false;
        }
        
        // Проверка прав write
        if (!checkWritePermission(file)) {
            cout << "Ошибка: " << name << ": Отказано в доступе" << endl;
            return false;
        }
        
        file->content.append(content);
        return true;
    }

    // Удалить файл или директорию
    bool rm(const string& name, bool recursive = false) {
        auto target = resolvePath(name);
        
        if (!target) {
            cout << "rm: невозможно удалить '" << name << "': Нет такого файла или каталога" << endl;
            return false;
        }
        
        if (target->isDirectory() && !recursive) {
            cout << "rm: невозможно удалить '" << name << "': Это каталог (используйте -r)" << endl;
            return false;
        }
        
        // Проверка прав write (нужно право на запись в родительской директории)
        // Находим родителя и имя файла для удаления
        shared_ptr<FSNode> parent;
        string fileName;
        
        size_t lastSlash = name.find_last_of('/');
        if (lastSlash != string::npos) {
            // Есть путь к родителю
            string parentPath = name.substr(0, lastSlash);
            fileName = name.substr(lastSlash + 1);
            
            if (parentPath.empty()) {
                parent = root;
            } else {
                parent = resolvePath(parentPath);
            }
        } else {
            // Просто имя в текущей директории
            parent = currentDir;
            fileName = name;
        }
        
        if (parent) {
            // Проверка прав write в родительской директории
            if (!checkWritePermission(parent)) {
                cout << "rm: невозможно удалить '" << name << "': Отказано в доступе" << endl;
                return false;
            }
            parent->removeChild(fileName);
        }
        return true;
    }

    // Список файлов в текущей директории
    void ls(bool showDetails = false) {
        auto children = currentDir->getChildren();
        
        if (children.empty()) {
            return;
        }
        
        if (!showDetails) {
            for (const auto& child : children) {
                if (child->isDirectory()) {
                    cout << "\033[1;34m" << child->name << "/\033[0m  ";
                } else {
                    cout << child->name << "  ";
                }
            }
            cout << endl;
        } else {
            for (const auto& child : children) {
                cout << (child->isDirectory() ? "d" : "-");
                cout << child->permissions.toString() << "  ";
                cout << setw(10) << (child->isFile() ? child->content.length() : 0) << "  ";
                
                if (child->isDirectory()) {
                    cout << "\033[1;34m" << child->name << "/\033[0m" << endl;
                } else {
                    cout << child->name << endl;
                }
            }
        }
    }
    
    // Список файлов в указанной директории (поддержка абсолютных путей)
    void ls(const string& path, bool showDetails = false) {
        auto dir = resolvePath(path);
        
        if (!dir) {
            cout << "ls: невозможно получить доступ к '" << path << "': Нет такого файла или каталога" << endl;
            return;
        }
        
        if (!dir->isDirectory()) {
            cout << "ls: '" << path << "': Не является каталогом" << endl;
            return;
        }
        
        // Проверка прав read для просмотра содержимого директории
        if (!checkReadPermission(dir)) {
            cout << "ls: невозможно открыть каталог '" << path << "': Отказано в доступе" << endl;
            return;
        }
        
        auto children = dir->getChildren();
        
        if (children.empty()) {
            return;
        }
        
        if (!showDetails) {
            for (const auto& child : children) {
                if (child->isDirectory()) {
                    cout << "\033[1;34m" << child->name << "/\033[0m  ";
                } else {
                    cout << child->name << "  ";
                }
            }
            cout << endl;
        } else {
            for (const auto& child : children) {
                cout << (child->isDirectory() ? "d" : "-");
                cout << child->permissions.toString() << "  ";
                cout << setw(10) << (child->isFile() ? child->content.length() : 0) << "  ";
                
                if (child->isDirectory()) {
                    cout << "\033[1;34m" << child->name << "/\033[0m" << endl;
                } else {
                    cout << child->name << endl;
                }
            }
        }
    }

    // Изменить права доступа
    bool chmod(const string& mode, const string& name) {
        auto target = resolvePath(name);
        
        if (!target) {
            cout << "chmod: невозможно получить доступ к '" << name << "': Нет такого файла или каталога" << endl;
            return false;
        }
        
        if (mode.length() != 3) {
            cout << "chmod: неверный формат прав (используйте, например, 755)" << endl;
            return false;
        }
        
        target->permissions.owner = mode[0] - '0';
        target->permissions.group = mode[1] - '0';
        target->permissions.others = mode[2] - '0';
        
        return true;
    }

    // Найти файлы по имени
    void findFiles(const string& name) {
        vector<string> results;
        
        // Ищем во всех детях текущей директории
        auto children = currentDir->getChildren();
        string basePath = getCurrentPath();
        
        for (auto& child : children) {
            searchRecursive(child, name, basePath, results);
        }
        
        if (results.empty()) {
            return;
        }
        
        for (const auto& res : results) {
            cout << res << endl;
        }
    }

    bool createDirectory(const string& path, bool silent = false) {
        if (!silent) {
            cout << "\n[Создание директории] " << path << endl;
        }

        vector<string> components = splitPath(path);
        if (components.empty()) {
            if (!silent) {
                cout << "  [Ошибка] Неверный путь" << endl;
            }
            return false;
        }

        shared_ptr<FSNode> current = root;
        string currentPath = "";

        for (size_t i = 0; i < components.size(); i++) {
            const string& comp = components[i];
            currentPath += "/" + comp;

            auto child = current->findChild(comp);

            if (!child) {
                if (i < components.size() - 1) {
                    if (!silent) {
                        cout << "  [Ошибка] Директория '" << currentPath << "' не существует" << endl;
                    }
                    return false;
                }

                // Проверка прав write в родительской директории
                if (!checkWritePermission(current)) {
                    if (!silent) {
                        cout << "  [Ошибка] Отказано в доступе" << endl;
                    }
                    return false;
                }

                auto newDir = make_shared<FSNode>(comp, NodeType::DIRECTORY, current.get());
                current->addChild(newDir, silent);
                if (!silent) {
                    cout << "  [Успех] Создана директория: " << currentPath << endl;
                }
                return true;
            } else {
                if (!child->isDirectory()) {
                    if (!silent) {
                        cout << "  [Ошибка] '" << currentPath << "' является файлом, а не директорией" << endl;
                    }
                    return false;
                }

                if (i == components.size() - 1) {
                    if (!silent) {
                        cout << "  [Ошибка] Директория уже существует" << endl;
                    }
                    return false;
                }
                
                current = child;
            }
        }

        return false;
    }


    bool createFile(const string& path, const string& content = "", bool silent = false) {
        if (!silent) {
            cout << "\n[Создание файла] " << path << endl;
        }
        
        vector<string> components = splitPath(path);
        if (components.empty()) {
            if (!silent) {
                cout << "  [Ошибка] Неверный путь" << endl;
            }
            return false;
        }
        
        string fileName = components.back();
        components.pop_back();
        
        shared_ptr<FSNode> parent = root;
        for (const auto& comp : components) {
            parent = parent->findChild(comp);
            if (!parent || !parent->isDirectory()) {
                if (!silent) {
                    cout << "  [Ошибка] Путь не существует" << endl;
                }
                return false;
            }
        }
        
        if (parent->findChild(fileName)) {
            if (!silent) {
                cout << "  [Ошибка] Файл уже существует" << endl;
            }
            return false;
        }
        
        // Проверка прав write в родительской директории
        if (!checkWritePermission(parent)) {
            if (!silent) {
                cout << "  [Ошибка] Отказано в доступе" << endl;
            }
            return false;
        }
        
        auto newFile = make_shared<FSNode>(fileName, NodeType::FILE, parent.get());
        if (!content.empty()) {
            newFile->content = Rope(content);
        }
        parent->addChild(newFile, silent);
        
        if (!silent) {
            cout << "  [Успех] Создан файл: " << path << endl;
            if (!content.empty()) {
                cout << "  [Содержимое] " << content.length() << " символов" << endl;
            }
        }
        return true;
    }
    
    bool writeToFile(const string& path, const string& content) {
        cout << "\n[Запись в файл] " << path << endl;
        
        auto node = findNode(path);
        if (!node) {
            cout << "  [Ошибка] Файл не найден" << endl;
            return false;
        }
        
        if (!node->isFile()) {
            cout << "  [Ошибка] Это директория, а не файл" << endl;
            return false;
        }
        
        node->content.append(content);
        cout << "  [Успех] Записано " << content.length() << " символов" << endl;
        cout << "  [Rope] Текущая длина: " << node->content.length() << " символов" << endl;
        return true;
    }

    string readFile(const string& path) {
        auto node = findNode(path);
        if (!node || !node->isFile()) {
            return "";
        }
        return node->content.toString();
    }

    int findInFile(const string& path, const string& substr) {
        cout << "\n[Поиск в файле] '" << substr << "' в " << path << endl;
        
        auto node = findNode(path);
        if (!node || !node->isFile()) {
            cout << "  [Ошибка] Файл не найден" << endl;
            return -1;
        }
        
        int pos = node->content.find(substr);
        if (pos >= 0) {
            cout << "  [Найдено] Позиция: " << pos << endl;
        } else {
            cout << "  [Не найдено]" << endl;
        }
        return pos;
    }
    
    bool deleteFromFile(const string& path, const string& substr) {
        cout << "\n[Удаление из файла] '" << substr << "' из " << path << endl;
        
        auto node = findNode(path);
        if (!node || !node->isFile()) {
            cout << "  [Ошибка] Файл не найден" << endl;
            return false;
        }
        
        return node->content.deleteSubstring(substr);
    }

    bool insertInFile(const string& path, int pos, const string& text) {
        cout << "\n[Вставка в файл] В " << path << " на позицию " << pos << endl;
        
        auto node = findNode(path);
        if (!node || !node->isFile()) {
            cout << "  [Ошибка] Файл не найден" << endl;
            return false;
        }
        
        node->content.insert(pos, text);
        return true;
    }

    void listDirectory(const string& path) {
        cout << "\n[Список файлов] " << path << endl;
        
        auto node = findNode(path);
        if (!node) {
            cout << "  [Ошибка] Путь не найден" << endl;
            return;
        }
        
        if (!node->isDirectory()) {
            cout << "  [Ошибка] Это файл, а не директория" << endl;
            return;
        }
        
        auto children = node->getChildren();
        if (children.empty()) {
            cout << "  [Пусто]" << endl;
            return;
        }
        
        cout << "  Права      Тип   Hash        Имя" << endl;
        cout << "  ---------  ----  ----------  ----" << endl;
        
        for (const auto& child : children) {
            cout << "  " << child->permissions.toString() << "  ";
            cout << (child->isDirectory() ? "DIR " : "FILE") << "  ";
            cout << setw(10) << HashFunction::hash(child->name) << "  ";
            
            if (child->isDirectory()) {
                cout << "\033[1;34m" << child->name << "/\033[0m" << endl;
            } else {
                cout << child->name;
                if (!child->content.empty()) {
                    cout << " (" << child->content.length() << " bytes)";
                }
                cout << endl;
            }
        }
        
        node->htree.printStats();
    }
    
    vector<string> search(const string& name) {
        cout << "\n[Глобальный поиск] '" << name << "'" << endl;
        
        vector<string> results;
        searchRecursive(root, name, "", results);
        
        if (results.empty()) {
            cout << "  [Не найдено]" << endl;
        } else {
            cout << "  [Найдено " << results.size() << " результат(ов)]:" << endl;
            for (const auto& res : results) {
                cout << "    " << res << endl;
            }
        }
        
        return results;
    }

    bool remove(const string& path) {
        cout << "\n[Удаление] " << path << endl;
        
        auto node = findNode(path);
        if (!node) {
            cout << "  [Ошибка] Путь не найден" << endl;
            return false;
        }
        
        if (!node->parent) {
            cout << "  [Ошибка] Нельзя удалить корневую директорию" << endl;
            return false;
        }
        
        FSNode* parent = node->parent;
        if (parent->removeChild(node->name)) {
            cout << "  [Успех] Удалено из H-Tree" << endl;
            return true;
        }
        
        return false;
    }

    bool setPermissions(const string& path, int owner, int group, int others) {
        cout << "\n[Установка прав] " << path << endl;
        
        auto node = findNode(path);
        if (!node) {
            cout << "  [Ошибка] Путь не найден" << endl;
            return false;
        }
        
        node->permissions.owner = owner;
        node->permissions.group = group;
        node->permissions.others = others;
        
        cout << "  [Успех] Права установлены: " << node->permissions.toString() << endl;
        return true;
    }

    void visualize() {
        cout << "\n" << string(70, '=') << endl;
        cout << "ВИЗУАЛИЗАЦИЯ ФАЙЛОВОЙ СИСТЕМЫ" << endl;
        cout << string(70, '=') << endl;
        cout << "/" << endl;
        
        auto children = root->getChildren();
        for (size_t i = 0; i < children.size(); i++) {
            bool isLast = (i == children.size() - 1);
            visualizeTree(children[i], "", isLast);
        }
        cout << string(70, '=') << endl;
    }

    void catFile(const string& path) {
        cout << "\n[Чтение файла] " << path << endl;
        
        auto node = findNode(path);
        if (!node) {
            cout << "  [Ошибка] Файл не найден" << endl;
            return;
        }
        
        if (!node->isFile()) {
            cout << "  [Ошибка] Это директория" << endl;
            return;
        }
        
        string content = node->content.toString();
        cout << "  [Содержимое]:" << endl;
        cout << "  " << string(50, '-') << endl;
        
        if (content.empty()) {
            cout << "  (пусто)" << endl;
        } else {
            // Выводим с отступом
            stringstream ss(content);
            string line;
            while (getline(ss, line)) {
                cout << "  " << line << endl;
            }
        }
        
        cout << "  " << string(50, '-') << endl;
        cout << "  [Размер] " << node->content.length() << " символов" << endl;
    }
};

// Разбить команду на токены
vector<string> tokenize(const string& line) {
    vector<string> tokens;
    stringstream ss(line);
    string token;
    
    while (ss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

// Парсинг перенаправления и конкатенации
struct Command {
    vector<string> args;
    string redirect_out;  // > file
    string redirect_append;  // >> file
    bool has_redirect = false;
    bool is_append = false;
};

Command parseCommand(const string& line) {
    Command cmd;
    stringstream ss(line);
    string token;
    bool redirect_mode = false;
    
    while (ss >> token) {
        if (token == ">") {
            redirect_mode = true;
            cmd.has_redirect = true;
            cmd.is_append = false;
        } else if (token == ">>") {
            redirect_mode = true;
            cmd.has_redirect = true;
            cmd.is_append = true;
        } else if (redirect_mode) {
            if (cmd.is_append) {
                cmd.redirect_append = token;
            } else {
                cmd.redirect_out = token;
            }
            redirect_mode = false;
        } else {
            cmd.args.push_back(token);
        }
    }
    
    return cmd;
}

void runTerminal(FileSystem& fs) {
    string line;
    
    cout << "\n╔════════════════════════════════════════════════════════════╗" << endl;
    cout << "║     Linux-подобный терминал файловой системы              ║" << endl;
    cout << "║     Введите 'help' для списка команд, 'exit' для выхода   ║" << endl;
    cout << "╚════════════════════════════════════════════════════════════╝" << endl;
    
    while (true) {
        // Промпт как в Linux
        const char* user = getenv("USER");
        if (!user) user = "user";
        cout << "\033[1;32m" << user << "@filesystem"  
             << "\033[0m:\033[1;34m" << fs.getCurrentPath() 
             << "\033[0m$ ";
        
        if (!getline(cin, line)) break;
        
        if (line.empty()) continue;
        
        Command cmd = parseCommand(line);
        if (cmd.args.empty()) continue;
        
        string command = cmd.args[0];
        
        // Обработка команд
        if (command == "exit" || command == "quit") {
            cout << "Выход из терминала..." << endl;
            break;
        }
        else if (command == "help") {
            cout << "Доступные команды:" << endl;
            cout << "  pwd              - показать текущий путь" << endl;
            cout << "  ls [-l]          - список файлов" << endl;
            cout << "  cd <path>        - перейти в директорию" << endl;
            cout << "  mkdir <name>     - создать директорию" << endl;
            cout << "  touch <name>     - создать пустой файл" << endl;
            cout << "  cat <file>       - вывести содержимое файла" << endl;
            cout << "  echo <text>      - вывести текст (можно с > file или >> file)" << endl;
            cout << "  rm <name>        - удалить файл" << endl;
            cout << "  rm -r <name>     - удалить директорию рекурсивно" << endl;
            cout << "  chmod <mode> <f> - изменить права (например: chmod 755 file)" << endl;
            cout << "  find <pattern>   - найти файлы по шаблону" << endl;
            cout << "  tree             - показать дерево файловой системы" << endl;
            cout << "  clear            - очистить экран" << endl;
            cout << "  debug            - переключить режим отладки" << endl;
            cout << "  exit             - выход" << endl;
        }
        else if (command == "pwd") {
            cout << fs.getCurrentPath() << endl;
        }
        else if (command == "ls") {
            bool showDetails = false;
            string path = "";
            
            // Парсим аргументы: ls [-l] [path]
            for (size_t i = 1; i < cmd.args.size(); i++) {
                if (cmd.args[i] == "-l") {
                    showDetails = true;
                } else {
                    path = cmd.args[i];
                }
            }
            
            if (!path.empty()) {
                fs.ls(path, showDetails);
            } else {
                fs.ls(showDetails);
            }
        }
        else if (command == "cd") {
            if (cmd.args.size() < 2) {
                fs.changeDirectory("/");
            } else {
                fs.changeDirectory(cmd.args[1]);
            }
        }
        else if (command == "mkdir") {
            if (cmd.args.size() < 2) {
                cout << "mkdir: отсутствует операнд" << endl;
            } else {
                fs.mkdir(cmd.args[1]);
            }
        }
        else if (command == "touch") {
            if (cmd.args.size() < 2) {
                cout << "touch: отсутствует операнд" << endl;
            } else {
                fs.touch(cmd.args[1]);
            }
        }
        else if (command == "cat") {
            if (cmd.args.size() < 2) {
                cout << "cat: отсутствует операнд" << endl;
            } else {
                fs.cat(cmd.args[1]);
            }
        }
        else if (command == "echo") {
            if (cmd.args.size() < 2) {
                cout << endl;
            } else {
                // Собираем всё после echo
                string text;
                for (size_t i = 1; i < cmd.args.size(); i++) {
                    if (i > 1) text += " ";
                    text += cmd.args[i];
                }
                
                if (cmd.has_redirect) {
                    text += "\n";
                    if (cmd.is_append) {
                        fs.appendFile(cmd.redirect_append, text);
                    } else {
                        fs.writeFile(cmd.redirect_out, text);
                    }
                } else {
                    cout << text << endl;
                }
            }
        }
        else if (command == "rm") {
            if (cmd.args.size() < 2) {
                cout << "rm: отсутствует операнд" << endl;
            } else if (cmd.args.size() == 2) {
                fs.rm(cmd.args[1], false);
            } else if (cmd.args[1] == "-r" && cmd.args.size() >= 3) {
                fs.rm(cmd.args[2], true);
            } else {
                fs.rm(cmd.args[1], false);
            }
        }
        else if (command == "chmod") {
            if (cmd.args.size() < 3) {
                cout << "chmod: отсутствует операнд" << endl;
            } else {
                fs.chmod(cmd.args[1], cmd.args[2]);
            }
        }
        else if (command == "find") {
            if (cmd.args.size() < 2) {
                cout << "find: отсутствует операнд" << endl;
            } else {
                fs.findFiles(cmd.args[1]);
            }
        }
        else if (command == "tree") {
            fs.visualize();
        }
        else if (command == "clear") {
            cout << "\033[2J\033[1;1H";
        }
        else if (command == "debug") {
            fs.toggleDebug();
        }
        else {
            cout << command << ": команда не найдена" << endl;
        }
    }
}

int main() {
    FileSystem fs;

    fs.createDirectory("/home");
    fs.createDirectory("/home/user");
    fs.createDirectory("/etc");
    fs.createDirectory("/var");
    fs.createDirectory("/tmp");

    runTerminal(fs);
    
    return 0;
}
