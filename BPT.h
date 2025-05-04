//
// Created by JaneZ on 2025/4/12.
//
#ifndef BPT_H
#define BPT_H
#include <fstream>
#include <string>
#include "vector.h"
#include "LRUCache.h"

using namespace sjtu;
template<class KEY,class OTHER,int M = 50,int L = 50> //我需要M L是偶数
class BPT {
private:
    std::fstream indexTree;//索引块  前2个int大小的块存nextIndexPos和 root.pos 先后顺序就是这个
    std::fstream leaf;//数据块 前2个int大小的块存nextLeafPos和 FirstLeaf.Pos 先后顺序就是这个
    std::string indexTree_name;
    std::string leaf_name;

    sjtu::vector<int> freeIndexPos;
    sjtu::vector<int> freeLeafPos;

    int allocateIndexPos(){
        if(!freeIndexPos.empty()) {
            int pos = freeIndexPos.back();
            freeIndexPos.pop_back();
            return pos;
        }
        return ++nextIndexNodePos;
    }
    int allocateLeadfPos() {
        if(!freeLeafPos.empty()) {
            int pos = freeLeafPos.back();
            freeLeafPos.pop_back();
            return pos;
        }
        return ++nextLeafNodePos;
    }

    void releaseIndexPos(int pos) {
        freeIndexPos.push_back(pos);
    }
    void releaseLeafPos(int pos) {
        freeLeafPos.push_back(pos);
    }

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
        KO() : k(), other() {}  // 添加默认构造
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
        KO Key[M + 1];//注意：这里必须存键值对，因为可能存在一个键值对应多个value的情况
        int ChildPointer[M + 1];//磁盘位置

    };

    struct LeafNode {
        bool is_leaf = true;
        int pos;//当前结点在disk上的位置
        int num;//存储键值对数量
        KO Info[L];//键值对信息
        int next;//下一个LeafNode位置

    };

    IndexNode root;
    int totalNum = 0;
    int nextIndexNodePos;
    int nextLeafNodePos;

    //添加LRU Cache
    LRUCache<int,IndexNode> IndexCache;
    LRUCache<int,LeafNode> LeafCache;

    void openFile() {
        indexTree.open(indexTree_name,std::ios::binary | std::ios::in | std::ios::out);
        leaf.open(leaf_name,std::ios::binary | std::ios::in | std::ios::out);
    }
    void closeFile() {
        indexTree.close();
        leaf.close();
    }

    void readIndexNode(IndexNode &current,int pos) {
        if(IndexCache.get(pos,current)) {
            return;
        }
        indexTree.seekg(pos * sizeof(IndexNode) + IndexFileHeaderSize);
        indexTree.read(reinterpret_cast<char*>(&current),sizeof(IndexNode));
        IndexCache.put(pos,current);
    }

    void readLeafNode(LeafNode &current,int pos) {
        if(LeafCache.get(pos,current)) {
            return;
        }
        leaf.seekg(pos * sizeof(LeafNode) + LeafFileHeaderSize);
        leaf.read(reinterpret_cast<char*>(&current),sizeof(LeafNode));
        LeafCache.put(pos,current);
    }

    void writeIndexNode(IndexNode &current) {
        IndexCache.put(current.pos,current);
        indexTree.seekp(current.pos*sizeof(IndexNode) + IndexFileHeaderSize);
        indexTree.write(reinterpret_cast<char*>(&current),sizeof(IndexNode));
    }

    void writeLeafNode(LeafNode &current) {
        LeafCache.put(current.pos,current);
        leaf.seekp(current.pos*sizeof(LeafNode) + LeafFileHeaderSize);
        leaf.write(reinterpret_cast<char*>(&current),sizeof(LeafNode));
    }

    void initialize() {
        freeIndexPos.clear();
        freeLeafPos.clear();

        root.is_leaf = true;
        root.keyNum = 0;
        root.pos = 1;
        root.ChildPointer[0] = 1;
        totalNum = 0;
        nextIndexNodePos = 1;
        nextLeafNodePos = 1;
        LeafNode FirstLeaf;
        FirstLeaf.is_leaf = true;
        FirstLeaf.next = 0;
        FirstLeaf.num = 0;
        FirstLeaf.pos = 1;
        writeLeafNode(FirstLeaf);
    }

    void LoadMetaData() {
        indexTree.seekg(0);
        leaf.seekg(0);
        IndexFileHeader tmp1;
        LeafFileHeader tmp2;
        indexTree.read(reinterpret_cast<char*>(&tmp1),IndexFileHeaderSize);
        nextIndexNodePos = tmp1.first_free;
        leaf.read(reinterpret_cast<char*>(&tmp2),LeafFileHeaderSize);
        if (!IndexCache.get(tmp1.root_pos, root)) {
            // 缓存未命中，从磁盘读取
            indexTree.seekg(IndexFileHeaderSize + tmp1.root_pos * sizeof(IndexNode));
            indexTree.read(reinterpret_cast<char*>(&root), sizeof(IndexNode));
            // 确保放入缓存
            IndexCache.put(root.pos, root);
        }
        totalNum = tmp2.sum_data;
        nextLeafNodePos = tmp2.first_free;
    }

    void UpdateMetaData() {
        IndexFileHeader tmp1(root.pos,nextIndexNodePos);
        LeafFileHeader tmp2(totalNum,nextLeafNodePos);
        indexTree.seekp(0);
        indexTree.write(reinterpret_cast<char*>(&tmp1),IndexFileHeaderSize);
        writeIndexNode(root);
        leaf.seekp(0);
        leaf.write(reinterpret_cast<char*>(&tmp2),LeafFileHeaderSize);
    }

    int searchIndexToFind(const KEY &k,IndexNode &current) {  //找到最后一个以小于k的键值开始的索引块（如果没有小于的则返回第0个块
        int l = 0,r = current.keyNum - 1;
        int MinBlock = -1;
        while(l <= r) {
            int mid = (l + r)/2;
            if(current.Key[mid].k < k) {
                MinBlock = mid;
                l = mid + 1;
            }else if(current.Key[mid].k == k) {
                MinBlock = mid - 1;
                r = mid - 1;
            }else{
                r = mid - 1;
            }
        }
        return MinBlock + 1;
    }

    int searchLeafToFind(const KEY &k,LeafNode &current) {
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

    int searchIndexForInsert(const KO &k,IndexNode &current) { //查询当前索引块中相应的孩子指针 二分  对于插入删除操作
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

    int searchLeafForInsert(const KO &k,LeafNode &current) { ////查询当前叶子块中KO对的位置 二分  对于插入操作
        int l = 0,r = current.num - 1;
        int ans = -1;
        while(l <= r) {
            int mid = (l + r)/2;
            if(current.Info[mid] < k) {
                ans = mid;
                l = mid + 1;
            }else if(current.Info[mid] == k) {
                ans = -2;
                break;
            }else {
                r = mid - 1;
            }
        }
        return ans + 1;//在current中的插入位置 返回-1则表示值已经存在
    }

    int searchLeafForErase(const KO &k,LeafNode &current) { //查询当前叶子块中KO对的位置 二分  对于删除操作
        int l = 0,r = current.num - 1;
        int ans = -1;
        while(l <= r) {
            int mid = (l + r)/2;
            if(current.Info[mid] < k) {
                l = mid + 1;
            }else if(current.Info[mid] == k) {
                ans = mid;
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
        //secondRoot.pos = ++nextIndexNodePos;
        secondRoot.pos = allocateIndexPos();
        secondRoot.is_leaf = current.is_leaf;
        secondRoot.keyNum = mid - 1;
        current.keyNum = mid - 1;
        writeIndexNode(secondRoot);
        writeIndexNode(current);
        IndexNode newRoot;
        newRoot.ChildPointer[0] = current.pos;
        newRoot.ChildPointer[1] = secondRoot.pos;
        newRoot.Key[0] = current.Key[mid -1];
        //newRoot.pos = ++nextIndexNodePos;
        newRoot.pos = allocateIndexPos();
        newRoot.keyNum = 1;
        newRoot.is_leaf = false;
        root = newRoot;
        writeIndexNode(newRoot);
    }

    void splitNormalIndexNode(IndexNode &current,IndexNode &above,int idx,bool &a) {
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
        //secondIndexNode.pos = ++nextIndexNodePos;
        secondIndexNode.pos = allocateIndexPos();
        secondIndexNode.keyNum = mid - 1;
        current.keyNum = mid - 1;
        writeIndexNode(secondIndexNode);
        writeIndexNode(current);
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
            a = true;
        }else {
            writeIndexNode(above);
            a = false;
        }
    }

    void splitLeaf(LeafNode &current,IndexNode &above,int idx,bool &a) {
        //分裂叶结点
        LeafNode newLeaf;
        int mid = L/2;
        for(int i = 0;i < mid;i ++) {
            newLeaf.Info[i] = current.Info[i + mid];
        }
        newLeaf.num = mid;
        current.num = mid;
        //newLeaf.pos = ++ nextLeafNodePos;
        newLeaf.pos = allocateLeadfPos();
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
            a = true;
        }else{
            writeIndexNode(above);
            a = false;
        }
    }

    bool Insert(IndexNode &current,KO &tmp) {
        /*
         * 该函数如果需要继续分裂块，则return true ，不然return false
         * 递归调用
         * 先从indexnode下面就是叶结点开始考虑
         */
        if(current.is_leaf) {
            int idx1 = searchIndexForInsert(tmp,current);
            LeafNode search;
            readLeafNode(search,current.ChildPointer[idx1]);
            int idx = searchLeafForInsert(tmp,search);//插入位置的坐标
            if(idx == -1) {
                return false;//插入节点原本就存在
            }
            totalNum ++;
            for(int i = search.num;i > idx;i --) {
                search.Info[i] = search.Info[i - 1];
            }
            search.Info[idx] = tmp;
            search.num ++;
            if(search.num == L) { //需要裂块
                bool a ;
                splitLeaf(search,current,idx1,a);
                if(a) { //需要对索引块进行分裂
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
            readIndexNode(child,current.ChildPointer[idx]);
            if(Insert(child,tmp)) {  //需要对普通的IndexNode进行分裂
                bool a ;
                splitNormalIndexNode(child,current,idx,a);
                if(a) {
                    return true;
                }else {
                    return false;
                }
            }else {
                return false;
            }
        }
    }

    void mergeRightLeaf(LeafNode &current,LeafNode &nextLeaf,IndexNode &above,int idx,bool &a) {
        for(int i = 0;i < L/2;i ++) {
            current.Info[i + L/2 - 1] = nextLeaf.Info[i];
        }
        current.num += L/2;
        current.next = nextLeaf.next;
        LeafCache.erase(nextLeaf.pos);//修改缓存
        releaseLeafPos(nextLeaf.pos);//空间回收
        writeLeafNode(current);
        for(int i = idx;i < above.keyNum - 1;i ++) {
            above.Key[i] = above.Key[i + 1];
        }
        for(int i = idx + 1;i < above.keyNum;i ++) {
            above.ChildPointer[i] = above.ChildPointer[i + 1];
        }
        above.keyNum --;
        if(above.keyNum < M/2 + 1) {
            a = true;
        }else {
            writeIndexNode(above);
            a = false;
        }
    }

    void mergeLeftLeaf(LeafNode &current,LeafNode &beforeLeaf,IndexNode &above,int idx,bool &a) {
        for(int i = 0;i < L/2 - 1;i ++) {
            beforeLeaf.Info[i + L/2] = current.Info[i];
        }
        beforeLeaf.num += L/2 - 1;
        beforeLeaf.next = current.next;
        LeafCache.erase(current.pos);
        releaseLeafPos(current.pos);//空间回收
        writeLeafNode(beforeLeaf);
        for(int i = idx - 1;i < above.keyNum - 1;i ++) {
            above.Key[i] = above.Key[i + 1];
        }
        for(int i = idx;i < above.keyNum;i ++) {
            above.ChildPointer[i] = above.ChildPointer[i + 1];
        }
        above.keyNum --;
        if(above.keyNum < M/2 + 1) {
            a = true;
        }else {
            writeIndexNode(above);
            a = false;
        }
    }

    void mergeRightIndex(IndexNode &current,IndexNode &nextIndex,IndexNode &above,int idx,bool &a) {
        for(int i = 0;i < nextIndex.keyNum;i ++) {
            current.Key[i + current.keyNum + 1] = nextIndex.Key[i];
        }
        for(int i = 0;i < nextIndex.keyNum + 1;i ++) {
            current.ChildPointer[i + current.keyNum + 1] = nextIndex.ChildPointer[i];
        }
        current.Key[current.keyNum] = above.Key[idx];
        current.keyNum += nextIndex.keyNum + 1;
        for(int i = idx;i < above.keyNum - 1;i ++) {
            above.Key[i] = above.Key[i + 1];
        }
        for(int i = idx + 1;i < above.keyNum;i ++) {
            above.ChildPointer[i] = above.ChildPointer[i + 1];
        }
        above.keyNum --;
        IndexCache.erase(nextIndex.pos);
        releaseIndexPos(nextIndex.pos);//空间回收
        writeIndexNode(current);

        if(above.keyNum < M/2 + 1) {
            a = true;
        }else {
            writeIndexNode(above);
            a = false;
        }
    }

    void mergeLeftIndex(IndexNode &current,IndexNode &beforeIndex,IndexNode &above,int idx,bool &a) {
        for(int i = 0;i < current.keyNum;i ++) {
            beforeIndex.Key[i + beforeIndex.keyNum + 1] = current.Key[i];
        }
        for(int i = 0;i < current.keyNum + 1;i ++) {
            beforeIndex.ChildPointer[i + beforeIndex.keyNum + 1] = current.ChildPointer[i];
        }
        beforeIndex.Key[beforeIndex.keyNum] = above.Key[idx - 1];
        beforeIndex.keyNum += current.keyNum + 1;
        for(int i = idx - 1;i < above.keyNum - 1;i ++) {
            above.Key[i] = above.Key[i + 1];
        }
        for(int i = idx;i < above.keyNum;i ++) {
            above.ChildPointer[i] = above.ChildPointer[i + 1];
        }
        above.keyNum --;
        IndexCache.erase(current.pos);
        releaseIndexPos(current.pos);//空间回收
        writeIndexNode(beforeIndex);

        if(above.keyNum < M/2 + 1) {
            a = true;
        }else {
            writeIndexNode(above);
            a =  false;
        }
    }

    bool Erase(IndexNode &current,KO &tmp) {
        if(current.is_leaf) {
            int targetPos = searchIndexForInsert(tmp,current);
            LeafNode target;
            readLeafNode(target,current.ChildPointer[targetPos]);
            int idx = searchLeafForErase(tmp,target);
            if(idx == -1) {  //没找到
                return false;
            }
            /*  tmp = 3     L = 6
             *  1 2 3 5  idx = 3
             *  1 2 5
             */
            for(int i = idx;i < target.num - 1;i ++) {
                target.Info[i] = target.Info[i + 1];
            }
            target.num --;
            totalNum --;
            if(target.num < L/2) { //太少了需要借或者并块
                // 首先考虑从右边的块那儿借一个
                bool rightBlock = false;
                LeafNode nextLeaf;
                LeafNode beforeLeaf;
                if(targetPos < current.keyNum) {  //下一个存在
                    rightBlock = true;
                    readLeafNode(nextLeaf,current.ChildPointer[targetPos + 1]);
                    if(nextLeaf.num > L/2) {  //可以借用 考虑借用第一个
                        target.Info[target.num] = nextLeaf.Info[0];
                        for(int i = 0;i < nextLeaf.num - 1;i ++) {
                            nextLeaf.Info[i] = nextLeaf.Info[i + 1];
                        }
                        nextLeaf.num --;
                        target.num ++;
                        current.Key[targetPos] = nextLeaf.Info[0];
                        writeIndexNode(current);
                        writeLeafNode(nextLeaf);
                        writeLeafNode(target);
                        return false;
                    }
                }
                //再考虑向左
                bool leftBlock = false;
                if(targetPos >= 1) {
                    leftBlock = true;
                    readLeafNode(beforeLeaf,current.ChildPointer[targetPos - 1]);
                    if(beforeLeaf.num > L/2) {
                        for(int i = target.num;i >= 1;i --) {
                            target.Info[i] = target.Info[i - 1];
                        }
                        target.Info[0] = beforeLeaf.Info[beforeLeaf.num - 1];
                        target.num ++;
                        beforeLeaf.num --;
                        current.Key[targetPos - 1] = target.Info[0];
                        writeIndexNode(current);
                        writeLeafNode(beforeLeaf);
                        writeLeafNode(target);
                        return false;
                    }
                }
                if(rightBlock) {  //右块存在
                    bool a ;
                    mergeRightLeaf(target,nextLeaf,current,targetPos,a);
                    if(a) {
                        return true;
                    }else {
                        return false;
                    }
                }
                if(leftBlock) {
                    bool b;
                    mergeLeftLeaf(target,beforeLeaf,current,targetPos,b);
                    if(b) {
                        return true;
                    }else {
                        return false;
                    }
                }
                writeLeafNode(target);
                return false;
            }else {
                writeLeafNode(target);
                return false;
            }
        }
        IndexNode child;
        int idx = searchIndexForInsert(tmp,current);
        readIndexNode(child,current.ChildPointer[idx]);
        if(Erase(child,tmp)) {  //需要对该IndexNode进行合并或借块
            IndexNode nextIndex;
            IndexNode beforeIndex;
            bool rightIndex = false;
            if(idx < current.keyNum) {
                rightIndex = true;
                readIndexNode(nextIndex,current.ChildPointer[idx + 1]);
                if(nextIndex.keyNum + 1 > M/2) {
                    child.Key[child.keyNum] = current.Key[idx];
                    child.ChildPointer[child.keyNum + 1] = nextIndex.ChildPointer[0];
                    current.Key[idx] = nextIndex.Key[0];
                    for(int i = 0;i < nextIndex.keyNum - 1;i ++) {
                        nextIndex.Key[i] = nextIndex.Key[i + 1];
                    }
                    for(int i = 0;i < nextIndex.keyNum;i ++) {
                        nextIndex.ChildPointer[i] = nextIndex.ChildPointer[i + 1];
                    }
                    child.keyNum ++;
                    nextIndex.keyNum --;
                    writeIndexNode(current);
                    writeIndexNode(child);
                    writeIndexNode(nextIndex);
                    return false;
                }
            }
            bool leftIndex = false;
            if(idx >= 1) {
                leftIndex = true;
                readIndexNode(beforeIndex,current.ChildPointer[idx - 1]);
                if(beforeIndex.keyNum + 1> M/2 ) {
                    for(int i = child.keyNum;i > 0;i --) {
                        child.Key[i] = child.Key[i - 1];
                    }
                    for(int i = child.keyNum + 1;i > 0;i --) {
                        child.ChildPointer[i] = child.ChildPointer[i - 1];
                    }
                    child.Key[0] = current.Key[idx - 1];
                    child.ChildPointer[0] = beforeIndex.ChildPointer[beforeIndex.keyNum];
                    current.Key[idx - 1] = beforeIndex.Key[beforeIndex.keyNum - 1];
                    beforeIndex.keyNum --;
                    child.keyNum ++;
                    writeIndexNode(current);
                    writeIndexNode(beforeIndex);
                    writeIndexNode(child);
                    return false;
                }
            }
            if(rightIndex) {
                bool c;
                mergeRightIndex(child,nextIndex,current,idx,c);
                if(c) {
                    return true;
                }else {
                    return false;
                }
            }
            if(leftIndex) {
                bool d;
                mergeLeftIndex(child,beforeIndex,current,idx,d);
                if(d) {
                    return true;
                }else {
                    return false;
                }
            }
            return false;
        }else {
            return false;
        }
    }

public:
    BPT(const std::string &s1,const std::string &s2):IndexCache(20000),LeafCache(20000) {
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
        IndexNode current = root;
        LeafNode target;
        int idx;
        while(!current.is_leaf) {
            idx = searchIndexToFind(k,current);
            readIndexNode(current,current.ChildPointer[idx]);
        }
        idx = searchIndexToFind(k,current);
        readLeafNode(target,current.ChildPointer[idx]);
        idx = searchLeafToFind(k,target);

        for(int i = idx;i < target.num;i ++) {
            if(target.Info[i].k == k){
                results.push_back(target.Info[i].other);
            }
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
            splitRoot(root);
        }
        UpdateMetaData();
        closeFile();
    }

    void erase(const KEY &k,const OTHER &other) {
        openFile();
        KO tmp(k,other);
        if(Erase(root,tmp)) {
            if(root.keyNum == 0) {
                if(!root.is_leaf) {
                    IndexNode newRoot;
                    readIndexNode(newRoot,root.ChildPointer[0]);
                    root = newRoot;
                }
            }
        }
        UpdateMetaData();
        closeFile();
    }
};
#endif //BPT_H