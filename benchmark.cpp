#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>
#include "AVLHTree.h"

using namespace std;
using namespace chrono;

string randomString(int index) {
    static random_device rd;
    static mt19937 gen(42);
    static uniform_int_distribution<> dis(0, 61);
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    string result = "rnd_" + to_string(index) + "_";
    int extraLength = 10 + (index % 10);
    for (int i = 0; i < extraLength; ++i) {
        result += alphanum[dis(gen)];
    }
    return result;
}

string collisionString(int index) {
    return "file_" + string(5 - to_string(index).length(), '0') + to_string(index) + ".txt";
}

string optimalString(int index) {
    string prefix = "opt_";
    int val = index * 31 + 17;
    return prefix + to_string(val) + "_file.txt";
}

void benchmarkInsert(const string& outputFile) {
    vector<int> sizes = {10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000};
    ofstream out(outputFile);
    out << "size,best,average,worst\n";
    
    cout << "Benchmarking INSERT operation...\n";
    
    for (int size : sizes) {
        cout << "  Size: " << size << "..." << flush;
        
        HTreeIndex treeBest;
        auto startBest = high_resolution_clock::now();
        for (int i = 0; i < size; i++) {
            string name = optimalString(i);
            auto node = make_shared<FSNode>(name, NodeType::FILE);
            treeBest.insert(name, node);
        }
        auto endBest = high_resolution_clock::now();
        double timeBest = duration_cast<nanoseconds>(endBest - startBest).count() / (double)size;
        
        HTreeIndex treeAvg;
        vector<string> randomNames;
        for (int i = 0; i < size; i++) {
            randomNames.push_back(randomString(i));
        }
        shuffle(randomNames.begin(), randomNames.end(), mt19937(42));
        
        auto startAvg = high_resolution_clock::now();
        for (const auto& name : randomNames) {
            auto node = make_shared<FSNode>(name, NodeType::FILE);
            treeAvg.insert(name, node);
        }
        auto endAvg = high_resolution_clock::now();
        double timeAvg = duration_cast<nanoseconds>(endAvg - startAvg).count() / (double)size;
        
        HTreeIndex treeWorst;
        vector<string> collisionNames;
        for (int i = 0; i < size; i++) {
            collisionNames.push_back(collisionString(i));
        }
        sort(collisionNames.begin(), collisionNames.end());
        
        auto startWorst = high_resolution_clock::now();
        for (const auto& name : collisionNames) {
            auto node = make_shared<FSNode>(name, NodeType::FILE);
            treeWorst.insert(name, node);
        }
        auto endWorst = high_resolution_clock::now();
        double timeWorst = duration_cast<nanoseconds>(endWorst - startWorst).count() / (double)size;
        
        out << size << "," << timeBest << "," << timeAvg << "," << timeWorst << "\n";
        cout << " Done\n";
    }
    
    out.close();
    cout << "Insert benchmark saved to " << outputFile << "\n\n";
}

void benchmarkFind(const string& outputFile) {
    vector<int> sizes = {10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000};
    ofstream out(outputFile);
    out << "size,best,average,worst\n";
    
    cout << "Benchmarking FIND operation...\n";
    
    for (int size : sizes) {
        cout << "  Size: " << size << "..." << flush;
        
        HTreeIndex treeBest;
        vector<string> namesBest;
        for (int i = 0; i < size; i++) {
            string name = optimalString(i);
            namesBest.push_back(name);
            auto node = make_shared<FSNode>(name, NodeType::FILE);
            treeBest.insert(name, node);
        }
        
        auto startBest = high_resolution_clock::now();
        for (int i = 0; i < 100; i++) {
            treeBest.find(namesBest[0]);
        }
        auto endBest = high_resolution_clock::now();
        double timeBest = duration_cast<nanoseconds>(endBest - startBest).count() / 100.0;
        
        HTreeIndex treeAvg;
        vector<string> namesAvg;
        for (int i = 0; i < size; i++) {
            string name = randomString(i);
            namesAvg.push_back(name);
            auto node = make_shared<FSNode>(name, NodeType::FILE);
            treeAvg.insert(name, node);
        }
        
        auto startAvg = high_resolution_clock::now();
        for (int i = 0; i < 100; i++) {
            treeAvg.find(namesAvg[size / 2]);
        }
        auto endAvg = high_resolution_clock::now();
        double timeAvg = duration_cast<nanoseconds>(endAvg - startAvg).count() / 100.0;
        
        HTreeIndex treeWorst;
        vector<string> namesWorst;
        for (int i = 0; i < size; i++) {
            string name = collisionString(i);
            namesWorst.push_back(name);
            auto node = make_shared<FSNode>(name, NodeType::FILE);
            treeWorst.insert(name, node);
        }
        
        auto startWorst = high_resolution_clock::now();
        for (int i = 0; i < 100; i++) {
            treeWorst.find(namesWorst[size - 1]);
        }
        auto endWorst = high_resolution_clock::now();
        double timeWorst = duration_cast<nanoseconds>(endWorst - startWorst).count() / 100.0;
        
        out << size << "," << timeBest << "," << timeAvg << "," << timeWorst << "\n";
        cout << " Done\n";
    }
    
    out.close();
    cout << "Find benchmark saved to " << outputFile << "\n\n";
}

