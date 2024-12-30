#include "HeapManager.h"
#include "FixedSizeAllocator.h"
#include "HeapManagerProxy.h"
#include <cstdio>
#include <inttypes.h>
#include <malloc.h>

// External global pointers for the allocators and heap manager
extern FixedSizeAllocator* s_pAllocators[3];
extern HeapManager* s_pHeapManager;

// Overloaded operator new
void* operator new(size_t requestedSize)
{
    printf("operator new: size = %zu\n", requestedSize);

    // Attempt using our HeapManager if available
    if (s_pHeapManager)
    {
        return s_pHeapManager->alloc(requestedSize);
    }

    // Fallback to aligned malloc
    return _aligned_malloc(requestedSize, 4);
}

// Overloaded operator delete
void operator delete(void* ptr)
{
    printf("operator delete: ptr = 0x%" PRIXPTR "\n", reinterpret_cast<uintptr_t>(ptr));
    if (s_pHeapManager && s_pHeapManager->IsAllocated(ptr))
    {
        HeapManagerProxy::free(s_pHeapManager, ptr);
        return;
    }
    _aligned_free(ptr);
}

// Overloaded operator new[]
void* operator new[](size_t requestedSize)
{
    printf("operator new[]: size = %zu\n", requestedSize);
    return _aligned_malloc(requestedSize, 4);
}

// Overloaded operator delete[]
void operator delete[](void* ptr)
{
    printf("operator delete[]: ptr = 0x%" PRIXPTR "\n", reinterpret_cast<uintptr_t>(ptr));

    // Check if it's allocated by one of our FixedSizeAllocators
    for (int index = 0; index < 3; ++index)
    {
        if (s_pAllocators[index] && s_pAllocators[index]->isAllocated(ptr))
        {
            s_pAllocators[index]->free(ptr);
            return;
        }
    }

    // Otherwise, if it belongs to the HeapManager, free it there
    if (HeapManagerProxy::Contains(s_pHeapManager, ptr))
    {
        HeapManagerProxy::free(s_pHeapManager, ptr);
        return;
    }

    _aligned_free(ptr);
}

// Replacement for malloc
void* __cdecl malloc(size_t sizeRequest)
{
    // Use our FixedSizeAllocators if appropriate
    if ((sizeRequest <= 16) && s_pAllocators[0])
        return s_pAllocators[0]->alloc();
    if ((sizeRequest <= 32) && s_pAllocators[1])
        return s_pAllocators[1]->alloc();
    if ((sizeRequest <= 96) && s_pAllocators[2])
        return s_pAllocators[2]->alloc();

    // Otherwise, go through the main HeapManager
    return s_pHeapManager->alloc(sizeRequest);
}

// Replacement for free
void __cdecl free(void* ptr)
{
    // Check if ptr came from a FixedSizeAllocator
    for (int i = 0; i < 3; ++i)
    {
        if (s_pAllocators[i] && s_pAllocators[i]->isAllocated(ptr))
        {
            s_pAllocators[i]->free(ptr);
            return;
        }
    }
    // If not found in an FSA, free via HeapManager
    HeapManagerProxy::free(s_pHeapManager, ptr);
}
