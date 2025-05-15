#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
/*adsadasdadas*/
typedef struct Block {
    void* ptr;
    size_t size;
    int free;
    struct Block* next;
} Block;

static void* memory_pool = NULL;
static Block* block_list = NULL;
static size_t memory_pool_size = 0;
static size_t total_used = 0;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void mem_init(size_t size) {
    pthread_mutex_lock(&lock);

    memory_pool = malloc(size);
    if (!memory_pool) {
        fprintf(stderr, "Failed to allocate memory pool\n");
        pthread_mutex_unlock(&lock);
        return;
    }

    block_list = malloc(sizeof(Block));
    if (!block_list) {
        free(memory_pool);
        memory_pool = NULL;
        fprintf(stderr, "Failed to allocate metadata block\n");
        pthread_mutex_unlock(&lock);
        return;
    }

    block_list->ptr = memory_pool;
    block_list->size = size;
    block_list->free = 1;
    block_list->next = NULL;

    memory_pool_size = size;
    total_used = size + sizeof(Block);

    pthread_mutex_unlock(&lock);
}

void* mem_alloc(size_t size) {
    pthread_mutex_lock(&lock);

    Block* current = block_list;

    // First-fit strategy
    while (current) {
        if (current->free && current->size >= size) {
            size_t leftover = current->size - size;

            // Only split if it's worth it
            if (leftover >= sizeof(Block) + 16) {
                // Prevent metadata explosion
                if (total_used + sizeof(Block) > memory_pool_size + memory_pool_size / 5) {
                    current->free = 0;  // Use whole block without split
                    pthread_mutex_unlock(&lock);
                    return current->ptr;
                }

                Block* new_block = malloc(sizeof(Block));
                if (!new_block) {
                    current->free = 0;  // Use whole block without split
                    pthread_mutex_unlock(&lock);
                    return current->ptr;
                }

                total_used += sizeof(Block);

                new_block->ptr = (char*)current->ptr + size;
                new_block->size = leftover;
                new_block->free = 1;
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }

            current->free = 0;
            pthread_mutex_unlock(&lock);
            return current->ptr;
        }

        current = current->next;
    }

    pthread_mutex_unlock(&lock);
    return NULL;
}


void mem_free(void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&lock);

    Block* current = block_list;

    while (current) {
        if (current->ptr == ptr) {
            current->free = 1;

            Block* next = current->next;
            if (next && next->free) {
                current->size += sizeof(Block) + next->size;
                current->next = next->next;
                free(next);
                total_used -= sizeof(Block);
            }

            break;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&lock);
}

void* mem_resize(void* ptr, size_t size) {
    if (!ptr) return mem_alloc(size);

    pthread_mutex_lock(&lock);

    Block* current = block_list;
    while (current) {
        if (current->ptr == ptr) {
            if (current->size >= size) {
                pthread_mutex_unlock(&lock);
                return ptr;
            } else {
                void* new_ptr = mem_alloc(size);
                if (new_ptr) {
                    memcpy(new_ptr, ptr, current->size);
                    mem_free(ptr);
                }
                pthread_mutex_unlock(&lock);
                return new_ptr;
            }
        }
        current = current->next;
    }

    pthread_mutex_unlock(&lock);
    return NULL;
}

void mem_deinit() {
    pthread_mutex_lock(&lock);

    Block* current = block_list;
    while (current) {
        Block* next = current->next;
        free(current);
        current = next;
    }

    free(memory_pool);
    memory_pool = NULL;
    block_list = NULL;
    memory_pool_size = 0;
    total_used = 0;

    pthread_mutex_unlock(&lock);
    pthread_mutex_destroy(&lock);
}
