#include "BitArray.h"
#include <cassert>
#include <cstring>
#include <intrin.h>

BitArray::BitArray(size_t totalBits)
    : m_Size(totalBits),
    m_NumElements((totalBits + 7) / 8)
{
    m_BitArray = new uint8_t[m_NumElements];
    ClearAll(); // Set all bits to 0
}

BitArray::~BitArray()
{
    delete[] m_BitArray;
}

// Returns the total number of bits this BitArray manages
size_t BitArray::GetBitCount() const
{
    return m_Size;
}

// Checks if a particular bit is set (1)
bool BitArray::IsBitSet(size_t index) const
{
    assert(index < m_Size);

    size_t byteIndex = index / 8;
    size_t bitOffset = index % 8;

    return (m_BitArray[byteIndex] & (1 << bitOffset)) != 0;
}

// Sets (turns on) a particular bit
void BitArray::SetBit(size_t index)
{
    assert(index < m_Size);

    size_t byteIndex = index / 8;
    size_t bitOffset = index % 8;

    m_BitArray[byteIndex] |= (1 << bitOffset);
}

// Clears (turns off) a particular bit
void BitArray::ClearBit(size_t index)
{
    assert(index < m_Size);

    size_t byteIndex = index / 8;
    size_t bitOffset = index % 8;

    m_BitArray[byteIndex] &= ~(1 << bitOffset);
}

// Clears all bits in the array
void BitArray::ClearAll()
{
    std::memset(m_BitArray, 0, m_NumElements);
}

// Finds the first clear (0) bit and returns its index if found
bool BitArray::GetFirstClearBit(size_t& outIndex) const
{
    for (size_t byteIndex = 0; byteIndex < m_NumElements; ++byteIndex)
    {
        uint8_t current = m_BitArray[byteIndex];

        // Only check if there's at least one 0 bit in the byte
        if (current != 0xFF)
        {
            unsigned long bitIndex;
            // ~current flips the bits so a 0 bit becomes 1
            if (_BitScanForward(&bitIndex, ~current))
            {
                outIndex = (byteIndex * 8) + bitIndex;
                if (outIndex < m_Size)
                    return true;
            }
        }
    }
    return false;
}
