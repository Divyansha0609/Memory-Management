#pragma once
#include "BitArray.h"

class FixedSizeAllocator 
{
private:
    size_t m_BlockSize;
    size_t m_NumBlocks;
    void* m_pMemory;
    BitArray m_BitArray;

public:
    FixedSizeAllocator(size_t i_BlockSize, size_t i_NumBlocks, void* i_pMemory);
    ~FixedSizeAllocator();

    void* alloc();
    void free(void* ptr);
    bool isAllocated(void* ptr) const;
};
