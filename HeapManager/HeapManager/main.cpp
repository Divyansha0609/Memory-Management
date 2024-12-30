#include <Windows.h>
#include "MemorySystem.h"

#include <assert.h>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <ctime>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

// Forward declaration of our test function
bool RunMemorySystemTests();

int main(int argumentCount, char** argumentValues)
{
    // We can seed our RNG here to ensure different random outcomes on each run
    srand(static_cast<unsigned int>(time(nullptr)));

    // Size of our main heap in bytes (1 MB)
    const size_t memHeapSize = 1024 * 1024;

    // Descriptor count (if you use a descriptor-based HeapManager)
    const unsigned int descriptorCount = 2048;

    // Allocate memory for the heap
    void* pMainHeapMemory = HeapAlloc(GetProcessHeap(), 0, memHeapSize);
    assert(pMainHeapMemory);

    // Initialize our custom MemorySystem (HeapManager + possible FixedSizeAllocators)
    InitializeMemorySystem(pMainHeapMemory, memHeapSize, descriptorCount);

    // Run the memory test
    bool testOutcome = RunMemorySystemTests();
    assert(testOutcome);

    // Clean up our Memory System
    DestroyMemorySystem();

    // Free the raw heap memory
    HeapFree(GetProcessHeap(), 0, pMainHeapMemory);

#if defined(_DEBUG)
    // Report memory leaks in Debug mode
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}

bool RunMemorySystemTests()
{
    // We'll cap our maximum random allocations
    const size_t maxPossibleAllocs = 10 * 1024;
    std::vector<void*> allocatedBlocks;

    long allocationCount = 0;
    long freeCount = 0;
    long collectCount = 0;

    size_t totalMemoryAllocated = 0;

    // Reserve space to avoid vector expansions causing spurious allocations
    allocatedBlocks.reserve(maxPossibleAllocs);

    // Randomly allocate memory chunks (up to 1024 bytes) until we can't
    do
    {
        const size_t maxChunkSize = 1024;
        size_t currentAllocSize = 1 + (rand() & (maxChunkSize - 1));

        // Attempt to allocate via malloc (replace with your custom allocation if needed)
        void* pCurrentBlock = malloc(currentAllocSize);

        // If we ran out of memory, attempt to collect (garbage collect)
        if (pCurrentBlock == nullptr)
        {
            Collect();  // Attempt to free up space
            pCurrentBlock = malloc(currentAllocSize);

            // If still no luck, break out of the loop
            if (pCurrentBlock == nullptr)
                break;
        }

        allocatedBlocks.push_back(pCurrentBlock);
        ++allocationCount;
        totalMemoryAllocated += currentAllocSize;

        // Periodically free or collect to simulate real usage
        const unsigned int freeChance = 0x07;
        const unsigned int collectChance = 0x07;
        bool shouldFreeBlock = (!allocatedBlocks.empty() && (rand() % freeChance == 0));
        bool shouldCollectNow = (rand() % collectChance == 0);

        if (shouldFreeBlock)
        {
            // Free the most recently allocated block
            void* pMemToRelease = allocatedBlocks.back();
            allocatedBlocks.pop_back();

            free(pMemToRelease);
            ++freeCount;
        }
        else if (shouldCollectNow)
        {
            Collect();
            ++collectCount;
        }

    } while (allocationCount < maxPossibleAllocs);

    // Free the allocated blocks in a random order
    if (!allocatedBlocks.empty())
    {
        // Shuffle the pointers
        std::random_shuffle(allocatedBlocks.begin(), allocatedBlocks.end());

        // Release them all
        while (!allocatedBlocks.empty())
        {
            void* pMemToRelease = allocatedBlocks.back();
            allocatedBlocks.pop_back();

            // Using delete[] here (as in the original code)
            delete[] static_cast<char*>(pMemToRelease);
        }

        // Perform a final collection to ensure big contiguous block
        Collect();

        // Test if we can allocate a large block now
        void* pLargeTestBlock = malloc(totalMemoryAllocated / 2);

        if (pLargeTestBlock)
        {
            free(pLargeTestBlock);
        }
        else
        {
            // If this fails, something is wrong
            return false;
        }
    }
    else
    {
        return false;
    }

    // Finally, verify new[]/delete[] go through the allocator
    char* testNewDelete = new char[1024];
    delete[] testNewDelete;

    return true;
}