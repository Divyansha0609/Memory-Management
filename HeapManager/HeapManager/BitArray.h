#pragma once

#include <cstddef>
#include <cstdint>

class BitArray 
{
    private:
        uint8_t* m_BitArray;
        size_t m_Size;
        size_t m_NumElements;

    public:
        BitArray(size_t i_NumBits);
        ~BitArray();

        bool GetFirstClearBit(size_t& o_Index) const;
        bool IsBitSet(size_t i_Index) const;
        void SetBit(size_t i_Index);
        void ClearBit(size_t i_Index);
        void ClearAll();

        size_t GetBitCount() const;
};

