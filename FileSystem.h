#pragma once

#include "AVLHTree.h"
#include "Rope.h"
#include <string>
#include <vector>
#include <memory>

using namespace std;

class FSNode;

class FileSystem {
    shared_ptr<FSNode> root;
    shared_ptr<FSNode> currentDir;
    bool debugMode;

    vector<string> splitPath(const string& path) const;
    shared_ptr<FSNode> findNode(const string& path);
    shared_ptr<FSNode> resolvePath(const string& path);
    string getPathToNode(shared_ptr<FSNode> node);
    bool checkReadPermission(shared_ptr<FSNode> node);
    bool checkWritePermission(shared_ptr<FSNode> node);
    bool checkExecutePermission(shared_ptr<FSNode> node);
    void searchRecursive(shared_ptr<FSNode> node, const string& name, 
                        const string& currentPath, vector<string>& results);
    void visualizeTree(shared_ptr<FSNode> node, const string& prefix, bool isLast);

public:
    FileSystem();
    void toggleDebug();
    bool isDebugMode() const;
    string getCurrentPath() const;
    bool changeDirectory(const string& path);
    bool mkdir(const string& name);
    bool touch(const string& name);
    void cat(const string& name);
    bool writeFile(const string& name, const string& content);
    bool appendFile(const string& name, const string& content);
    bool rm(const string& name, bool recursive = false);
    void ls(bool showDetails = false);
    void ls(const string& path, bool showDetails = false);
    bool chmod(const string& mode, const string& name);
    void findFiles(const string& name);
    bool createDirectory(const string& path, bool silent = false);
    bool createFile(const string& path, const string& content = "", bool silent = false);
    bool writeToFile(const string& path, const string& content);
    string readFile(const string& path);
    int findInFile(const string& path, const string& substr);
    bool deleteFromFile(const string& path, const string& substr);
    bool insertInFile(const string& path, int pos, const string& text);
    void listDirectory(const string& path);
    vector<string> search(const string& name);
    bool remove(const string& path);
    bool setPermissions(const string& path, int owner, int group, int others);
    void visualize();
    void catFile(const string& path);
};