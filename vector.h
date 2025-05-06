//
// Created by JaneZ on 2025/4/13.
//
#ifndef VECTOR_H
#define VECTOR_H

#include "exceptions.h"

#include <climits>
#include <cstddef>

namespace sjtu {
    /**
     * a data container like std::vector
     * store data in a successive memory and support random access.
     */
    template<typename T>
    class vector {
    private:
        T *data;
        size_t current;
        size_t capacity;

    public:
        class const_iterator;

        class iterator {
            // The following code is written for the C++ type_traits library.
            // Type traits is a C++ feature for describing certain properties of a type.
            // For instance, for an iterator, iterator::value_type is the type that the
            // iterator points to.
            // STL algorithms and containers may use these type_traits (e.g. the following
            // typedef) to work properly. In particular, without the following code,
            // @code{std::sort(iter, iter1);} would not compile.
            // See these websites for more information:
            // https://en.cppreference.com/w/cpp/header/type_traits
            // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
            // About iterator_category: https://en.cppreference.com/w/cpp/iterator
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T *;
            using reference = T &;
            using iterator_category = std::output_iterator_tag;

        private:
            pointer ptr;

        public:
            iterator(pointer p = nullptr): ptr(p) {
            }

            /**
             * return a new iterator which pointer n-next elements
             * as well as operator-
             */
            iterator operator+(const int &n) const {
                return iterator(ptr + n);
            }

            iterator operator-(const int &n) const {
                return iterator(ptr - n);
            }

            // return the distance between two iterators,
            // if these two iterators point to different vectors, throw invaild_iterator.
            int operator-(const iterator &rhs) const {
                return ptr - rhs.ptr;
            }

            iterator &operator+=(const int &n) {
                ptr += n;
                return *this;
            }

            iterator &operator-=(const int &n) {
                ptr -= n;
                return *this;
            }

            iterator operator++(int) {
                iterator temp = *this;
                ++ptr;
                return temp;
            }

            iterator &operator++() {
                ++ptr;
                return *this;
            }

            iterator operator--(int) {
                iterator temp = *this;
                --ptr;
                return temp;
            }

            iterator &operator--() {
                --ptr;
                return *this;
            }

            T &operator*() const {
                return *ptr;
            }

            bool operator==(const iterator &rhs) const {
                return ptr == rhs.ptr;
            }

            bool operator!=(const iterator &rhs) const {
                return ptr != rhs.ptr;
            }
        };

        class const_iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T *;
            using reference = T &;
            using iterator_category = std::output_iterator_tag;

        private:
            pointer ptr;

        public:
            const_iterator(pointer p = nullptr): ptr(p) {
            }

            /**
             * return a new iterator which pointer n-next elements
             * as well as operator-
             */
            const_iterator operator+(const int &n) const {
                return const_iterator(ptr + n);
            }

            const_iterator operator-(const int &n) const {
                return const_iterator(ptr - n);
            }

            // return the distance between two iterators,
            // if these two iterators point to different vectors, throw invaild_iterator.
            int operator-(const const_iterator &rhs) const {
                return ptr - rhs.ptr;
            }

            const_iterator &operator+=(const int &n) {
                ptr += n;
                return *this;
            }

            const_iterator &operator-=(const int &n) {
                ptr -= n;
                return *this;
            }

            const_iterator operator++(int) {
                const_iterator temp = *this;
                ++ptr;
                return temp;
            }

            const_iterator &operator++() {
                ++ptr;
                return *this;
            }

            const_iterator operator--(int) {
                const_iterator temp = *this;
                --ptr;
                return temp;
            }

            const_iterator &operator--() {
                --ptr;
                return *this;
            }

            const T &operator*() const {
                return *ptr;
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory address).
             */
            bool operator==(const const_iterator &rhs) const {
                return ptr == rhs.ptr;
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const const_iterator &rhs) const {
                return ptr != rhs.ptr;
            }
        };

        vector(): current(0), capacity(100) {
            data = (T *) malloc(capacity * sizeof(T));
        }

        vector(const vector &other) : current(other.current), capacity(other.capacity) {
            data = (T *) malloc(other.capacity * sizeof(T));
            for (int i = 0; i < other.current; i++) {
                new(data + i) T(other[i]);
            }
        }

        ~vector() {
            for (int i = 0; i < current; i++) {
                data[i].~T();
            }
            free(data);
        }

        vector &operator=(const vector &other) {
            if (this != &other) {
                for (int i = 0; i < current; i++) {
                    data[i].~T();
                }
                free(data);
                current = other.current;
                capacity = other.capacity;
                data = (T *) malloc(other.capacity * sizeof(T));
                for (int i = 0; i < other.current; i++) {
                    new(data + i) T(other[i]);
                }
            }
            return *this;
        }

        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         */
        T &at(const size_t &pos) {
            return data[pos];
        }

