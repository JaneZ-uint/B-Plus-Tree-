//
// Created by JaneZ on 2025/4/21.
//

#ifndef LIST_H
#define LIST_H
#include "LRUCache.h"
template<class NodeType>
class list {
public:
    struct Node {
        Node *prev = nullptr;
        Node *next = nullptr;
        NodeType data;

        Node(const NodeType &t):prev(nullptr),next(nullptr),data(t){}
        ~Node() {
            prev = nullptr;
            next = nullptr;
        }
    };

    Node *head = nullptr;
    Node *tail = nullptr;
    size_t list_capacity;
public:
    list():head(nullptr),tail(nullptr),list_capacity(0){}

    ~list() {
        clear();
    }

    void clear() {
        while(!empty()) {
            pop_front();
        }
        head = nullptr;
        tail = nullptr;
    }

    class iterator {
    private:
        Node *current = nullptr;

    public:
        iterator(Node* t = nullptr):current(t){}

        ~iterator() {
            current = nullptr;
        }

        NodeType& operator*() const {
            return current->data;
        }

        NodeType* operator->() const {
            return &(current->data);
        }

        iterator& operator++() {
            current = current->next;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            current = current->next;
            return tmp;
        }

        iterator& operator--() {
            current = current->prev;
            return *this;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            current = current->prev;
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return current == other.current;
        }

        bool operator!=(const iterator& other) const {
            return current != other.current;
        }

        Node* get_node() const {
            return current;
        }
    };

    iterator begin() {
        return iterator(head);
    }
    iterator end() {
        return iterator(nullptr);
    }

    bool empty() const {
        return list_capacity == 0;
    }

    size_t size() {
        return list_capacity;
    }

    NodeType& front() {
        return head->data;
    }

    NodeType& back() {
        return tail->data;
    }

    void push_front(const NodeType &value) {
        Node *newNode = new Node(value);
        if(head == nullptr) {
            head = newNode;
            tail = newNode;
        }else {
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        list_capacity ++;
    }

    void push_back(const NodeType& value) {
        Node *newNode = new Node(value);
        if(tail == nullptr) {
            head = newNode;
            tail = newNode;
        }else {
            newNode->prev = tail;
            tail->next = newNode;
            tail = newNode;
        }
        list_capacity ++;
    }

    void pop_front() {
        if(head == nullptr) {
            return;
        }
        Node *tmp = head;
        head = head->next;
        if(head) {
            head->prev = nullptr;
        }else {
            tail = nullptr;
        }
        delete tmp;
        list_capacity --;
    }

    void pop_back() {
        if(tail == nullptr) {
            return;
        }
        Node *tmp = tail;
        tail = tail->prev;
        if(tail) {
            tail->next = nullptr;
        }else {
            head = nullptr;
        }
        delete tmp;
        list_capacity --;
    }

    iterator insert(iterator pos, const NodeType& value) {
        if (pos == begin()) {
            push_front(value);
            return begin();
        } else if (pos == end()) {
            push_back(value);
            return iterator(tail);
        } else {
            Node *new_node = new Node(value);
            Node *current = pos.get_node();
            new_node->prev = current->prev;
            new_node->next = current;
            current->prev->next = new_node;
            current->prev = new_node;
            list_capacity ++;
            return iterator(new_node);
        }
    }

    iterator erase(iterator pos) {
        if (pos == end()) return end();
        Node *current = pos.get_node();
        iterator next_iter(current->next);
        if (current->prev) {
            current->prev->next = current->next;
        } else {
            head = current->next;
        }
        if (current->next) {
            current->next->prev = current->prev;
        } else {
            tail = current->prev;
        }
        delete current;
        list_capacity --;
        return next_iter;
    }

    void splice(iterator pos, list& other, iterator it) {
        if (it == other.end() || other.empty() || &other == this && it == pos) return;
        Node *node = it.get_node();
        //从other链表中移除
        if(node->prev != nullptr) {
            node->prev->next = node->next;
        }else {
            other.head = node->next;
        }

        if(node->next != nullptr) {
            node->next->prev = node->prev;
        }else {
            other.tail = node->prev;
        }
        other.list_capacity --;

        //插入当前链表
        if(pos == begin()) {
            node->prev = nullptr;
            node->next = head;
            if(head) {
                head->prev = node;
            }else {
                tail = node;
            }
            head = node;
        }else if(pos == end()) {
            node->next = nullptr;
            node->prev = tail;
            if(tail) {
                tail->next = node;
            }else {
                head = node;
            }
            tail = node;
        }else {
            Node *current = pos.get_node();
            node->prev = current->prev;
            node->next = current;
            if(current->prev) {
                current->prev->next = node;
            }else {
                head = node;
            }
            current->prev = node;
        }
        list_capacity ++;
    }
};

#endif //LIST_H
