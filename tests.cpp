#include <gtest/gtest.h>
#include "Rope.h"
#include "AVLHTree.h"
#include "FileSystem.h"
#include <sstream>

class RopeTest : public ::testing::Test {
protected:
    Rope rope;
};

TEST_F(RopeTest, EmptyRopeLength) {
    EXPECT_EQ(rope.length(), 0);
    EXPECT_TRUE(rope.empty());
}

TEST_F(RopeTest, ConstructorWithString) {
    Rope r("Hello World");
    EXPECT_EQ(r.length(), 11);
    EXPECT_FALSE(r.empty());
    EXPECT_EQ(r.toString(), "Hello World");
}

TEST_F(RopeTest, AppendString) {
    rope.append("Hello");
    EXPECT_EQ(rope.length(), 5);
    EXPECT_EQ(rope.toString(), "Hello");
    
    rope.append(" World");
    EXPECT_EQ(rope.length(), 11);
    EXPECT_EQ(rope.toString(), "Hello World");
}

TEST_F(RopeTest, InsertAtBeginning) {
    Rope r("World");
    r.insert(0, "Hello ");
    EXPECT_EQ(r.toString(), "Hello World");
    EXPECT_EQ(r.length(), 11);
}

TEST_F(RopeTest, InsertInMiddle) {
    Rope r("Hello World");
    r.insert(5, " Beautiful");
    EXPECT_EQ(r.toString(), "Hello Beautiful World");
}

TEST_F(RopeTest, InsertAtEnd) {
    Rope r("Hello");
    r.insert(5, " World");
    EXPECT_EQ(r.toString(), "Hello World");
}

TEST_F(RopeTest, InsertIntoEmptyRope) {
    rope.insert(0, "Test");
    EXPECT_EQ(rope.toString(), "Test");
    EXPECT_EQ(rope.length(), 4);
}

TEST_F(RopeTest, FindSubstring) {
    Rope r("Hello World Hello");
    EXPECT_EQ(r.find("World"), 6);
    EXPECT_EQ(r.find("Hello"), 0);
    EXPECT_EQ(r.find("NotFound"), -1);
}

TEST_F(RopeTest, FindInEmptyRope) {
    EXPECT_EQ(rope.find("test"), -1);
}

TEST_F(RopeTest, DeleteSubstring) {
    Rope r("Hello Beautiful World");
    EXPECT_TRUE(r.deleteSubstring(" Beautiful"));
    EXPECT_EQ(r.toString(), "Hello World");
}

TEST_F(RopeTest, DeleteNonExistentSubstring) {
    Rope r("Hello World");
    EXPECT_FALSE(r.deleteSubstring("NotFound"));
    EXPECT_EQ(r.toString(), "Hello World");
}

TEST_F(RopeTest, ToString) {
    Rope r("Hello");
    std::string str = r.toString();
    EXPECT_EQ(str[0], 'H');
    EXPECT_EQ(str[4], 'o');
}

TEST_F(RopeTest, CopyConstructor) {
    Rope r1("Hello World");
    Rope r2 = r1;
    EXPECT_EQ(r2.toString(), "Hello World");
    r2.append("!");
    EXPECT_EQ(r1.toString(), "Hello World");
    EXPECT_EQ(r2.toString(), "Hello World!");
}

TEST_F(RopeTest, AssignmentOperator) {
    Rope r1("Hello");
    Rope r2("World");
    r1 = r2;
    EXPECT_EQ(r1.toString(), "World");
}

TEST_F(RopeTest, LargeStringOperations) {
    std::string large(10000, 'A');
    Rope r(large);
    EXPECT_EQ(r.length(), 10000);
    r.insert(5000, "BREAK");
    EXPECT_EQ(r.length(), 10005);
}

TEST_F(RopeTest, MultipleInserts) {
    rope.insert(0, "A");
    rope.insert(1, "B");
    rope.insert(2, "C");
    EXPECT_EQ(rope.toString(), "ABC");
}

TEST_F(RopeTest, MultipleDeletes) {
    Rope r("ABCDEFGH");
    r.deleteSubstring("C");
    r.deleteSubstring("F");
    EXPECT_EQ(r.toString(), "ABDEGH");
}

