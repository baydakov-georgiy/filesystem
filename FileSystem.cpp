#include "FileSystem.h"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

FileSystem::FileSystem() : debugMode(false) {
    root = make_shared<FSNode>("", NodeType::DIRECTORY, nullptr);
    currentDir = root;
    cout << "[ФС] Файловая система инициализирована" << endl;
}

void FileSystem::toggleDebug() {
    debugMode = !debugMode;
    cout << "Режим отладки: " << (debugMode ? "ВКЛ" : "ВЫКЛ") << endl;
}

bool FileSystem::isDebugMode() const {
    return debugMode;
}

vector<string> FileSystem::splitPath(const string& path) const {
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

shared_ptr<FSNode> FileSystem::findNode(const string& path) {
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

shared_ptr<FSNode> FileSystem::resolvePath(const string& path) {
    if (path.empty()) {
        return nullptr;
    }
    
    if (path[0] == '/') {
        return findNode(path);
    }
    
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
            if (!current->parent) {
                continue;
            }
            
            FSNode* parentRaw = current->parent;
            
            if (parentRaw == root.get()) {
                current = root;
            } else {
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
            continue;
        } else {
            current = current->findChild(comp);
            if (!current) {
                return nullptr;
            }
        }
    }
    
    return current;
}

string FileSystem::getPathToNode(shared_ptr<FSNode> node) {
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

bool FileSystem::checkReadPermission(shared_ptr<FSNode> node) {
    if (!node) return false;
    return (node->permissions.owner & 4) != 0;
}

bool FileSystem::checkWritePermission(shared_ptr<FSNode> node) {
    if (!node) return false;
    return (node->permissions.owner & 2) != 0;
}

bool FileSystem::checkExecutePermission(shared_ptr<FSNode> node) {
    if (!node) return false;
    return (node->permissions.owner & 1) != 0;
}

void FileSystem::searchRecursive(shared_ptr<FSNode> node, const string& name, 
                    const string& currentPath, vector<string>& results) {
    if (!node) return;
    
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

void FileSystem::visualizeTree(shared_ptr<FSNode> node, const string& prefix, bool isLast) {
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

string FileSystem::getCurrentPath() const {
    if (currentDir == root) return "/";
    
    string path;
    FSNode* node = currentDir.get();
    
    while (node && node->parent) {
        path = "/" + node->name + path;
        node = node->parent;
    }
    
    return path.empty() ? "/" : path;
}

bool FileSystem::changeDirectory(const string& path) {
    if (path == "/") {
        currentDir = root;
        return true;
    }
    
    shared_ptr<FSNode> target;
    
    if (path[0] == '/') {
        target = findNode(path);
    } else if (path == "..") {
        if (!currentDir->parent) {
            return true;
        }
        
        FSNode* parentRaw = currentDir->parent;
        
        if (parentRaw == root.get()) {
            currentDir = root;
            return true;
        }
        
        string currentPath = getCurrentPath();
        size_t lastSlash = currentPath.find_last_of('/');
        
        if (lastSlash == 0) {
            currentDir = root;
        } else {
            string parentPath = currentPath.substr(0, lastSlash);
            currentDir = findNode(parentPath);
            
            if (!currentDir) {
                currentDir = root;
            }
        }
        
        return true;
    } else {
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
    
    if (!checkExecutePermission(target)) {
        cout << "cd: " << path << ": Отказано в доступе" << endl;
        return false;
    }
    
    currentDir = target;
    return true;
}

bool FileSystem::mkdir(const string& name) {
    if (name.empty()) {
        cout << "mkdir: отсутствует операнд" << endl;
        return false;
    }
    
    if (name.find('/') != string::npos) {
        bool result = createDirectory(name, !debugMode);
        if (!result) {
            cout << "mkdir: невозможно создать каталог '" << name << "': Отказано в доступе" << endl;
        }
        return result;
    }
    
    if (!checkWritePermission(currentDir)) {
        cout << "mkdir: невозможно создать каталог '" << name << "': Отказано в доступе" << endl;
        return false;
    }
    
    if (currentDir->findChild(name)) {
        cout << "mkdir: невозможно создать каталог '" << name << "': Файл существует" << endl;
        return false;
    }
    
    auto newDir = make_shared<FSNode>(name, NodeType::DIRECTORY, currentDir.get());
    currentDir->addChild(newDir, !debugMode);
    return true;
}

bool FileSystem::touch(const string& name) {
    if (name.empty()) {
        cout << "touch: отсутствует операнд" << endl;
        return false;
    }
    
    if (name.find('/') != string::npos) {
        bool result = createFile(name, "", !debugMode);
        if (!result) {
            cout << "touch: невозможно создать '" << name << "': Отказано в доступе" << endl;
        }
        return result;
    }
    
    if (currentDir->findChild(name)) {
        return true;
    }
    
    if (!checkWritePermission(currentDir)) {
        cout << "touch: невозможно создать файл '" << name << "': Отказано в доступе" << endl;
        return false;
    }
    
    auto newFile = make_shared<FSNode>(name, NodeType::FILE, currentDir.get());
    currentDir->addChild(newFile, !debugMode);
    return true;
}

void FileSystem::cat(const string& name) {
    auto file = resolvePath(name);
    
    if (!file) {
        cout << "cat: " << name << ": Нет такого файла или каталога" << endl;
        return;
    }
    
    if (!file->isFile()) {
        cout << "cat: " << name << ": Это каталог" << endl;
        return;
    }
    
    if (!checkReadPermission(file)) {
        cout << "cat: " << name << ": Отказано в доступе" << endl;
        return;
    }
    
    cout << file->content.toString();
}

bool FileSystem::writeFile(const string& name, const string& content) {
    auto file = resolvePath(name);
    
    if (!file) {
        shared_ptr<FSNode> parent;
        string fileName;
        
        size_t lastSlash = name.find_last_of('/');
        if (lastSlash != string::npos) {
            string parentPath = name.substr(0, lastSlash);
            fileName = name.substr(lastSlash + 1);
            
            if (parentPath.empty()) {
                parent = root;
            } else {
                parent = resolvePath(parentPath);
            }
        } else {
            parent = currentDir;
            fileName = name;
        }
        
        if (!parent || !parent->isDirectory()) {
            cout << "Ошибка: Путь не существует" << endl;
            return false;
        }
        
        auto newFile = make_shared<FSNode>(fileName, NodeType::FILE, parent.get());
        newFile->content = Rope(content);
        parent->addChild(newFile, !debugMode);
        return true;
    }
    
    if (!file->isFile()) {
        cout << "Ошибка: " << name << " является каталогом" << endl;
        return false;
    }
    
    if (!checkWritePermission(file)) {
        cout << "Ошибка: " << name << ": Отказано в доступе" << endl;
        return false;
    }
    
    file->content = Rope(content);
    return true;
}

bool FileSystem::appendFile(const string& name, const string& content) {
    auto file = resolvePath(name);
    
    if (!file) {
        return writeFile(name, content);
    }
    
    if (!file->isFile()) {
        cout << "Ошибка: " << name << " является каталогом" << endl;
        return false;
    }
    
    if (!checkWritePermission(file)) {
        cout << "Ошибка: " << name << ": Отказано в доступе" << endl;
        return false;
    }
    
    file->content.append(content);
    return true;
}

bool FileSystem::rm(const string& name, bool recursive) {
    auto target = resolvePath(name);
    
    if (!target) {
        cout << "rm: невозможно удалить '" << name << "': Нет такого файла или каталога" << endl;
        return false;
    }
    
    if (target->isDirectory() && !recursive) {
        cout << "rm: невозможно удалить '" << name << "': Это каталог (используйте -r)" << endl;
        return false;
    }
    
    shared_ptr<FSNode> parent;
    string fileName;
    
    size_t lastSlash = name.find_last_of('/');
    if (lastSlash != string::npos) {
        string parentPath = name.substr(0, lastSlash);
        fileName = name.substr(lastSlash + 1);
        
        if (parentPath.empty()) {
            parent = root;
        } else {
            parent = resolvePath(parentPath);
        }
    } else {
        parent = currentDir;
        fileName = name;
    }
    
    if (parent) {
        if (!checkWritePermission(parent)) {
            cout << "rm: невозможно удалить '" << name << "': Отказано в доступе" << endl;
            return false;
        }
        parent->removeChild(fileName);
    }
    return true;
}

void FileSystem::ls(bool showDetails) {
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

void FileSystem::ls(const string& path, bool showDetails) {
    auto dir = resolvePath(path);
    
    if (!dir) {
        cout << "ls: невозможно получить доступ к '" << path << "': Нет такого файла или каталога" << endl;
        return;
    }
    
    if (!dir->isDirectory()) {
        cout << "ls: '" << path << "': Не является каталогом" << endl;
        return;
    }
    
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

bool FileSystem::chmod(const string& mode, const string& name) {
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

void FileSystem::findFiles(const string& name) {
    vector<string> results;
    
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

bool FileSystem::createDirectory(const string& path, bool silent) {
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

bool FileSystem::createFile(const string& path, const string& content, bool silent) {
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

bool FileSystem::writeToFile(const string& path, const string& content) {
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

string FileSystem::readFile(const string& path) {
    auto node = findNode(path);
    if (!node || !node->isFile()) {
        return "";
    }
    return node->content.toString();
}

int FileSystem::findInFile(const string& path, const string& substr) {
    auto node = resolvePath(path);
    if (!node || !node->isFile()) {
        return -1;
    }
    
    return node->content.find(substr);
}

bool FileSystem::deleteFromFile(const string& path, const string& substr) {
    auto node = resolvePath(path);
    if (!node || !node->isFile()) {
        return false;
    }
    
    if (!checkWritePermission(node)) {
        return false;
    }
    
    return node->content.deleteSubstring(substr);
}

bool FileSystem::insertInFile(const string& path, int pos, const string& text) {
    auto node = resolvePath(path);
    if (!node || !node->isFile()) {
        return false;
    }
    
    if (!checkWritePermission(node)) {
        return false;
    }
    
    node->content.insert(pos, text);
    return true;
}

void FileSystem::listDirectory(const string& path) {
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

vector<string> FileSystem::search(const string& name) {
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

bool FileSystem::remove(const string& path) {
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

bool FileSystem::setPermissions(const string& path, int owner, int group, int others) {
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

void FileSystem::visualize() {
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

void FileSystem::catFile(const string& path) {
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
        stringstream ss(content);
        string line;
        while (getline(ss, line)) {
            cout << "  " << line << endl;
        }
    }
    
    cout << "  " << string(50, '-') << endl;
    cout << "  [Размер] " << node->content.length() << " символов" << endl;
}