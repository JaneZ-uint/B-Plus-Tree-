#include <iostream>
#include <fstream>
#include <string>
#include "BPT.h"
unsigned long long customHash(const std::string &s) {
    unsigned long long hash = 0;
    unsigned long long prime = 31; // 选择一个小的素数作为乘积因子
    for (size_t i = 0; i < s.size(); ++i) {
        hash = hash * prime + s[i];
    }
    return hash;
}
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    int n;
    std::cin >> n;
    std::string op,index;
    BPT<unsigned long long int,int> BPlusTree("IndexFile","LeafFile");
    int value;
    for(int i = 1;i <= n;i ++) {
        std::cin >> op;
        if(op == "insert") {
            std::cin >> index >> value;
            BPlusTree.insert(customHash(index),value);
        }else if(op == "delete") {
            std::cin >> index >> value;
            BPlusTree.erase(customHash(index),value);
        }else if(op == "find") {
            std::cin >> index;
            sjtu::vector<int> tmp = BPlusTree.find(customHash(index));
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
