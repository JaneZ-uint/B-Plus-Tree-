#include "BPT.h"
//
// Created by JaneZ on 2025/4/15.
//
/*
 *并未将所有成员函数都写入source file，只有与文件读写相关的函数
 */
template<class KEY,class OTHER,int M = 100,int L = 100>
void BPT<KEY,OTHER,M,L>::openFile() {
    indexTree.open(indexTree_name,std::ios::binary | std::ios::in | std::ios::out);
    leaf.open(leaf_name,std::ios::binary | std::ios::in | std::ios::out);
}

template<class KEY,class OTHER,int M = 100,int L = 100>
void BPT<KEY,OTHER,M,L>::closeFile() {
    indexTree.close();
    leaf.close();
}

template<class KEY,class OTHER,int M = 100,int L = 100>
void BPT<KEY,OTHER,M,L>::readIndexNode(IndexNode &current,int pos) {
    indexTree.seekg(pos * sizeof(IndexNode) + IndexFileHeaderSize);
    indexTree.read(reinterpret_cast<char*>(&current),sizeof(IndexNode));
}

template<class KEY,class OTHER,int M = 100,int L = 100>
void BPT<KEY,OTHER,M,L>::readLeafNode(LeafNode &current,int pos) {
    leaf.seekg(pos * sizeof(LeafNode) + LeafFileHeaderSize);
    leaf.read(reinterpret_cast<char*>(&current),sizeof(LeafNode));
}

template<class KEY,class OTHER,int M = 100,int L = 100>
void BPT<KEY,OTHER,M,L>::writeIndexNode(IndexNode &current) {
    indexTree.seekp(current.pos*sizeof(IndexNode) + IndexFileHeaderSize);
    indexTree.write(reinterpret_cast<char*>(&current),sizeof(IndexNode));
}

template<class KEY,class OTHER,int M = 100,int L = 100>
void BPT<KEY,OTHER,M,L>::writeLeafNode(LeafNode &current) {
    leaf.seekp(current.pos*sizeof(LeafNode) + LeafFileHeaderSize);
    leaf.write(reinterpret_cast<char*>(&current),sizeof(LeafNode));
}

template<class KEY,class OTHER,int M = 100,int L = 100>
void BPT<KEY,OTHER,M,L>::initialize() {
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

template<class KEY,class OTHER,int M = 100,int L = 100>
void BPT<KEY,OTHER,M,L>::LoadMetaData() {
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