#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "BPT.h"
#include <cstdint>
// 针对短字符串（≤64B）的高效哈希函数
uint64_t customHash(const char* str) {
    uint64_t hash = 0;
    const uint64_t prime = 11400714819323198485ULL; // 2^64 / φ (黄金比例素数)
    while (*str != '\0') {
        hash = hash * prime + static_cast<uint8_t>(*str); // 显式转换为无符号
        ++str;
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
