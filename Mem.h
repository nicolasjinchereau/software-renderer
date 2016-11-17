/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdint>
#include <memory>

void *AlignedAlloc(size_t size, size_t alignment);
void AlignedFree(void* ptr);
bool IsAligned(const void* ptr, size_t alignment);

template<class T, typename... Args>
T* AlignedAlloc(size_t count, size_t alignment, Args&&... args) {
    T* mem = (T*)AlignedAlloc(count * sizeof(T), alignment);
    return new (mem) T(std::forward<Args>(args)...);
}

template<typename T>
void AlignedFree(T* ptr) {
    ptr->~T();
    AlignedFree((void*)ptr);
}

template<typename T>
struct AlignedDeleter {
    void operator()(T* p) {
        AlignedFree<T>(p);
    }
};

template<int alignment>
bool IsAligned(const void* ptr)
{
    static_assert((alignment & (alignment - 1)) == 0,
                  "alignment must be a power of 2");
    constexpr int mask = alignment - 1;
    return (((uintptr_t)ptr) & mask) == 0;
}

template<typename T, size_t alignment, typename... Args>
std::shared_ptr<T> AlignedMakeShared(Args&&... args) {
    return std::shared_ptr<T>(AlignedAlloc<T>(1, alignment, std::forward<Args>(args)...),
                              AlignedFree<T>);
}

template <class T, int N>
class AlignedAllocator
{
public:
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template <class U>
    struct rebind {
        typedef AlignedAllocator<U, N> other;
    };

    inline AlignedAllocator() throw() {}
    inline AlignedAllocator(const AlignedAllocator&) throw(){}

    template <class U>
    inline AlignedAllocator(const AlignedAllocator<U, N>&) throw() {
        
    }

    inline ~AlignedAllocator() throw() {
        
    }

    inline pointer address(reference r) {
        return &r;
    }

    inline const_pointer address(const_reference r) const {
        return &r;
    }

    pointer allocate(size_type n, typename std::allocator<void>::const_pointer hint = 0) {
        pointer p = (pointer)AlignedAlloc(n * sizeof(T), N);
        if(!p) throw std::bad_alloc();
        return p;
    }

    inline void deallocate(pointer p, size_type) {
        AlignedFree((void*)p);
    }

    inline void construct(pointer p, const_reference value) {
        new (p) value_type(value);
    }

    inline void destroy(pointer p) {
        p->~T();
    }

    inline size_type max_size() const throw() {
        return size_type(-1) / sizeof(T);
    }

    inline bool operator==(const AlignedAllocator&) {
        return true;
    }
    
    inline bool operator!=(const AlignedAllocator& rhs) {
        return false;
    }
};
