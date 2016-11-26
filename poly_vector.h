/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <algorithm>
using std::size_t;
using std::ptrdiff_t;

template<class Base, class Allocator = std::allocator<unsigned char>>
class poly_vector
{
    static_assert(sizeof(Allocator::value_type) == 1,
                  "Allocator value type must be of size 1");
    
    typedef unsigned char byte;
    
    class obj_data_base
    {
    public:
        obj_data_base(){}
        virtual ~obj_data_base(){}
        virtual obj_data_base* move_construct(void* addr) = 0;
        virtual obj_data_base* copy_construct(void* addr) = 0;
        virtual Base* get() = 0;
        virtual size_t size() = 0;
    };

    template<class Subclass>
    class obj_data : public obj_data_base
    {
    public:
        Subclass obj;

        template<class U>
        obj_data(U&& obj)
            : obj(std::forward<U>(obj)){}

        virtual obj_data_base* move_construct(void* addr) override {
            return new (addr) obj_data<Subclass>(std::move(obj));
        };

        virtual obj_data_base* copy_construct(void* addr) override {
            return new (addr) obj_data<Subclass>(obj);
        };

        virtual Base* get() override {
            return &obj;
        }

        virtual size_t size() override {
            return sizeof(obj_data<Subclass>);
        }
    };

    Allocator alloc;
    byte* _first;
    byte* _last;
    byte* _end;
public:

    class iterator
    {
        byte* ptr;
    public:
        iterator(byte* ptr)
            : ptr(ptr){}

        Base* operator->() const {
            auto obj = (obj_data_base*)ptr;
            return obj->get();
        }
        
        Base* operator*() const {
            auto obj = (obj_data_base*)ptr;
            return obj->get();
        }

        iterator& operator++() {
            auto obj = (obj_data_base*)ptr;
            ptr += obj->size();
            return *this;
        }
        
        iterator operator++(int) {
            T* p = ptr;
            auto obj = (obj_data_base*)ptr;
            ptr += obj->size();
            return p;
        }

        bool operator==(const iterator& it) const {
            return ptr == it.ptr;
        }
        
        bool operator!=(const iterator& it) const {
            return ptr != it.ptr;
        }
    };

    poly_vector() :
        _first(nullptr),
        _last(nullptr),
        _end(nullptr){}

    poly_vector(const poly_vector& pv)
    {
        alloc = pv.alloc;
        _alloc(pv.capacity());
        _copyElements(pv);
    }

    poly_vector(poly_vector&& pv) {
        _moveShallow(pv);
    }
    
    poly_vector& operator=(const poly_vector& pv)
    {
        clear();

        size_t pvCap = pv.capacity();
        if(alloc != pv.alloc)
        {
            _dealloc();
            alloc = pv.alloc;
            _alloc(pvCap);
        }

        if(capacity() < pvCap)
        {
            _dealloc();
            _alloc(pvCap);
        }

        _copyElements(pv);
        return *this;
    }

    poly_vector& operator=(poly_vector&& pv) {
        clear();
        _dealloc();
        _moveShallow(pv);
        return *this;
    }

    ~poly_vector() {
        clear();
        _dealloc();
    }

    void reserve(size_t count, size_t size) {
        reserve(count * (sizeof(obj_data_base) + size));
    }

    void reserve(size_t required)
    {
        size_t current_cap = capacity();
        if(current_cap < required)
        {
            size_t cap = 1;
			while(cap < required)
				cap <<= 1;

            byte* mem = alloc.allocate(cap);
            byte* new_last = mem;

            for(byte* e = _first; e != _last; )
            {
                auto obj = (obj_data_base*)e;
                size_t sz = obj->size();
                obj->move_construct(new_last);
                obj->~obj_data_base();
                new_last += sz;
                e += sz;
            }

            _dealloc();

            _first = mem;
            _last = new_last;
            _end = mem + cap;
        }
    }

    template<class Subclass>
    void push_back(Subclass&& obj)
    {
        typedef std::decay<Subclass>::type U;
        static_assert(std::is_base_of<Base, U>::value, "'Subclass' must be a subclass of 'Base'");

        reserve(size() + sizeof(obj_data<U>));
        new (_last) obj_data<U>(std::forward<Subclass>(obj));
        _last += sizeof(obj_data<U>);
    }

    void clear()
    {
        auto *first = _first;
        while(first != _last)
        {
            auto obj = (obj_data_base*)first;
            size_t sz = obj->size();
            obj->~obj_data_base();
            first += sz;
        }

        _last = _first;
    }

    size_t capacity() const {
        return _end - _first;
    }

    size_t size() const {
        return _last - _first;
    }

    bool empty() const {
        return _first == _last;
    }

    iterator begin() {
        return _first;
    }

    iterator end() {
        return _last;
    }

private:
    void _alloc(size_t capacity)
    {
        _first = alloc.allocate(capacity);
        _last = _first;
        _end = _first + capacity;
    }

    void _dealloc()
    {
        if(_first)
            alloc.deallocate(_first, capacity());
    }

    void _copyElements(const poly_vector& pv)
    {
        auto pvFirst = pv._first;
        auto pvLast = pv._last;

        while(pvFirst != pvLast)
        {
            auto obj = (obj_data_base*)pvFirst;
            size_t sz = obj->size();
            obj->copy_construct(_last);
            _last += sz;
            pvFirst += sz;
        }
    }

    void _moveShallow(poly_vector&& pv)
    {
        alloc = std::move(pv.alloc);
        _first = pv._first;
        _last = pv._last;
        _end = pv._end;
        pv._first = nullptr;
        pv._last = nullptr;
        pv._end = nullptr;
    }
};
