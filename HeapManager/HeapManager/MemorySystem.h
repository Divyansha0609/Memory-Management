#pragma once

bool InitializeMemorySystem(void* i_pHeapMemory, size_t i_sizeHeapMemory, unsigned int i_OptionalNumDescriptors);
void Collect();
void DestroyMemorySystem();

void* __cdecl malloc(size_t i_size);
void  __cdecl free(void* i_ptr);
void* operator new(size_t i_size);
