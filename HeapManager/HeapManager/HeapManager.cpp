#include "HeapManager.h"
#include <iostream>
#include <cstdio>
#include <Windows.h>
#include <assert.h>

using namespace std;

// Constructor
HeapManager::HeapManager(void* pHeapMem, size_t HeapSize, size_t NumDescriptors)
    : m_pHeapMemory(pHeapMem), m_HeapSize(HeapSize), m_FreeList(nullptr)
{
    if (pHeapMem == nullptr) {
        printf("Constructor Error: Provided heap memory pointer was null.\n");
        return;  // Early out if memory is invalid
    }

    if (HeapSize == 0) {
        printf("Constructor Error: Heap size of zero is invalid.\n");
        return;  // Early out if heap size is invalid
    }

    printf("HeapManager ctor invoked. MemoryStart: %p, Size: %zu bytes\n", pHeapMem, HeapSize);

    // Initialize the free list at the start of the provided memory
    m_FreeList = reinterpret_cast<MemoryBlock*>(pHeapMem);
    m_FreeList->Size = HeapSize - sizeof(MemoryBlock);
    m_FreeList->IsFree = true;
    m_FreeList->Next = nullptr;
    m_FreeList->Prev = nullptr;

    printf("HeapManager successfully initialized.\n");
}

// Contains (checks if a pointer is within the heap range)
bool HeapManager::Contains(void* ptr)
{
    uintptr_t start = reinterpret_cast<uintptr_t>(m_pHeapMemory);
    uintptr_t end = start + m_HeapSize;
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    return (address >= start) && (address < end);
}

// IsAllocated (checks if the pointer is currently allocated)
bool HeapManager::IsAllocated(void* ptr)
{
    MemoryBlock* pBlock = reinterpret_cast<MemoryBlock*>(reinterpret_cast<char*>(ptr) - sizeof(MemoryBlock));
    return !pBlock->IsFree;
}

// GetLargestFreeBlock
size_t HeapManager::GetLargestFreeBlock()
{
    size_t maxSize = 0;
    MemoryBlock* pBlock = m_FreeList;
    while (pBlock)
    {
        if (pBlock->IsFree && pBlock->Size > maxSize)
        {
            maxSize = pBlock->Size;
        }
        pBlock = pBlock->Next;
    }
    return maxSize;
}

// ShowFreeBlocks
void HeapManager::ShowFreeBlocks()
{
    MemoryBlock* pBlock = m_FreeList;
    std::cout << "Listing Free Blocks:\n";
    while (pBlock)
    {
        if (pBlock->IsFree)
        {
            std::cout << " Free Block -> Size: " << pBlock->Size << "\n";
        }
        pBlock = pBlock->Next;
    }
}

// ShowOutstandingAllocations
void HeapManager::ShowOutstandingAllocations()
{
    MemoryBlock* pBlock = m_FreeList;
    std::cout << "Outstanding (Allocated) Blocks:\n";
    while (pBlock)
    {
        if (!pBlock->IsFree)
        {
            std::cout << " Allocated Block -> Size: " << pBlock->Size << "\n";
        }
        pBlock = pBlock->Next;
    }
}

// DisplayHeap (prints out all blocks, free or allocated)
void HeapManager::DisplayHeap()
{
    MemoryBlock* pBlock = m_FreeList;
    cout << "Heap Status Overview:" << endl;
    while (pBlock)
    {
        cout << " Block @ " << pBlock
            << " | Size: " << pBlock->Size
            << " | IsFree: " << (pBlock->IsFree ? "Yes" : "No")
            << endl;
        pBlock = pBlock->Next;
    }
}

// alloc (un-aligned version)
void* HeapManager::alloc(size_t Size)
{
    if (this == nullptr) {
        printf("Error: HeapManager pointer is null inside alloc.\n");
        return nullptr;
    }

    MemoryBlock* pBlock = m_FreeList;
    while (pBlock)
    {
        if (pBlock->IsFree && pBlock->Size >= Size)
        {
            SplitBlock(pBlock, Size);
            pBlock->IsFree = false;
            return reinterpret_cast<void*>(reinterpret_cast<char*>(pBlock + 1));
        }
        pBlock = pBlock->Next;
    }

    printf("HeapManager::alloc failed: No suitable free block for requested size %zu\n", Size);
    return nullptr;
}

