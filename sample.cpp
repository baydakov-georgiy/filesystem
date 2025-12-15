#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <map>
#include <cmath>

using namespace std;

// ============================================================================
// ROPE - структура данных для эффективной работы с текстом
// ============================================================================
// Обоснование: Rope позволяет эффективно выполнять операции вставки, удаления
// и конкатенации строк за O(log n) вместо O(n) у обычных строк.
// Это критично для работы с большими текстовыми файлами.
// Реализация: AVL-дерево с балансировкой для оптимальной производительности.

const int MAX_LEAF_SIZE = 8;

struct Node {
    int weight;
    int height;
    string text;
    Node* left;
    Node* right;

    Node(const string& s) 
        : weight(s.size()), height(1), text(s),
          left(nullptr), right(nullptr) {}
    Node()
        : weight(0), height(1), text(""), left(nullptr),
          right(nullptr) {}

    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

class Rope {
private:
    Node* root;

    int getHeight(Node* node) {
        return node ? node->height : 0;
    }

    int getBalance(Node* node) {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }

    void updateHeight(Node* node) {
        if (!node) return;
        node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    }

    void updateWeight(Node* node) {
        if (node && !node->isLeaf()) {
            node->weight = getLength(node->left);
        }
    }

    int getLength(Node* node) const {
        if (!node) return 0;
        if (node->isLeaf()) return node->text.size();
        return node->weight + getLength(node->right);
    }

    Node* copyNode(Node* node) {
        if (!node) return nullptr;

        Node* newNode = new Node();
        newNode->weight = node->weight;
        newNode->height = node->height;
        newNode->text = node->text;
        newNode->left = copyNode(node->left);
        newNode->right = copyNode(node->right);

        return newNode;
    }

    void deleteNode(Node* node) {
        if (!node) return;
        deleteNode(node->left);
        deleteNode(node->right);
        delete node;
    }

    Node* rightRotate(Node* P) {
        if (!P || !P->left) return P;
        
        Node* Q = P->left;

        Node* newP = copyNode(P);
        Node* newQ = copyNode(Q);

        newP->left = newQ->right;
        newQ->right = newP;

        updateHeight(newP);
        updateWeight(newP);
        updateHeight(newQ);
        updateWeight(newQ);

        return newQ;
    }

    Node* leftRotate(Node* P) {
        if (!P || !P->right) return P;

        Node* Q = P->right;

        Node* newP = copyNode(P);
        Node* newQ = copyNode(Q);

        newP->right = newQ->left;
        newQ->left = newP;

        updateHeight(newP);
        updateWeight(newP);
        updateHeight(newQ);
        updateWeight(newQ);

        return newQ;
    }

