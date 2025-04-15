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

    struct KO {
        KEY k;
        OTHER other;
        KO(KEY _k):k(_k),OTHER(std::numeric_limits<OTHER>::min()){}
        KO(KEY _k,OTHER _other):k(_k),other(_other){}
        bool operator<(const KO a) { //重载<运算符
            if(this->k < a.k) {
                return true;
            }else if(this -> k > a.k) {
                return false;
            }else {
                if(this->other < a.other) {
                    return true;
                }else {
                    return false;
                }
            }
        }
    };

    struct IndexNode {
        bool is_leaf = false;
        int keyNum;//关键字数量
        int childNum;//孩子数量
        int pos;//当前结点在disk上的位置
        KO Key[M - 1];//注意：这里必须存键值对，因为可能存在一个键值对应多个value的情况
        int ChildPointer[M];//磁盘位置

        IndexNode(bool whether,int k = 0,int c = 0,int p = 1):is_leaf(whether),keyNum(k),childNum(c),pos(p){}
    };

    struct LeafNode {
        bool is_leaf = true;
        int pos;//当前结点在disk上的位置
        int num;//存储键值对数量
        KO Info[L];//键值对信息
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
    void readIndexNode(IndexNode &current,int pos) {
        indexTree.seekg(pos * sizeof(IndexNode) + IndexFileHeaderSize);
        indexTree.read(reinterpret_cast<char*>(&current),sizeof(IndexNode));
    }
    void readLeafNode(LeafNode &current,int pos) {
        leaf.seekg(pos * sizeof(LeafNode) + LeafFileHeaderSize);
        leaf.read(reinterpret_cast<char*>(&current),sizeof(LeafNode));
    }
    void writeIndexNode(IndexNode &current) {
        indexTree.seekp(current.pos*sizeof(IndexNode) + IndexFileHeaderSize);
        indexTree.write(reinterpret_cast<char*>(&current),sizeof(IndexNode));
    }
    void writeLeafNode(LeafNode &current) {
        leaf.seekp(current.pos*sizeof(LeafNode) + LeafFileHeaderSize);
        leaf.write(reinterpret_cast<char*>(&current),sizeof(LeafNode));
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
    int searchIndexToFindMin(KEY &k,IndexNode &current) {  //找到可能出现的最小块和最大块 TODO 很好这个要重写
        int l = 0,r = current.keyNum - 1;
        int MinBlock = -1;
        while(l <= r) {
            int mid = (l + r)/2;
            if(current.Key[mid].k < k) {
                MinBlock = mid;
                l = mid + 1;
            }else {
                r = mid - 1;
            }
        }
        return MinBlock + 1;
    }
    int searchIndexToFindMax(KEY &k,IndexNode &current) {
        int l = 0,r = current.keyNum - 1;
        int MaxBlock = current.keyNum - 1;
        while(l <= r) {
            int mid = (l + r)/2;
            if(current.Key[mid].k > k) {
                MaxBlock = mid;
                r = mid - 1;
            }else {
                l = mid + 1;
            }
        }
        return MaxBlock;
    }
    int searchIndexForInsert(KO &k,IndexNode &current) { //查询当前索引块中相应的孩子指针 二分  对于插入删除操作
        int l = 0,r = current.keyNum - 1;
        int ans = -1;
        while(l <= r) {
            int mid = (l + r)/2;
            if(current.Key[mid] < k) {
                ans = mid;
                l = mid + 1;
            }else if(current.Key[mid] == k){
                ans = mid;
                break;
            }else {
                r = mid - 1;
            }
        }
        return ans + 1;//在childPointer数组中的下标
    }
    int searchLeafForInsert(KO &k,LeafNode &current) { ////查询当前叶子块中KO对的位置 二分  对于插入操作
        int l = 0,r = current.num - 1;
        int ans = -1;
        while(l <= r) {
            int mid = (l + r)/2;
            if(current.num[mid] < k) {
                ans = mid;
                l = mid + 1;
            }else if(current.num[mid] == k) {
                ans = -2;
                break;
            }else {
                r = mid - 1;
            }
        }
        return ans + 1;//在current中的插入位置 返回-1则表示值已经存在
    }
    int searchLeafForErase(KO &k,LeafNode &current) { //查询当前叶子块中KO对的位置 二分  对于删除操作
        int l = 0,r = current.num - 1;
        int ans = -1;
        while(l <= r) {
            int mid = (l + r)/2;
            if(current.Info[mid] < k) {
                l = mid + 1;
            }else if(current.Info[mid] == k) {
                ans = k;
                break;
            }else {
                r = mid -1;
            }
        }
        return ans;//返回-1则表示键值对不存在
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
        indexTree.seekp(0);
        leaf.seekp(0);
        IndexFileHeader tmp1(root.pos,nextIndexNodePos);
        indexTree.write(reinterpret_cast<char*>(&tmp1),IndexFileHeaderSize);
        LeafFileHeader tmp2(totalNum,nextLeafNodePos);
        leaf.write(reinterpret_cast<char*>(&tmp2),LeafFileHeaderSize);
        writeIndexNode(root);
        closeFile();
    }
    sjtu::vector<OTHER> find(const KEY &k) {
        sjtu::vector<OTHER> results;
        if(root.keyNum == 0) {
            return results;
        }
        IndexNode current = root;
        while(!current.is_leaf) {
            int idx = searchIndexToFindMin(k,current);

        }
    }
    void insert(const KEY &k,const OTHER &other) {

    }
    void erase(const KEY &k,const OTHER &other) {

    }
};
#endif //BPT_H