// alloc (aligned version)
void* HeapManager::alloc(size_t Size, unsigned int Alignment)
{
    if (this == nullptr) {
        printf("Error: HeapManager pointer is null inside alloc (aligned).\n");
        return nullptr;
    }

    MemoryBlock* pBlock = m_FreeList;
    while (pBlock)
    {
        if (pBlock->IsFree && pBlock->Size >= Size)
        {
            uintptr_t baseAddr = reinterpret_cast<uintptr_t>(pBlock + 1);
            uintptr_t alignedAddress = (baseAddr + Alignment - 1) & ~(Alignment - 1);
            size_t padding = alignedAddress - baseAddr;
            size_t totalSizeNeeded = Size + padding;

            if (pBlock->Size >= totalSizeNeeded)
            {
                // If we need some alignment offset, split off that portion
                if (padding > 0)
                {
                    SplitBlock(pBlock, padding);
                    pBlock = pBlock->Next; // Move to the newly split block
                }

                SplitBlock(pBlock, Size);
                pBlock->IsFree = false;

                return reinterpret_cast<void*>(alignedAddress);
            }
        }
        pBlock = pBlock->Next;
    }

    printf("HeapManager::alloc (aligned) failed: No free block for size %zu\n", Size);
    return nullptr;
}

// Free
bool HeapManager::Free(void* ptr)
{
    if (!ptr)
        return false;

    // Identify block metadata
    MemoryBlock* pBlock = reinterpret_cast<MemoryBlock*>(reinterpret_cast<char*>(ptr) - sizeof(MemoryBlock));
    if (!Contains(pBlock))
        return false;

    pBlock->IsFree = true;
    Coalesce(pBlock);  // Attempt to merge with neighboring free blocks

    return true;
}

// Collect (calls Coalesce on all free blocks)
void HeapManager::Collect()
{
    MemoryBlock* pBlock = m_FreeList;
    while (pBlock)
    {
        if (pBlock->IsFree)
        {
            Coalesce(pBlock);
        }
        pBlock = pBlock->Next;
    }
}

// Coalesce (merges adjacent free blocks into a single bigger block)
void HeapManager::Coalesce(MemoryBlock* pBlock)
{
    if (!Contains(pBlock))
        return;

    // Merge with the next block if it's free
    if (pBlock->Next && Contains(pBlock->Next) && pBlock->Next->IsFree)
    {
        pBlock->Size += sizeof(MemoryBlock) + pBlock->Next->Size;
        pBlock->Next = pBlock->Next->Next;

        if (pBlock->Next)
        {
            pBlock->Next->Prev = pBlock;
        }
    }

    // Merge with the previous block if it's free
    if (pBlock->Prev && Contains(pBlock->Prev) && pBlock->Prev->IsFree)
    {
        pBlock->Prev->Size += sizeof(MemoryBlock) + pBlock->Size;
        pBlock->Prev->Next = pBlock->Next;

        if (pBlock->Next)
        {
            pBlock->Next->Prev = pBlock->Prev;
        }
    }
}

// SplitBlock (creates a new block if the free block is larger than requested size)
void HeapManager::SplitBlock(MemoryBlock* pBlock, size_t requiredSize)
{
    if (pBlock->Size > requiredSize + sizeof(MemoryBlock) + s_MinumumToLeave)
    {
        MemoryBlock* pNewBlock = reinterpret_cast<MemoryBlock*>(
            reinterpret_cast<char*>(pBlock + 1) + requiredSize);

        // Check if the new block pointer is valid and inside the heap
        if (!Contains(pNewBlock) ||
            reinterpret_cast<uintptr_t>(pNewBlock) >= reinterpret_cast<uintptr_t>(m_pHeapMemory) + m_HeapSize)
        {
            return; // Do not split if new block is out of heap bounds
        }

        pNewBlock->Size = pBlock->Size - requiredSize - sizeof(MemoryBlock);
        pNewBlock->IsFree = true;
        pNewBlock->Next = pBlock->Next;
        pNewBlock->Prev = pBlock;

        if (pBlock->Next)
        {
            pBlock->Next->Prev = pNewBlock;
        }

        pBlock->Next = pNewBlock;
        pBlock->Size = requiredSize;
    }
}

// Alignment (utility method for adjusting addresses to meet alignment requirements)
void* HeapManager::Alignment(void* Address, unsigned int Alignment, size_t& Padding)
{
    uintptr_t baseAddr = reinterpret_cast<uintptr_t>(Address);
    uintptr_t misalignment = baseAddr % Alignment;
    Padding = (misalignment == 0) ? 0 : (Alignment - misalignment);
    return reinterpret_cast<void*>(baseAddr + Padding);
}
