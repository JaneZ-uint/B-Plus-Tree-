#include <iostream>
#include <fstream>
#include <string>
#include "BPT.h"
int main() {
    int n;
    std::cin >> n;
    std::string op;
    int index,value;
    for(int i = 1;i <= n;i ++) {
        std::cin >> op;
        if(op == "insert") {
            std::cin >> index >> value;
        }else if(op == "delete") {
            std::cin >> index >> value;
        }else if(op == "find") {
            std::cin >> index;
        }
    }
}