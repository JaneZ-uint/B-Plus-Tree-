//
// Created by JaneZ on 2025/4/12.
//
#ifndef BPT_H
#define BPT_H
#include <fstream>
#include <string>

#include "exceptions.h"
#include "vector.h"
using namespace sjtu;
template<class KEY,class OTHER,int M = 100,int L = 100>
class BPT {
private:
    std::fstream indexTree;//索引块  前2个int大小的块存nextIndexPos和 root.pos 先后顺序就是这个
    std::fstream leaf;//数据块 前2个int大小的块存nextLeafPos和 FirstLeaf.Pos 先后顺序就是这个
    std::string indexTree_name;
    std::string leaf_name;

    struct IndexFileHeader{
        int root_pos;
        int first_free;
    };
    const int IndexFileHeaderSize = sizeof(IndexFileHeader);

    struct LeafFileHeader{
        int sum_data;
        int first_free;
    };
    const int LeafFileHeaderSize = sizeof(LeafFileHeader);

    struct IndexNode {
        bool is_leaf = false;
        int keyNum;//关键字数量
        int childNum;//孩子数量
        int pos;//当前结点在disk上的位置
        KEY Key[M - 1];
        int ChildPointer[M];//磁盘位置

        IndexNode(bool whether,int k = 0,int c = 0,int p = 1):is_leaf(whether),keyNum(k),childNum(c),pos(p){}
    };

    struct LeafNode {
        bool is_leaf = true;
        int pos;//当前结点在disk上的位置
        int num;//存储键值对数量
        std::pair<KEY,OTHER> Info[L];//键值对信息
        int next;//下一个LeafNode位置

        LeafNode(bool whether = true,int p = 1,int n = 0,int nxt):is_leaf(whether),pos(p),num(n),next(nxt) {}
    };

    IndexNode root;
    LeafNode Data;
    int totalNum = 0;
    int nextIndexNodePos;
    int nextLeafNodePos;

    void openFile() {
        indexTree.open(indexTree_name,std::ios::binary | std::ios::in | std::ios::out);
        leaf.open(leaf_name,std::ios::binary | std::ios::in | std::ios::out);
    }
    void closeFile() {
        indexTree.close();
        leaf.close();
    }
    void writeInfoIndex(IndexNode &written_index,int position) {
        indexTree.seekp(IndexFileHeaderSize + position * sizeof(IndexNode));
        indexTree.write(reinterpret_cast<char*>(&written_index),sizeof(IndexNode));
    }
    void writeInfoLeaf(LeafNode &written_leaf,int position) {
        leaf.seekp(LeafFileHeaderSize + position * sizeof(LeafNode));
        leaf.write(reinterpret_cast<char*>(&written_leaf),sizeof(LeafNode));
    }
    void initialize() {
        root.is_leaf = true;
        root.keyNum = 0;
        root.childNum = 1;
        root.pos = 1;
        root.ChildPointer[0] = 1;
        totalNum = 0;
        nextIndexNodePos = 1;
        nextLeafNodePos = 1;
        LeafNode FirstLeaf(true,1,0,0);
        writeInfoLeaf(FirstLeaf,1);
    }
    void LoadMetaData() {
        indexTree.seekg(0);
        leaf.seekg(0);
        IndexFileHeader tmp1;
        LeafFileHeader tmp2;
        indexTree.read(reinterpret_cast<char*>(&tmp1),IndexFileHeaderSize);
        nextIndexNodePos = tmp1.first_free;
        leaf.read(reinterpret_cast<char*>(&tmp2),LeafFileHeaderSize);
        indexTree.seekg(IndexFileHeaderSize + tmp1.root_pos * sizeof(IndexNode));
        indexTree.read(reinterpret_cast<char*>(&root),sizeof(IndexNode));//读根结点
        totalNum = tmp2.sum_data;
        nextLeafNodePos = tmp2.first_free;
    }
public:
    BPT(const std::string &s1,const std::string &s2) {
        indexTree_name = s1;
        leaf_name = s2;
        openFile();
        if(!indexTree && !leaf) {
            indexTree.open(indexTree_name,std::ios::binary | std::ios::out);
            leaf.open(leaf_name,std::ios::binary | std::ios::out);
            initialize();
            closeFile();
            openFile();
        }else {
            LoadMetaData();
        }
        closeFile();
    }
    ~BPT() {
        try {
            if(indexTree.is_open()) {
                indexTree.close();
            }
            if(leaf.is_open()) {
                leaf.close();
            }
        }catch(...) {
            std::cerr << "Failed to close files.";
        }
    }
    sjtu::vector<int> find(KEY k) {

    }
};
#endif //BPT_H