class AVLHTreeTest : public ::testing::Test {
protected:
    HTreeIndex htree;
    shared_ptr<FSNode> node1;
    shared_ptr<FSNode> node2;
    shared_ptr<FSNode> node3;

    void SetUp() override {
        node1 = make_shared<FSNode>("file1.txt", NodeType::FILE);
        node2 = make_shared<FSNode>("file2.txt", NodeType::FILE);
        node3 = make_shared<FSNode>("dir1", NodeType::DIRECTORY);
    }
};

TEST_F(AVLHTreeTest, EmptyTree) {
    EXPECT_TRUE(htree.empty());
    EXPECT_EQ(htree.size(), 0);
}

TEST_F(AVLHTreeTest, InsertSingleNode) {
    htree.insert("file1.txt", node1);
    EXPECT_FALSE(htree.empty());
    EXPECT_EQ(htree.size(), 1);
}

TEST_F(AVLHTreeTest, FindExistingNode) {
    htree.insert("file1.txt", node1);
    auto found = htree.find("file1.txt");
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->name, "file1.txt");
}

TEST_F(AVLHTreeTest, FindNonExistentNode) {
    htree.insert("file1.txt", node1);
    auto found = htree.find("notfound.txt");
    EXPECT_EQ(found, nullptr);
}

TEST_F(AVLHTreeTest, InsertMultipleNodes) {
    htree.insert("file1.txt", node1);
    htree.insert("file2.txt", node2);
    htree.insert("dir1", node3);
    EXPECT_EQ(htree.size(), 3);
}

TEST_F(AVLHTreeTest, RemoveExistingNode) {
    htree.insert("file1.txt", node1);
    htree.insert("file2.txt", node2);
    EXPECT_TRUE(htree.remove("file1.txt"));
    EXPECT_EQ(htree.size(), 1);
    EXPECT_EQ(htree.find("file1.txt"), nullptr);
}

TEST_F(AVLHTreeTest, RemoveNonExistentNode) {
    htree.insert("file1.txt", node1);
    EXPECT_FALSE(htree.remove("notfound.txt"));
    EXPECT_EQ(htree.size(), 1);
}

TEST_F(AVLHTreeTest, GetAllNodes) {
    htree.insert("file1.txt", node1);
    htree.insert("file2.txt", node2);
    htree.insert("dir1", node3);
    auto nodes = htree.getAllNodes();
    EXPECT_EQ(nodes.size(), 3);
}

TEST_F(AVLHTreeTest, HashCollisionHandling) {
    for (int i = 0; i < 100; i++) {
        std::string name = "file" + std::to_string(i) + ".txt";
        auto node = make_shared<FSNode>(name, NodeType::FILE);
        htree.insert(name, node);
    }
    EXPECT_EQ(htree.size(), 100);
    
    for (int i = 0; i < 100; i++) {
        std::string name = "file" + std::to_string(i) + ".txt";
        EXPECT_NE(htree.find(name), nullptr);
    }
}

TEST_F(AVLHTreeTest, TreeBalance) {
    for (int i = 0; i < 1000; i++) {
        std::string name = "node" + std::to_string(i);
        auto node = make_shared<FSNode>(name, NodeType::FILE);
        htree.insert(name, node);
    }
    EXPECT_EQ(htree.size(), 1000);
}

class FSNodeTest : public ::testing::Test {
protected:
    shared_ptr<FSNode> dirNode;
    shared_ptr<FSNode> fileNode;

    void SetUp() override {
        dirNode = make_shared<FSNode>("testdir", NodeType::DIRECTORY);
        fileNode = make_shared<FSNode>("testfile.txt", NodeType::FILE);
    }
};

TEST_F(FSNodeTest, IsDirectory) {
    EXPECT_TRUE(dirNode->isDirectory());
    EXPECT_FALSE(dirNode->isFile());
}

TEST_F(FSNodeTest, IsFile) {
    EXPECT_TRUE(fileNode->isFile());
    EXPECT_FALSE(fileNode->isDirectory());
}

TEST_F(FSNodeTest, AddChild) {
    auto child = make_shared<FSNode>("child.txt", NodeType::FILE);
    dirNode->addChild(child, true);
    EXPECT_EQ(dirNode->htree.size(), 1);
}

