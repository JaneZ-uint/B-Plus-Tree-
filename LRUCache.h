//
// Created by JaneZ on 2025/4/21.
//
#ifndef LRUCACHE_H
#define LRUCACHE_H

#include "BPT.h"
#include "list.h"

template<class KEY,class OTHER>
class LRUCache {
private:
    struct CacheEntry {
        KEY key;
        OTHER value;

        CacheEntry(const KEY& k,const OTHER& other):key(k),value(other){}
    };
    list<CacheEntry> cacheList; //用于维护访问先后顺序

    struct HashMapNode {
        KEY key;
        typename list<CacheEntry>::Node* list_node;
        HashMapNode* next;

        HashMapNode(const KEY&k,typename list<CacheEntry>::Node* p = nullptr,HashMapNode* n = nullptr):key(k),list_node(p),next(n){}
    };

    class HashTable {
    private:
        static const size_t HashTableSize = 20000;
        HashMapNode* table[HashTableSize];

        size_t HASH(const KEY&k) {
            return static_cast<size_t>(k) % HashTableSize;
        }
    public:
        HashTable() {
            for(size_t i = 0;i < HashTableSize;i ++) {
                table[i] = nullptr;
            }
        }
        ~HashTable() {
            clear();
        }

        void insert(const KEY& key,typename list<CacheEntry>::Node* node) {
            size_t h = HASH(key);
            HashMapNode* new_node = new HashMapNode(key,node);
            new_node->next = table[h];
            table[h] = new_node;
        }

        typename list<CacheEntry>::Node* find(const KEY& k) {
            size_t h = HASH(k);
            HashMapNode* current = table[h];
            while(current) {
                if(current->key == k) {
                    return current->list_node;
                }
                current = current->next;
            }
            return nullptr;
        }

        void erase(const KEY& k) {
            size_t h = HASH(k);
            HashMapNode* current = table[h];
            HashMapNode* prev = nullptr;

            while(current) {
                if(current->key == k) {
                    if(prev) {
                        prev->next = current->next;
                    }else {
                        table[h] = current->next;
                    }
                    delete current;
                    return;
                }
                prev = current;
                current = current->next;
            }
        }

        void clear() {
            for(size_t i = 0;i < HashTableSize;i ++) {
                HashMapNode* current = table[i];
                while(current) {
                    HashMapNode* tmp = current;
                    current = current->next;
                    delete tmp;
                }
                table[i] = nullptr;
            }
        }
    };

    HashTable unordered_map;
    size_t capacity;

    void move_to_front(typename list<CacheEntry>::Node* node) {
        if (!node || cacheList.empty() || node == cacheList.begin().get_node()) {
            return;
        }
        cacheList.splice(cacheList.begin(), cacheList, typename list<CacheEntry>::iterator(node));
    }

public:
    LRUCache(size_t Capacity):capacity(Capacity){}

    ~LRUCache() {
        clear();
    }

    bool get(const KEY& k,OTHER& other) {
        auto node = unordered_map.find(k);
        if(!node) {
            return false;
        }
        other = node->data.value;
        move_to_front(node);
        return true;
    }

    void put(const KEY& k,const OTHER& value) {
        auto node = unordered_map.find(k);

        //本来就存在
        if(node) {
            node->data.value = value;
            move_to_front(node);
            return;
        }

        if(cacheList.size() >= capacity) {
            //std::cout << "Full Full Full Full Full Full JaneZ!" << '\n';
            auto last = cacheList.back();
            unordered_map.erase(last.key);
            cacheList.pop_back();
        }

        cacheList.push_front(CacheEntry(k,value));
        unordered_map.insert(k,cacheList.begin().get_node());
    }

    void erase(const KEY& k) {
        auto node = unordered_map.find(k);
        if(!node) {
            return;
        }
        auto it = typename list<CacheEntry>::iterator(node);
        cacheList.erase(it);
        unordered_map.erase(k);

    }

    void clear() {
        unordered_map.clear();
        cacheList.clear();
    }

    size_t size() {
        return cacheList.size();
    }

    size_t getCapacity() {
        return capacity;
    }
};

#endif //LRUCACHE_H
