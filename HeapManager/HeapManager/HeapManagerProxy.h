#pragma once

#include "HeapManager.h"
#include <iostream>

namespace HeapManagerProxy 
{
    HeapManager* CreateHeapManager(void* HeapMemory, size_t HeapSize, size_t numDescriptors) 
    {
        return new HeapManager(HeapMemory, HeapSize, numDescriptors);
    }

    void Destroy(HeapManager* pHeapManager) 
    {
        delete pHeapManager;
    }

    void* Alloc(HeapManager* pHeapManager, size_t Size) 
    {
        return pHeapManager->alloc(Size);
    }

    void* Alloc(HeapManager* pHeapManager, size_t Size, unsigned int Alignment) 
    {
        return pHeapManager->alloc(Size, Alignment);
    }

    bool free(HeapManager* pHeapManager, void* ptr) 
    {
        return pHeapManager->Free(ptr);
    }

    void collect(HeapManager* pHeapManager) 
    {
        pHeapManager->Collect();
    }

    size_t GetLargestFreeBlock(HeapManager* pHeapManager) 
    {
        return pHeapManager->GetLargestFreeBlock();
    }

    bool Contains(HeapManager* pHeapManager, void* ptr) 
    {
        return pHeapManager->Contains(ptr);
    }

    bool IsAllocated(HeapManager* pHeapManager, void* ptr) 
    {
        return pHeapManager->IsAllocated(ptr);
    }

    void ShowFreeBlocks(HeapManager* pHeapManager) 
    {
        pHeapManager->ShowFreeBlocks();
    }

    void ShowOutstandingAllocations(HeapManager* pHeapManager) 
    {
        pHeapManager->ShowOutstandingAllocations();
    }
}