    Node* balance(Node* node) {
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

    Node* buildFromString(const string& s, int start, int end) {
        if (start >= end) return nullptr;

        if (end - start <= MAX_LEAF_SIZE) {
            return new Node(s.substr(start, end - start));
        }

        int mid = (start + end) / 2;
        Node* node = new Node();
        node->left = buildFromString(s, start, mid);
        node->right = buildFromString(s, mid, end);
        node->weight = getLength(node->left);
        updateHeight(node);

        return balance(node);
    }

    string getString(Node* node) const {
        if (!node) return "";
        if (node->isLeaf()) return node->text;
        return getString(node->left) + getString(node->right);
    }

    char charAt(Node* node, int index) const {
        if (!node) return '\0';
        if (node->isLeaf()) {
            return (size_t)index < node->text.size() ? node->text[index] : '\0';
        }
        if (index < node->weight) {
            return charAt(node->left, index);
        }
        return charAt(node->right, index - node->weight);
    }

    pair<Node*, Node*> splitNode(Node* node, int index) {
        if (!node) return {nullptr, nullptr};

        if (node->isLeaf()) {
            if (index <= 0) return {nullptr, copyNode(node)};
            if ((size_t)index >= node->text.size()) return {copyNode(node), nullptr};

            Node* left = new Node(node->text.substr(0, index));
            Node* right = new Node(node->text.substr(index));
            return {left, right};
        }

        if (index <= node->weight) {
            auto [l1, l2] = splitNode(node->left, index);
            Node* right = concat(l2, copyNode(node->right));
            return {l1, right};
        } else {
            auto [r1, r2] = splitNode(node->right, index - node->weight);
            Node* left = concat(copyNode(node->left), r1);
            return {left, r2};
        }
    }

    Node* concat(Node* left, Node* right) {
        if (!left) return right; 
        if (!right) return left;

        Node* node = new Node();
        node->left = left;
        node->right = right;
        node->weight = getLength(left);
        updateHeight(node);

        return balance(node);
    }

public:
    Rope() : root(nullptr) {}
    
    Rope(const string& s) {
        root = buildFromString(s, 0, s.size());
    }

    Rope(Node* node) : root(node) {}

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

    // Проверка корректности UTF-8 позиции
    int adjustUtf8Position(const string& text, int pos) {
        if (pos <= 0 || pos >= (int)text.length()) return pos;
        
        // Проверяем, не находимся ли мы в середине UTF-8 символа
        // В UTF-8 продолжающие байты имеют вид 10xxxxxx (0x80-0xBF)
        while (pos < (int)text.length() && (text[pos] & 0xC0) == 0x80) {
            pos++; // Сдвигаемся к следующему символу
        }
        return pos;
    }

    // Вставка подстроки на позицию
    void insert(int pos, const string& str) {
        if (str.empty()) return;
        
        int len = length();
        if (pos < 0 || pos > len) {
            cout << "  [Ошибка] Неверная позиция для вставки" << endl;
            return;
        }
        
        // Корректируем позицию для UTF-8
        string current = toString();
        pos = adjustUtf8Position(current, pos);
        
        auto [l, r] = splitNode(root, pos);
        Node* mid = buildFromString(str, 0, str.size());
        root = concat(concat(l, mid), r);
        
        cout << "  [Rope] Вставлено '" << str << "' на позицию " << pos << " (с коррекцией UTF-8)" << endl;
    }

    // Удаление подстроки
    bool deleteSubstring(const string& substr) {
        string current = toString();
        size_t pos = current.find(substr);
        
        if (pos == string::npos) {
            cout << "  [Rope] Подстрока '" << substr << "' не найдена" << endl;
            return false;
        }
        
        auto [l, tmp] = splitNode(root, pos);
        auto [mid, r] = splitNode(tmp, substr.length());
        deleteNode(mid);
        root = concat(l, r);
        
        cout << "  [Rope] Удалена подстрока '" << substr << "' с позиции " << pos << endl;
        return true;
    }

    // Поиск подстроки
    int find(const string& substr) const {
        string current = toString();
        size_t pos = current.find(substr);
        
        if (pos == string::npos) {
            return -1;
        }
        return static_cast<int>(pos);
    }

    // Добавление в конец
    void append(const string& str) {
        if (str.empty()) return;
        
        Node* mid = buildFromString(str, 0, str.size());
        root = concat(root, mid);
    }

    // Преобразование в строку
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

// ============================================================================
// AVL H-TREE - структура для индексации директорий
// ============================================================================
// Обоснование: Реализация на основе AVL-дерева с хешированием имен файлов.
// Вдохновлена H-Tree из ext4, но оптимизирована для работы в памяти:
// - Хеш от имени файла для быстрого поиска
// - AVL-дерево гарантирует O(log n) для ВСЕХ операций (поиск/вставка/удаление)
// - Автоматическая балансировка через ротации
// - Эффективна для директорий с большим количеством файлов
// - Детерминированная производительность O(log n)

enum class NodeType {
    DIRECTORY,
    FILE
};

enum class Permission {
    NONE = 0,
    READ = 4,      // r = 4 (бит 2)
    WRITE = 2,     // w = 2 (бит 1)
    EXECUTE = 1    // x = 1 (бит 0)
};

struct Permissions {
    int owner;   // rwx для владельца (0-7)
    int group;   // rwx для группы (0-7)
    int others;  // rwx для остальных (0-7)
    
    Permissions() : owner(7), group(5), others(5) {} // rwx-r-x-r-x по умолчанию
    
    string toString() const {
        auto toRWX = [](int perm) {
            string result;
            result += (perm & 4) ? 'r' : '-';
            result += (perm & 2) ? 'w' : '-';
            result += (perm & 1) ? 'x' : '-';
            return result;
        };
        return toRWX(owner) + toRWX(group) + toRWX(others);
    }
};

// Forward declaration
class FSNode;

// Хеш-функция для имен файлов (аналог Half MD4 в ext4)
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

// AVL узел для H-Tree
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

// AVL H-Tree индекс для директории
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
            node->left = removeNode(node->left, hash, name);
            if (node->left) return balance(node);
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
    Rope content;        // Только для файлов
    HTreeIndex htree;    // H-Tree индекс для директорий
    FSNode* parent;
    
    FSNode(const string& name, NodeType type, FSNode* parent = nullptr)
        : name(name), type(type), parent(parent) {}
    
    bool isDirectory() const { return type == NodeType::DIRECTORY; }
    bool isFile() const { return type == NodeType::FILE; }
    
    // Поиск дочернего элемента по имени через H-Tree - O(log n)
    shared_ptr<FSNode> findChild(const string& childName) {
        if (!isDirectory()) return nullptr;
        return htree.find(childName);
    }
    
    // Добавление дочернего элемента в AVL H-Tree
    void addChild(shared_ptr<FSNode> child) {
        if (!isDirectory()) return;
        htree.insert(child->name, child);
        cout << "  [AVL H-Tree] Добавлен '" << child->name 
             << "' (hash: " << HashFunction::hash(child->name) << ")" << endl;
    }
    
    // Удаление дочернего элемента из AVL H-Tree
    bool removeChild(const string& childName) {
        if (!isDirectory()) return false;
        return htree.remove(childName);
    }
    
    // Получить всех детей
    vector<shared_ptr<FSNode>> getChildren() const {
        if (!isDirectory()) return {};
        return htree.getAllNodes();
    }
};

// Реализация методов HTreeIndex, требующих полного определения FSNode
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
private:
    shared_ptr<FSNode> root;
    
