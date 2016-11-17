/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Mem.h"
#include <cstdlib>
#include <cassert>

void *AlignedAlloc(size_t size, size_t alignment) {
    void *mem = malloc(sizeof(void*) + (alignment - 1) + size);
    void **ptr = (void**)((uintptr_t)((char*)mem + sizeof(void*) + alignment) & ~(alignment - 1));
    ptr[-1] = mem;
    return ptr;
}

void AlignedFree(void* ptr) {
    free(((void**)ptr)[-1]);
}

bool IsAligned(const void* ptr, size_t alignment)
{
    // alignment must be a power of 2
    assert((alignment & (alignment - 1)) == 0);
    return (((uintptr_t)ptr) & (alignment - 1)) == 0;
}
