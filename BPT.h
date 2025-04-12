//
// Created by JaneZ on 2025/4/12.
//
#ifndef BPT_H
#define BPT_H
#include <fstream>
#include <string>
template<class KEY,class OTHER,int M = 100,int L = 100>
class BPT {
private:
    std::fstream indexTree;//索引块
    std::fstream leaf;//数据块
    std::string indexTree_name;
    std::string leaf_name;

    struct IndexNode {
        bool is_leaf = false;
        int keyNum;//关键字数量
        int childNum;//孩子数量
        int pos;//当前结点在disk上的位置
        KEY Key[M - 1];
        int ChildPointer[M];//磁盘位置
    };

    struct LeafNode {
        bool is_leaf = true;
        int pos;//当前结点在disk上的位置
        int num;//存储键值对数量
        std::pair<KEY,OTHER> Info[L];//键值对信息
        int next;//下一个LeafNode位置
    };

    IndexNode root;
    LeafNode Data;
};
#endif //BPT_H