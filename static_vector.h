/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdlib>
#include <exception>
#include <algorithm>
#include <iterator>
#include <initializer_list>

template<typename T, size_t Capacity>
class static_vector
{
    T _data[Capacity];
    T* _last;
public:
    typedef T              value_type;
    typedef size_t         size_type;
    typedef ptrdiff_t      difference_type;
    typedef T*             pointer;
    typedef const T*       const_pointer;
    typedef T&             reference;
    typedef const T&       const_reference;

    class iterator : public std::iterator<std::bidirectional_iterator_tag, T>
    {
        T* ptr;
    public:
        iterator(T* ptr) : ptr(ptr){}
        T& operator*() const { return *ptr; }
        T* operator->() const { return ptr; }
        iterator& operator++() { ++ptr; return *this; }
        iterator operator++(int) { T* p = ptr; ++ptr; return p; }
        iterator& operator--() { --ptr; return *this; }
        iterator operator--(int) { T* p = ptr; --ptr; return p; }
        bool operator==(const iterator& it) const { return ptr == it.ptr; }
        bool operator!=(const iterator& it) const { return ptr != it.ptr; }
    };
    
    class const_iterator : public std::iterator<std::bidirectional_iterator_tag, T>
    {
        const T* ptr;
    public:
        const_iterator(const T* ptr) : ptr(ptr){}
        const T& operator*() const { return *ptr; }
        const T* operator->() const { return ptr; }
        const_iterator& operator++() { ++ptr; return *this; }
        const_iterator operator++(int) { T* p = ptr; ++ptr; return p; }
        const_iterator& operator--() { --ptr; return *this; }
        const_iterator operator--(int) { T* p = ptr; --ptr; return p; }
        bool operator==(const const_iterator& it) const { return ptr == it.ptr; }
        bool operator!=(const const_iterator& it) const { return ptr != it.ptr; }
    };

    static_vector() : _last((T*)_data){}

    static_vector(std::initializer_list<T> il) : _last((T*)_data){
        for(auto& e : il) _pushBack(e);
    }

    static_vector(const static_vector& v) : _last((T*)_data) {
        for(auto& e : v) _pushBack(e);
    }
    
    static_vector(static_vector&& v)  : _last((T*)_data) {
        for(auto&& e : v) _pushBack(std::move(e));
        v.clear();
    }

    static_vector& operator=(const static_vector& v) {
        clear();
        for(auto& e : v) _pushBack(e);
        return *this;
    }
    
    static_vector& operator=(static_vector&& v) {
        clear();
        for(auto&& e : v) _pushBack(std::move(e));
        v.clear();
        return *this;
    }

    ~static_vector() {
        clear();
    }
    
    template<typename S>
    void push_back(S&& value) {
        if(full()) throw std::bad_alloc();
        _pushBack(std::forward<S>(value));
    }
    
    template<typename... Ts>
    void emplace_back(Ts&&... args) {
        if(full()) throw std::bad_alloc();
        new (_last++) T(std::forward<Ts>(args)...);
    }

    void pop_back() {
        (--_last)->~T();
    }

    void clear() {
        while(!empty()) pop_back();
    }

    T& operator[](size_t i) {
        return _data[i];
    }

    const T& operator[](size_t i) const {
        return _data[i];
    }

    T& front() {
        return _data[0];
    }

    const T& front() const {
        return _data[0];
    }

    T& back() {
        return *(_last - 1);
    }

    const T& back() const {
        return *(_last - 1);
    }

    T* data() {
        return _data;
    }

    const T* data() const {
        return _data;
    }

    iterator begin() {
        return iterator((T*)_data);
    }

    const_iterator begin() const {
        return const_iterator((const T*)_data);
    }

    iterator end() {
        return iterator(_last);
    }

    const_iterator end() const {
        return const_iterator(_last);
    }

    size_t size() const {
        return _last - _data;
    }

    bool empty() const {
        return _last == _data;
    }

    bool full() const {
        return _last == _data + Capacity;
    }

    size_t capacity() const {
        return Capacity;
    }

    size_t max_size() const {
        return Capacity;
    }

private:
    template<class S>
    void _pushBack(S&& e) {
        new (_last++) T(std::forward<S>(e));
    }
};
