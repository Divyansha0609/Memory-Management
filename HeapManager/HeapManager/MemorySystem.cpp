#include "MemorySystem.h"
#include "HeapManager.h"
#include "FixedSizeAllocator.h"
#include <cstdio>

// Global variables for memory system
HeapManager* s_pHeapManager = nullptr;
FixedSizeAllocator* s_pAllocators[3] = { nullptr, nullptr, nullptr };

bool InitializeMemorySystem(void* i_pHeapMemory, size_t i_sizeHeapMemory, unsigned int i_OptionalNumDescriptors)
{
    printf("Starting Memory System initialization...\n");

    // Check the memory pointer and size
    if (!i_pHeapMemory || i_sizeHeapMemory == 0)
    {
        printf("Error: Invalid memory pointer or heap size specified.\n");
        return false;
    }

    // Initialize HeapManager
    s_pHeapManager = new HeapManager(i_pHeapMemory, i_sizeHeapMemory, i_OptionalNumDescriptors);
    if (!s_pHeapManager)
    {
        printf("Error: HeapManager creation failed.\n");
        return false;
    }

    // Log successful creation
    printf("HeapManager created at address: %p\n", s_pHeapManager);

    // Allocate memory for FixedSizeAllocators
    void* blockMemory = s_pHeapManager->alloc(16 * 100);
    if (!blockMemory)
    {
        printf("Error: Unable to allocate memory for 16-byte FixedSizeAllocator.\n");
        return false;
    }
    s_pAllocators[0] = new FixedSizeAllocator(16, 100, blockMemory);

    blockMemory = s_pHeapManager->alloc(32 * 200);
    if (!blockMemory)
    {
        printf("Error: Unable to allocate memory for 32-byte FixedSizeAllocator.\n");
        return false;
    }
    s_pAllocators[1] = new FixedSizeAllocator(32, 200, blockMemory);

    blockMemory = s_pHeapManager->alloc(96 * 400);
    if (!blockMemory)
    {
        printf("Error: Unable to allocate memory for 96-byte FixedSizeAllocator.\n");
        return false;
    }
    s_pAllocators[2] = new FixedSizeAllocator(96, 400, blockMemory);

    printf("Memory System initialization complete.\n");
    return true;
}

void Collect()
{
    // Trigger a collection in the HeapManager
    s_pHeapManager->Collect();
}

void DestroyMemorySystem()
{
    printf("Starting Memory System shutdown...\n");

    // Release all FixedSizeAllocators
    for (auto& allocator : s_pAllocators)
    {
        delete allocator;
        allocator = nullptr;
    }

    // Release the HeapManager
    if (s_pHeapManager)
    {
        delete s_pHeapManager;
        s_pHeapManager = nullptr;
        printf("HeapManager successfully destroyed.\n");
    }
    else
    {
        printf("No valid HeapManager found to destroy.\n");
    }
}