TEST_F(FSNodeTest, FindChild) {
    auto child = make_shared<FSNode>("child.txt", NodeType::FILE);
    dirNode->addChild(child, true);
    auto found = dirNode->findChild("child.txt");
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->name, "child.txt");
}

TEST_F(FSNodeTest, RemoveChild) {
    auto child = make_shared<FSNode>("child.txt", NodeType::FILE);
    dirNode->addChild(child, true);
    EXPECT_TRUE(dirNode->removeChild("child.txt"));
    EXPECT_EQ(dirNode->htree.size(), 0);
}

TEST_F(FSNodeTest, GetChildren) {
    dirNode->addChild(make_shared<FSNode>("file1.txt", NodeType::FILE), true);
    dirNode->addChild(make_shared<FSNode>("file2.txt", NodeType::FILE), true);
    auto children = dirNode->getChildren();
    EXPECT_EQ(children.size(), 2);
}

TEST_F(FSNodeTest, Permissions) {
    EXPECT_EQ(fileNode->permissions.owner, 7);
    EXPECT_EQ(fileNode->permissions.group, 5);
    EXPECT_EQ(fileNode->permissions.others, 5);
    
    fileNode->permissions.owner = 6;
    EXPECT_EQ(fileNode->permissions.owner, 6);
}

TEST_F(FSNodeTest, FileContent) {
    fileNode->content = Rope("Hello World");
    EXPECT_EQ(fileNode->content.toString(), "Hello World");
    EXPECT_EQ(fileNode->content.length(), 11);
}

class FileSystemTest : public ::testing::Test {
protected:
    FileSystem* fs;
    
    void SetUp() override {
        testing::internal::CaptureStdout();
        fs = new FileSystem();
        testing::internal::GetCapturedStdout();
    }
    
    void TearDown() override {
        delete fs;
    }
};

TEST_F(FileSystemTest, InitialState) {
    EXPECT_EQ(fs->getCurrentPath(), "/");
}

TEST_F(FileSystemTest, CreateDirectory) {
    testing::internal::CaptureStdout();
    EXPECT_TRUE(fs->mkdir("testdir"));
    testing::internal::GetCapturedStdout();
}

TEST_F(FileSystemTest, CreateFile) {
    testing::internal::CaptureStdout();
    EXPECT_TRUE(fs->touch("testfile.txt"));
    testing::internal::GetCapturedStdout();
}

TEST_F(FileSystemTest, ChangeDirectory) {
    testing::internal::CaptureStdout();
    fs->mkdir("testdir");
    EXPECT_TRUE(fs->changeDirectory("testdir"));
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(fs->getCurrentPath(), "/testdir");
}

TEST_F(FileSystemTest, ChangeDirectoryParent) {
    testing::internal::CaptureStdout();
    fs->mkdir("dir1");
    fs->changeDirectory("dir1");
    EXPECT_TRUE(fs->changeDirectory(".."));
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(fs->getCurrentPath(), "/");
}

TEST_F(FileSystemTest, WriteFile) {
    testing::internal::CaptureStdout();
    fs->touch("test.txt");
    EXPECT_TRUE(fs->writeFile("test.txt", "Hello World"));
    testing::internal::GetCapturedStdout();
}

TEST_F(FileSystemTest, AppendFile) {
    testing::internal::CaptureStdout();
    fs->touch("test.txt");
    fs->writeFile("test.txt", "Hello");
    EXPECT_TRUE(fs->appendFile("test.txt", " World"));
    testing::internal::GetCapturedStdout();
}

TEST_F(FileSystemTest, ReadFile) {
    testing::internal::CaptureStdout();
    fs->touch("test.txt");
    fs->writeFile("test.txt", "Test Content");
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(fs->readFile("test.txt"), "Test Content");
}

TEST_F(FileSystemTest, FindInFile) {
    testing::internal::CaptureStdout();
    fs->touch("test.txt");
    fs->writeFile("test.txt", "Hello World");
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(fs->findInFile("test.txt", "World"), 6);
    EXPECT_EQ(fs->findInFile("test.txt", "NotFound"), -1);
}