void benchmarkRemove(const string& outputFile) {
    vector<int> sizes = {10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000};
    ofstream out(outputFile);
    out << "size,best,average,worst\n";
    
    cout << "Benchmarking REMOVE operation...\n";
    
    for (int size : sizes) {
        cout << "  Size: " << size << "..." << flush;
        
        HTreeIndex treeBest;
        vector<string> namesBest;
        for (int i = 0; i < size; i++) {
            string name = optimalString(i);
            namesBest.push_back(name);
            auto node = make_shared<FSNode>(name, NodeType::FILE);
            treeBest.insert(name, node);
        }
        
        int removeCount = max(1, size / 10);
        auto startBest = high_resolution_clock::now();
        for (int i = 0; i < removeCount; i++) {
            treeBest.remove(namesBest[size - 1 - i]);
        }
        auto endBest = high_resolution_clock::now();
        double timeBest = duration_cast<nanoseconds>(endBest - startBest).count() / (double)removeCount;
        
        HTreeIndex treeAvg;
        vector<string> namesAvg;
        for (int i = 0; i < size; i++) {
            string name = randomString(i);
            namesAvg.push_back(name);
            auto node = make_shared<FSNode>(name, NodeType::FILE);
            treeAvg.insert(name, node);
        }
        
        vector<int> indices;
        for (int i = 0; i < size; i++) indices.push_back(i);
        shuffle(indices.begin(), indices.end(), mt19937(42));
        
        auto startAvg = high_resolution_clock::now();
        for (int i = 0; i < removeCount; i++) {
            treeAvg.remove(namesAvg[indices[i]]);
        }
        auto endAvg = high_resolution_clock::now();
        double timeAvg = duration_cast<nanoseconds>(endAvg - startAvg).count() / (double)removeCount;
        
        HTreeIndex treeWorst;
        vector<string> namesWorst;
        for (int i = 0; i < size; i++) {
            string name = collisionString(i);
            namesWorst.push_back(name);
            auto node = make_shared<FSNode>(name, NodeType::FILE);
            treeWorst.insert(name, node);
        }
        
        auto startWorst = high_resolution_clock::now();
        for (int i = 0; i < removeCount; i++) {
            treeWorst.remove(namesWorst[i]);
        }
        auto endWorst = high_resolution_clock::now();
        double timeWorst = duration_cast<nanoseconds>(endWorst - startWorst).count() / (double)removeCount;
        
        out << size << "," << timeBest << "," << timeAvg << "," << timeWorst << "\n";
        cout << " Done\n";
    }
    
    out.close();
    cout << "Remove benchmark saved to " << outputFile << "\n\n";
}

int main() {
    srand(time(nullptr));
    
    cout << "=== AVL H-Tree Performance Benchmark ===\n\n";
    
    benchmarkInsert("benchmark_insert.csv");
    benchmarkFind("benchmark_find.csv");
    benchmarkRemove("benchmark_remove.csv");
    
    cout << "All benchmarks completed!\n";
    cout << "Run 'python3 plot_benchmarks.py' to generate graphs.\n";
    
    return 0;
}
