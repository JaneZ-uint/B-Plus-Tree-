#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "BPT.h"
int main() {
    int n;
    std::cin >> n;
    std::string op,index;
    BPT<const char*,int,100,100> BPlusTree("IndexFile.txt","LeafFile.txt");
    int value;
    for(int i = 1;i <= n;i ++) {
        std::cin >> op;
        if(op == "insert") {
            std::cin >> index >> value;
            const char* tranfer_index = index.c_str();
            BPlusTree.insert(tranfer_index,value);
        }else if(op == "delete") {
            std::cin >> index >> value;
            const char* tranfer_index = index.c_str();
            BPlusTree.erase(tranfer_index,value);
        }else if(op == "find") {
            std::cin >> index;
            const char* tranfer_index = index.c_str();
            sjtu::vector<int> tmp = BPlusTree.find(tranfer_index);
            if(tmp.empty()) {
                std::cout << "null" << '\n';
            }else {
                for(int j = 0;j < tmp.size();j ++) {
                    std::cout << tmp[j] << " ";
                }
                std::cout << '\n';
            }
        }
    }
}