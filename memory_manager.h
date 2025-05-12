
#ifndef memory_manager.h
#define memory_manager.h

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

void mem_init(size_t size);
void* mem_alloc(size_t size);
void mem_free(void* block);
void* mem_resize(void* block, size_t size);
void mem_deinit();

#endif // CUSTOM_MEMORY_MANAGER_H
