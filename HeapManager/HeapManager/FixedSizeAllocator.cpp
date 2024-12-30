#include "FixedSizeAllocator.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

// Constructor
FixedSizeAllocator::FixedSizeAllocator(size_t blockSize, size_t blockCount, void* startMemory)
    : m_BlockSize(blockSize), m_NumBlocks(blockCount), m_pMemory(startMemory), m_BitArray(blockCount)
{
    assert(m_BlockSize > 0 && m_NumBlocks > 0 && m_pMemory != nullptr);
    m_BitArray.ClearAll();
    printf("FixedSizeAllocator constructed: BlockSize=%zu, NumBlocks=%zu\n", blockSize, blockCount);
}

// Destructor
FixedSizeAllocator::~FixedSizeAllocator()
{
#ifdef _DEBUG
    // Check for any allocated blocks that were not freed
    for (size_t i = 0; i < m_NumBlocks; ++i)
    {
        if (m_BitArray.IsBitSet(i))
        {
            printf("Warning: Potential memory leak at block %zu\n", i);
        }
    }
#endif
}

// Checks if a pointer belongs to a currently allocated block
bool FixedSizeAllocator::isAllocated(void* ptr) const
{
    uintptr_t startAddr = reinterpret_cast<uintptr_t>(m_pMemory);
    uintptr_t endAddr = startAddr + (m_BlockSize * m_NumBlocks);
    uintptr_t checkAddr = reinterpret_cast<uintptr_t>(ptr);

    // If pointer is out of range, it's definitely not allocated by this allocator
    if (checkAddr < startAddr || checkAddr >= endAddr)
        return false;

    // Compute index in the bit array
    size_t idx = (checkAddr - startAddr) / m_BlockSize;
    return m_BitArray.IsBitSet(idx);
}

// Allocates a free block, or returns nullptr if none are available
void* FixedSizeAllocator::alloc()
{
    size_t freeIndex;
    if (m_BitArray.GetFirstClearBit(freeIndex))
    {
        m_BitArray.SetBit(freeIndex);
        return static_cast<char*>(m_pMemory) + (freeIndex * m_BlockSize);
    }
    return nullptr; // No free blocks
}

// Frees a previously allocated block
void FixedSizeAllocator::free(void* ptr)
{
    uintptr_t startAddr = reinterpret_cast<uintptr_t>(m_pMemory);
    uintptr_t endAddr = startAddr + (m_BlockSize * m_NumBlocks);
    uintptr_t targetAddr = reinterpret_cast<uintptr_t>(ptr);

    assert(targetAddr >= startAddr && targetAddr < endAddr && "Pointer out of range for this FixedSizeAllocator");

    size_t blockIndex = (targetAddr - startAddr) / m_BlockSize;
    m_BitArray.ClearBit(blockIndex);
}
