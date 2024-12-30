#pragma once

#include "cstddef"

struct MemoryBlock {
    size_t Size;
    bool IsFree;
    MemoryBlock* Next;
    MemoryBlock* Prev;
};

class HeapManager
{
private:
    void* m_pHeapMemory;
    size_t m_HeapSize;
    MemoryBlock* m_FreeList;

public:
    static const size_t s_MinumumToLeave = 16;

    HeapManager(void* HeapMemory, size_t HeapSize, size_t NumDescriptors);
    void* alloc(size_t Size);
    void* alloc(size_t Size, unsigned int Alignment);
    void* Alignment(void* Address, unsigned int Alignment, size_t& Padding);
    void SplitBlock(MemoryBlock* Block, size_t Size);
    void DisplayHeap();
    void Collect();
    bool Free(void* ptr);
    void Coalesce(MemoryBlock* Block);
    size_t GetLargestFreeBlock();
    bool Contains(void* ptr);
    bool IsAllocated(void* ptr);
    void ShowFreeBlocks();
    void ShowOutstandingAllocations();
};