TEST_F(FileSystemTest, InsertInFile) {
    testing::internal::CaptureStdout();
    fs->touch("test.txt");
    fs->writeFile("test.txt", "Hello World");
    EXPECT_TRUE(fs->insertInFile("test.txt", 5, " Beautiful"));
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(fs->readFile("test.txt"), "Hello Beautiful World");
}

TEST_F(FileSystemTest, DeleteFromFile) {
    testing::internal::CaptureStdout();
    fs->touch("test.txt");
    fs->writeFile("test.txt", "Hello Beautiful World");
    EXPECT_TRUE(fs->deleteFromFile("test.txt", " Beautiful"));
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(fs->readFile("test.txt"), "Hello World");
}

TEST_F(FileSystemTest, RemoveFile) {
    testing::internal::CaptureStdout();
    fs->touch("test.txt");
    EXPECT_TRUE(fs->rm("test.txt"));
    testing::internal::GetCapturedStdout();
}

TEST_F(FileSystemTest, RemoveDirectory) {
    testing::internal::CaptureStdout();
    fs->mkdir("testdir");
    EXPECT_TRUE(fs->rm("testdir", true));
    testing::internal::GetCapturedStdout();
}

TEST_F(FileSystemTest, ChmodFile) {
    testing::internal::CaptureStdout();
    fs->touch("test.txt");
    EXPECT_TRUE(fs->chmod("644", "test.txt"));
    testing::internal::GetCapturedStdout();
}

TEST_F(FileSystemTest, PermissionCheck) {
    testing::internal::CaptureStdout();
    fs->touch("test.txt");
    fs->chmod("000", "test.txt");
    EXPECT_FALSE(fs->writeFile("test.txt", "test"));
    testing::internal::GetCapturedStdout();
}

TEST_F(FileSystemTest, AbsolutePath) {
    testing::internal::CaptureStdout();
    fs->mkdir("dir1");
    fs->mkdir("dir1/dir2");
    EXPECT_TRUE(fs->changeDirectory("/dir1/dir2"));
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(fs->getCurrentPath(), "/dir1/dir2");
}

TEST_F(FileSystemTest, RelativePath) {
    testing::internal::CaptureStdout();
    fs->mkdir("dir1");
    fs->changeDirectory("dir1");
    fs->mkdir("dir2");
    EXPECT_TRUE(fs->changeDirectory("dir2"));
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(fs->getCurrentPath(), "/dir1/dir2");
}

TEST_F(FileSystemTest, CreateFileInDirectory) {
    testing::internal::CaptureStdout();
    fs->mkdir("testdir");
    fs->changeDirectory("testdir");
    EXPECT_TRUE(fs->touch("file.txt"));
    testing::internal::GetCapturedStdout();
}

TEST_F(FileSystemTest, LargeFileOperations) {
    testing::internal::CaptureStdout();
    fs->touch("large.txt");
    std::string large(10000, 'X');
    fs->writeFile("large.txt", large);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(fs->readFile("large.txt").length(), 10000);
}

TEST_F(FileSystemTest, MultipleFilesInDirectory) {
    testing::internal::CaptureStdout();
    fs->mkdir("testdir");
    fs->changeDirectory("testdir");
    for (int i = 0; i < 100; i++) {
        fs->touch("file" + std::to_string(i) + ".txt");
    }
    testing::internal::GetCapturedStdout();
}

TEST_F(FileSystemTest, DebugMode) {
    EXPECT_FALSE(fs->isDebugMode());
    testing::internal::CaptureStdout();
    fs->toggleDebug();
    testing::internal::GetCapturedStdout();
    EXPECT_TRUE(fs->isDebugMode());
}

TEST_F(FileSystemTest, FileNotFound) {
    EXPECT_EQ(fs->readFile("notfound.txt"), "");
}

TEST_F(FileSystemTest, DirectoryNotFound) {
    testing::internal::CaptureStdout();
    EXPECT_FALSE(fs->changeDirectory("notfound"));
    testing::internal::GetCapturedStdout();
}

TEST_F(FileSystemTest, RemoveNonEmptyDirectoryWithoutRecursive) {
    testing::internal::CaptureStdout();
    fs->mkdir("dir1");
    fs->touch("dir1/file.txt");
    EXPECT_FALSE(fs->rm("dir1", false));
    testing::internal::GetCapturedStdout();
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