        const T &at(const size_t &pos) const {
            return data[pos];
        }

        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         * !!! Pay attentions
         *   In STL this operator does not check the boundary but I want you to do.
         */
        T &operator[](const size_t &pos) {
            return at(pos);
        }

        const T &operator[](const size_t &pos) const {
            return at(pos);
        }

        /**
         * access the first element.
         * throw container_is_empty if size == 0
         */
        const T &front() const {
            return *data;
        }

        /**
         * access the last element.
         * throw container_is_empty if size == 0
         */
        const T &back() const {
            return data[current - 1];
        }

        /**
         * returns an iterator to the beginning.
         */
        iterator begin() {
            return iterator(data);
        }

        const_iterator cbegin() const {
            return const_iterator(data);
        }

        /**
         * returns an iterator to the end.
         */
        iterator end() {
            return iterator(data + current);
        }

        const_iterator cend() const {
            return const_iterator(data + current);
        }

        /**
         * checks whether the container is empty
         */
        bool empty() const {
            return current == 0;
        }

        /**
         * returns the number of elements
         */
        size_t size() const {
            return current;
        }

        /**
         * clears the contents
         */
        void clear() {
            for (int i = 0; i < current; i++) {
                data[i].~T();
            }
            current = 0;
        }

        /**
         * inserts value before pos
         * returns an iterator pointing to the inserted value.
         */
        iterator insert(iterator pos, const T &value) {
            size_t index = pos - begin();
            if (current == capacity) {
                size_t new_capacity = capacity == 0 ? 100 : capacity * 10;
                T *new_data = (T *) malloc(new_capacity * sizeof(T));
                for (size_t i = 0; i < index; ++i) {
                    new(new_data + i) T(std::move(data[i]));
                    data[i].~T();
                }
                new(new_data + index) T(value);
                for (size_t i = index; i < current; ++i) {
                    new(new_data + i + 1) T(std::move(data[i]));
                    data[i].~T();
                }
                free(data);
                data = new_data;
                capacity = new_capacity;
            } else {
                for (size_t i = current; i > index; --i) {
                    new(data + i) T(std::move(data[i - 1]));
                    data[i - 1].~T();
                }
                new(data + index) T(value);
            }
            ++current;
            return iterator(data + index);
        }

        /**
         * inserts value at index ind.
         * after inserting, this->at(ind) == value
         * returns an iterator pointing to the inserted value.
         * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
         */
        iterator insert(const size_t &ind, const T &value) {
            if (current == capacity) {
                size_t new_capacity = capacity == 0 ? 100 : capacity * 10;
                T *new_data = (T *) malloc(new_capacity * sizeof(T));
                if (!new_data) {
                    throw std::bad_alloc();
                }
                for (size_t i = 0; i < ind; ++i) {
                    new(new_data + i) T(std::move(data[i]));
                    data[i].~T();
                }
                new(new_data + ind) T(value);
                for (size_t i = ind; i < current; ++i) {
                    new(new_data + i + 1) T(std::move(data[i]));
                    data[i].~T();
                }
                free(data);
                data = new_data;
                capacity = new_capacity;
            } else {
                for (size_t i = current; i > ind; --i) {
                    new(data + i) T(std::move(data[i - 1]));
                    data[i - 1].~T();
                }
                new(data + ind) T(value);
            }
            ++current;
            return iterator(data + ind);
        }

        /**
         * removes the element at pos.
         * return an iterator pointing to the following element.
         * If the iterator pos refers the last element, the end() iterator is returned.
         */
        iterator erase(iterator pos) {
            size_t index = pos - begin();
            data[index].~T();
            for (size_t i = index; i < current - 1; ++i) {
                new(data + i) T(std::move(data[i + 1]));
                data[i + 1].~T();
            }
            --current;
            return iterator(data + index);
        }

        /**
         * removes the element with index ind.
         * return an iterator pointing to the following element.
         * throw index_out_of_bound if ind >= size
         */
        iterator erase(const size_t &ind) {
            data[ind].~T();
            for (size_t i = ind; i < current - 1; ++i) {
                new(data + i) T(std::move(data[i + 1]));
                data[i + 1].~T();
            }
            --current;
            return iterator(data + ind);
        }

        /**
         * adds an element to the end.
         */
        void push_back(const T &value) {
            insert(end(), value);
        }

        /**
         * remove the last element from the end.
         * throw container_is_empty if size() == 0
         */
        void pop_back() {
            --current;
            data[current].~T();
        }
    };
}

#endif //VECTOR_H
