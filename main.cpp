#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "BPT.h"
int main() {
    int n;
    std::cin >> n;
    std::string op,index;
    BPT<const char*,int,100,100> BPlusTree("IndexFile","LeafFile");
    int value;
    for(int i = 1;i <= n;i ++) {
        std::cin >> op;
        if(op == "insert") {
            std::cin >> index >> value;
            const char* tranfer_index = index.c_str();
        }else if(op == "delete") {
            std::cin >> index >> value;
            const char* tranfer_index = index.c_str();
        }else if(op == "find") {
            std::cin >> index;
            const char* tranfer_index = index.c_str();
        }
    }
}