//
// Created by JaneZ on 2025/4/12.
//
#ifndef BPT_H
#define BPT_H
#include <emmintrin.h>
#include <fstream>
#include <string>

#include "exceptions.h"
#include "vector.h"
using namespace sjtu;
template<class KEY,class OTHER,int M = 100,int L = 100>  //我需要M L是偶数
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
        KO(const KEY &_k,const OTHER &_other):k(_k),other(_other){}
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
        bool operator==(const KO &a) const {
            if(k == a.k && other == a.other) {
                return true;
            }
            return false;
        }
    };

    struct IndexNode {
        bool is_leaf = false;
        int keyNum;//关键字数量
        int pos;//当前结点在disk上的位置
        KO Key[M - 1];//注意：这里必须存键值对，因为可能存在一个键值对应多个value的情况
        int ChildPointer[M];//磁盘位置

        IndexNode(bool whether,int k = 0,int p = 1):is_leaf(whether),keyNum(k),pos(p){}
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

    void openFile();
    void closeFile();
    void readIndexNode(IndexNode &current,int pos);
    void readLeafNode(LeafNode &current,int pos);
    void writeIndexNode(IndexNode &current);
    void writeLeafNode(LeafNode &current);
    void initialize();
    void LoadMetaData();

    int searchIndexToFind(KEY &k,IndexNode &current) {  //找到最后一个以小于k的键值开始的索引块（如果没有小于的则返回第0个块
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
    int searchLeafToFind(KEY &k,LeafNode &current) {
        int l = 0,r = current.num - 1;
        int ans = 0;
        while(l <= r) {
            int mid = (l + r)/2;
            if(current.Info[mid].k < k) {
                l = mid + 1;
            }else if(current.Info[mid].k == k) {
                ans = mid;
                r = mid - 1;
            }else {
                r = mid - 1;
            }
        }
        return ans;
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
    void splitRoot(IndexNode &current) {  //分裂根节点稍有不同
        IndexNode secondRoot;
        int mid = M / 2;
        /* M = 6
         *     3
         *    / \
         *  1 2 4 5
         */
        for(int i = 0;i < mid;i ++) {
            secondRoot.ChildPointer[i] = current.ChildPointer[i + mid];
        }
        for(int i = 0;i < mid - 1;i ++) {
            secondRoot.Key[i] = current.Key[i + mid];
        }
        secondRoot.pos = ++nextIndexNodePos;
        secondRoot.is_leaf = current.is_leaf;
        secondRoot.keyNum = mid - 1;
        current.keyNum = mid - 1;
        writeIndexNode(secondRoot);
        writeIndexNode(current);
        IndexNode newRoot;
        newRoot.ChildPointer[0] = current.pos;
        newRoot.ChildPointer[1] = secondRoot.pos;
        newRoot.Key = current[mid -1];
        newRoot.pos = ++nextIndexNodePos;
        newRoot.keyNum = 1;
        newRoot.is_leaf = false;
        root = newRoot;
        writeIndexNode(newRoot);
    }
    bool splitNormalIndexNode(IndexNode &current,IndexNode &above,int idx) {
        /*
         *   1  2 3 4 5
         *  / \  \ \ \ \
         *       3
         *      / \
         *   1 2    4 5
         *  / \ \  / \ \
         */
        IndexNode secondIndexNode;
        int mid = M / 2;
        for(int i = 0;i < mid;i ++) {
            secondIndexNode.ChildPointer[i] = current.ChildPointer[i + mid];
        }
        for(int i = 0;i < mid - 1;i ++) {
            secondIndexNode.Key[i] = current.Key[i + mid];
        }
        secondIndexNode.is_leaf = current.is_leaf;
        secondIndexNode.pos = ++nextIndexNodePos;
        secondIndexNode.keyNum = mid - 1;
        current.keyNum = mid - 1;
        writeIndexNode(secondIndexNode);
        writeIndexNode(current);
        //TODO
        for(int i = above.keyNum + 1;i > idx + 1;i --) {
            above.ChildPointer[i] = above.ChildPointer[i - 1];
        }
        above.ChildPointer[idx + 1] = secondIndexNode.pos;
        for(int i = above.keyNum;i > idx;i --) {
            above.Key[i] = above.Key[i - 1];
        }
        above.Key[idx] = current.Key[mid - 1];
        above.keyNum ++;
        if(above.keyNum == M - 1) {
            return true;
        }else {
            writeIndexNode(above);
            return false;
        }
    }
    bool splitLeaf(LeafNode &current,IndexNode &above,int idx) { //分裂叶结点
        LeafNode newLeaf;
        int mid = L/2;
        for(int i = 0;i < mid;i ++) {
            newLeaf.Info[i] = current.Info[i + mid];
        }
        newLeaf.num = mid;
        current.num = mid;
        newLeaf.pos = ++ nextLeafNodePos;
        //TODO Wait to be checked
        for(int i = above.keyNum + 1;i > idx + 1;i --) {
            above.ChildPointer[i] = above.ChildPointer[i - 1];
        }
        above.ChildPointer[idx + 1] = newLeaf.pos;
        for(int i = above.keyNum;i > idx;i --) {
            above.Key[i] = above.Key[i - 1];
        }
        above.Key[idx] = newLeaf.Info[0];
        newLeaf.next = current.next;
        current.next = newLeaf.pos;
        writeLeafNode(newLeaf);
        writeLeafNode(current);
        above.keyNum ++;
        if(above.keyNum == M - 1) {  //需要对父节点进行分裂
            return true;
        }
        return false;
    }
    bool Insert(IndexNode &current,KO &tmp) {
        /*
         * 该函数如果需要继续分裂块，则return true ，不然return false
         * 递归调用
         * 先从indexnode下面就是叶结点开始考虑
         */
        if(current.is_leaf) {
            int idx = searchIndexForInsert(tmp,current);
            LeafNode search;
            readLeafNode(search,current.ChildPointer[idx]);
            idx = searchLeafForInsert(tmp,search);//插入位置的坐标
            if(idx == -1) {
                return false;//插入节点原本就存在
            }
            totalNum ++;
            for(int i = search.Info[search.num - 1];i > idx;i --) {
                search.Info[i] = search.Info[i - 1];
            }
            search.num ++;
            if(search.num == L) { //需要裂块
                if(splitLeaf(search,current,idx)) { //需要对索引块进行分裂
                    return true;
                }else {
                    return false;
                }
            }else {
                writeLeafNode(search);
                return false;
            }
        }else {
            IndexNode child;
            int idx = searchIndexForInsert(tmp,current);
            readIndexNode(child,idx);
            if(Insert(child,tmp)) {  //需要对普通的IndexNode进行分裂
                if(splitNormalIndexNode(child,current,idx)) {
                    return true;
                }else {
                    return false;
                }
            }else {
                return false;
            }
        }
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
        openFile();
        sjtu::vector<OTHER> results;
        if(root.keyNum == 0) {
            return results;
        }
        IndexNode current = root;
        LeafNode target;
        int idx;
        while(!current.is_leaf) {
            idx = searchIndexToFind(k,current);
            readIndexNode(current,idx);
        }
        idx = searchIndexToFind(k,current);
        readLeafNode(target,idx);
        idx = searchLeafToFind(k,target);
        for(int i = idx;i < target.num;i ++) {
            results.push_back(target.Info[i].other);
        }
        bool flag = true;
        while(target.next != 0 && flag) {
            readLeafNode(target,target.next);
            for(int i = 0;i < target.num;i ++) {
                if(target.Info[i].k == k) {
                    results.push_back(target.Info[i].other);
                }else {
                    flag = false;
                    break;
                }
            }
        }
        closeFile();
        return results;
    }
    void insert(const KEY &k,const OTHER &other) {
        openFile();
        KO tmp(k,other);
        if(Insert(root,tmp)) {
            splitRoot();
        }
        //TODO
        closeFile();
    }
    void erase(const KEY &k,const OTHER &other) {

    }
};
#endif //BPT_H