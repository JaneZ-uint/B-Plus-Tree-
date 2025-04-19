#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "BPT.h"
unsigned long long customHash(const char* str) {
    unsigned long long hash = 0;
    unsigned long long prime = 31; // 选择一个小的素数作为乘积因子
    for (size_t i = 0; i < strlen(str); ++i) {
        hash = hash * prime + str[i];
    }
    return hash;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    std::cout.tie(0);
    int n;
    std::cin >> n;
    std::string op,index;
    BPT<unsigned long long int,int,100,100> BPlusTree("IndexFile","LeafFile");
    int value;
    for(int i = 1;i <= n;i ++) {
        std::cin >> op;
        if(op == "insert") {
            std::cin >> index >> value;
            const char* tranfer_index = index.c_str();
            unsigned long long int HashChar = customHash(tranfer_index);
            BPlusTree.insert(HashChar,value);
        }else if(op == "delete") {
            std::cin >> index >> value;
            const char* tranfer_index = index.c_str();
            unsigned long long int HashChar = customHash(tranfer_index);
            BPlusTree.erase(HashChar,value);
        }else if(op == "find") {
            std::cin >> index;
            const char* tranfer_index = index.c_str();
            unsigned long long int HashChar = customHash(tranfer_index);
            sjtu::vector<int> tmp = BPlusTree.find(HashChar);
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