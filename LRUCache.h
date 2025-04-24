//
// Created by JaneZ on 2025/4/21.
//
#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <unordered_map>

#include "BPT.h"
#include "list.h"
#include "unordered_map.h"
template<class NodeType>
class LRUCache {
private:
    size_t capacity;
    list<NodeType> cacheList;
    unordered_map<NodeType> cacheMap;

public:

};

#endif //LRUCACHE_H