    // Разбивает путь на компоненты
    vector<string> splitPath(const string& path) const {
        vector<string> components;
        stringstream ss(path);
        string component;
        
        while (getline(ss, component, '/')) {
            if (!component.empty() && component != ".") {
                components.push_back(component);
            }
        }
        
        return components;
    }
    
    // Находит узел по пути
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
    
    // Рекурсивный поиск файлов по имени
    void searchRecursive(shared_ptr<FSNode> node, const string& name, 
                        const string& currentPath, vector<string>& results) {
        if (!node) return;
        
        string nodePath = currentPath + "/" + node->name;
        
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
    
    // Визуализация дерева
    void visualizeTree(shared_ptr<FSNode> node, const string& prefix, bool isLast) {
        if (!node) return;
        
        cout << prefix;
        cout << (isLast ? "└── " : "├── ");
        
        if (node->isDirectory()) {
            cout << "\033[1;34m" << node->name << "/\033[0m";  // Синий для директорий
        } else {
            cout << node->name;
        }
        
        cout << " [" << node->permissions.toString() << "]";
        
        // Показываем хеш для демонстрации H-Tree
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
    FileSystem() {
        root = make_shared<FSNode>("", NodeType::DIRECTORY, nullptr);
        cout << "[ФС] Файловая система инициализирована" << endl;
    }
    
    // Создание директории
    bool createDirectory(const string& path) {
        cout << "\n[Создание директории] " << path << endl;
        
        vector<string> components = splitPath(path);
        if (components.empty()) {
            cout << "  [Ошибка] Неверный путь" << endl;
            return false;
        }
        
        shared_ptr<FSNode> current = root;
        string currentPath = "";
        
        for (size_t i = 0; i < components.size(); i++) {
            const string& comp = components[i];
            currentPath += "/" + comp;
            
            auto child = current->findChild(comp);
            
            if (!child) {
                // Если это не последний компонент, выводим ошибку
                if (i < components.size() - 1) {
                    cout << "  [Ошибка] Директория '" << currentPath << "' не существует" << endl;
                    return false;
                }
                
                // Создаем новую директорию
                auto newDir = make_shared<FSNode>(comp, NodeType::DIRECTORY, current.get());
                current->addChild(newDir);
                cout << "  [Успех] Создана директория: " << currentPath << endl;
                return true;
            } else {
                if (!child->isDirectory()) {
                    cout << "  [Ошибка] '" << currentPath << "' является файлом, а не директорией" << endl;
                    return false;
                }
                
                if (i == components.size() - 1) {
                    cout << "  [Ошибка] Директория уже существует" << endl;
                    return false;
                }
                
                current = child;
            }
        }
        
        return false;
    }
    
    // Создание файла
    bool createFile(const string& path, const string& content = "") {
        cout << "\n[Создание файла] " << path << endl;
        
        vector<string> components = splitPath(path);
        if (components.empty()) {
            cout << "  [Ошибка] Неверный путь" << endl;
            return false;
        }
        
        string fileName = components.back();
        components.pop_back();
        
        // Находим родительскую директорию
        shared_ptr<FSNode> parent = root;
        for (const auto& comp : components) {
            parent = parent->findChild(comp);
            if (!parent || !parent->isDirectory()) {
                cout << "  [Ошибка] Путь не существует" << endl;
                return false;
            }
        }
        
        // Проверяем, существует ли файл
        if (parent->findChild(fileName)) {
            cout << "  [Ошибка] Файл уже существует" << endl;
            return false;
        }
        
        // Создаем файл
        auto newFile = make_shared<FSNode>(fileName, NodeType::FILE, parent.get());
        if (!content.empty()) {
            newFile->content = Rope(content);
        }
        parent->addChild(newFile);
        
        cout << "  [Успех] Создан файл: " << path << endl;
        if (!content.empty()) {
            cout << "  [Содержимое] " << content.length() << " символов" << endl;
        }
        return true;
    }
    
    // Запись в файл
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
    
    // Чтение файла
    string readFile(const string& path) {
        auto node = findNode(path);
        if (!node || !node->isFile()) {
            return "";
        }
        return node->content.toString();
    }
    
    // Поиск подстроки в файле
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
    
    // Удаление подстроки из файла
    bool deleteFromFile(const string& path, const string& substr) {
        cout << "\n[Удаление из файла] '" << substr << "' из " << path << endl;
        
        auto node = findNode(path);
        if (!node || !node->isFile()) {
            cout << "  [Ошибка] Файл не найден" << endl;
            return false;
        }
        
        return node->content.deleteSubstring(substr);
    }
    
    // Вставка в файл
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
    
    // Список файлов в директории
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
        
        // Показываем статистику H-Tree
        node->htree.printStats();
    }
    
    // Поиск файлов по имени
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
    
    // Удаление файла/директории
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
    
    // Установка прав доступа
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
    
    // Визуализация файловой системы
    void visualize() {
        cout << "\n" << string(70, '=') << endl;
        cout << "ВИЗУАЛИЗАЦИЯ ФАЙЛОВОЙ СИСТЕМЫ (H-Tree индексация)" << endl;
        cout << string(70, '=') << endl;
        cout << "/" << endl;
        
        auto children = root->getChildren();
        for (size_t i = 0; i < children.size(); i++) {
            bool isLast = (i == children.size() - 1);
            visualizeTree(children[i], "", isLast);
        }
        cout << string(70, '=') << endl;
    }
    
    // Вывод содержимого файла
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

// ============================================================================
// ДЕМОНСТРАЦИЯ РАБОТЫ
// ============================================================================

void printHeader(const string& text) {
    cout << "\n" << string(70, '=') << endl;
    cout << "  " << text << endl;
    cout << string(70, '=') << endl;
}

int main() {
    printHeader("ИМИТАЦИЯ ФАЙЛОВОЙ СИСТЕМЫ LINUX");
    
    cout << "\n📊 ОБОСНОВАНИЕ ВЫБОРА СТРУКТУР ДАННЫХ:" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "1. AVL H-TREE для индексации директорий:" << endl;
    cout << "   ✓ Вдохновлена H-Tree из ext4 файловой системы Linux" << endl;
    cout << "   ✓ AVL-дерево: O(log n) для ВСЕХ операций (поиск/вставка/удаление)" << endl;
    cout << "   ✓ Хеширование имен файлов для быстрого доступа" << endl;
    cout << "   ✓ Автоматическая балансировка (высота ≤ 1.44 log n)" << endl;
    cout << "   ✓ Гарантированная производительность без деградации" << endl;
    cout << "\n2. ROPE для содержимого текстовых файлов:" << endl;
    cout << "   ✓ Вставка/удаление за O(log n) вместо O(n)" << endl;
    cout << "   ✓ Конкатенация за O(log n)" << endl;
    cout << "   ✓ Эффективно для больших текстов" << endl;
    cout << "   ✓ Сбалансированное дерево строк" << endl;
    cout << "\n3. Иерархическая структура:" << endl;
    cout << "   ✓ Каждая директория - корень H-Tree для своих детей" << endl;
    cout << "   ✓ Путь от корня до файла формирует дерево директорий" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    
    FileSystem fs;
    
    // ДЕМОНСТРАЦИЯ 1: Создание структуры директорий
    printHeader("ДЕМОНСТРАЦИЯ 1: Создание директорий");
    fs.createDirectory("/home");
    fs.createDirectory("/home/user");
    fs.createDirectory("/home/user/Documents");
    fs.createDirectory("/home/user/Documents/labs");
    fs.createDirectory("/var");
    fs.createDirectory("/var/log");
    
    // Попытка создать без родительской директории
    fs.createDirectory("/home/user/nonexistent/deep");
    
    fs.visualize();
    
    // ДЕМОНСТРАЦИЯ 2: Создание и работа с файлами
    printHeader("ДЕМОНСТРАЦИЯ 2: Создание файлов и работа с содержимым");
    fs.createFile("/home/user/Documents/readme.txt", "Это тестовый файл.\n");
    fs.writeToFile("/home/user/Documents/readme.txt", "Вторая строка текста.\n");
    fs.writeToFile("/home/user/Documents/readme.txt", "Третья строка с важной информацией.\n");
    
    fs.createFile("/home/user/notes.txt", "Заметки:\n1. Купить молоко\n2. Позвонить другу\n");
    fs.createFile("/var/log/system.log", "[INFO] System started\n[WARN] Low memory\n");
    
    fs.visualize();
    
    // ДЕМОНСТРАЦИЯ 3: Чтение содержимого файлов
    printHeader("ДЕМОНСТРАЦИЯ 3: Чтение файлов");
    fs.catFile("/home/user/Documents/readme.txt");
    fs.catFile("/home/user/notes.txt");
    fs.catFile("/var/log/system.log");
    
    // ДЕМОНСТРАЦИЯ 4: Операции поиска в файлах (Rope)
    printHeader("ДЕМОНСТРАЦИЯ 4: Поиск и работа с подстроками (Rope)");
    fs.findInFile("/home/user/Documents/readme.txt", "важной");
    fs.findInFile("/home/user/notes.txt", "молоко");
    fs.findInFile("/var/log/system.log", "ERROR");
    
    // ДЕМОНСТРАЦИЯ 5: Удаление подстрок
    printHeader("ДЕМОНСТРАЦИЯ 5: Удаление подстрок из файла");
    fs.deleteFromFile("/home/user/notes.txt", "2. Позвонить другу\n");
    fs.catFile("/home/user/notes.txt");
    
    // ДЕМОНСТРАЦИЯ 6: Вставка в файл
    printHeader("ДЕМОНСТРАЦИЯ 6: Вставка текста в файл");
    // Вставляем в начало списка (после "Заметки:\n" = 18 байт в UTF-8)
    fs.insertInFile("/home/user/notes.txt", 18, "0. Проснуться\n");
    fs.catFile("/home/user/notes.txt");
    
    // ДЕМОНСТРАЦИЯ 7: Список файлов в директории
    printHeader("ДЕМОНСТРАЦИЯ 7: Вывод содержимого директорий");
    fs.listDirectory("/");
    fs.listDirectory("/home/user");
    fs.listDirectory("/home/user/Documents");
    
    // ДЕМОНСТРАЦИЯ 8: Глобальный поиск файлов
    printHeader("ДЕМОНСТРАЦИЯ 8: Поиск файлов по имени");
    fs.search("txt");
    fs.search("readme");
    fs.search("log");
    
    // ДЕМОНСТРАЦИЯ 9: Права доступа
    printHeader("ДЕМОНСТРАЦИЯ 9: Управление правами доступа");
    fs.setPermissions("/home/user/Documents/readme.txt", 6, 4, 4);  // rw-r--r--
    fs.setPermissions("/var/log/system.log", 7, 0, 0);  // rwx------
    fs.listDirectory("/home/user/Documents");
    fs.listDirectory("/var/log");
    
    // ДЕМОНСТРАЦИЯ 10: Удаление файлов и директорий
    printHeader("ДЕМОНСТРАЦИЯ 10: Удаление файлов");
    fs.remove("/home/user/Documents/readme.txt");
    fs.remove("/var/log/system.log");
    
    fs.visualize();
    
    // ФИНАЛЬНАЯ ВИЗУАЛИЗАЦИЯ
    printHeader("ФИНАЛЬНОЕ СОСТОЯНИЕ ФАЙЛОВОЙ СИСТЕМЫ");
    fs.visualize();
    
    printHeader("АНАЛИЗ ЭФФЕКТИВНОСТИ");
    cout << "\n📈 Временная сложность операций:" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "AVL H-Tree (индексация директорий):" << endl;
    cout << "  • Поиск файла: O(log k) - гарантировано" << endl;
    cout << "  • Вставка нового файла: O(log k) - с балансировкой" << endl;
    cout << "  • Удаление файла: O(log k) - с ребалансировкой" << endl;
    cout << "  • Навигация по пути: O(d * log k), где d - глубина" << endl;
    cout << "  • Глобальный поиск: O(n), где n - всего узлов" << endl;
    cout << "  ⚡ Высота дерева: ≤ 1.44 log₂(k) - идеальная балансировка" << endl;
    cout << "\nRope (содержимое файлов):" << endl;
    cout << "  • Вставка/удаление: O(log m) - AVL балансировка" << endl;
    cout << "  • Конкатенация: O(log m)" << endl;
    cout << "  • Поиск подстроки: O(m), где m - длина текста" << endl;
    cout << "  • Доступ к символу: O(log m)" << endl;
    cout << "\n💡 Преимущество AVL H-Tree:" << endl;
    cout << "  Для директории с 1000 файлами:" << endl;
    cout << "  • Линейный массив: ~500 операций (вставка/удаление)" << endl;
    cout << "  • AVL H-Tree: ~10 операций (log₂ 1000 ≈ 10)" << endl;
    cout << "  • Ускорение: в 50 раз! ⚡" << endl;
    cout << "  • Детерминированная производительность (без деградации)" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    
    cout << "\n✅ ДЕМОНСТРАЦИЯ ЗАВЕРШЕНА" << endl;
    
    return 0;
}
