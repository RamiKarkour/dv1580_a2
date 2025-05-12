#include "memory_manager.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

typedef struct MemUnit {
    void* start;
    size_t size;
    bool is_free;
    struct MemUnit* next;
} MemUnit;

static void* heap_base = NULL;
static MemUnit* unit_list = NULL;
static size_t max_heap_size = 0;
static size_t total_used_malloc = 0;
static pthread_mutex_t heap_lock = PTHREAD_MUTEX_INITIALIZER;

void mem_init(size_t size) {
    pthread_mutex_lock(&heap_lock);

    max_heap_size = size;
    total_used_malloc = 0;

    heap_base = malloc(size);
    if (!heap_base) {
        fprintf(stderr, "Heap allocation failed.\n");
        pthread_mutex_unlock(&heap_lock);
        return;
    }
    total_used_malloc += size;

    unit_list = malloc(sizeof(MemUnit));
    if (!unit_list) {
        free(heap_base);
        total_used_malloc -= size;
        fprintf(stderr, "Metadata allocation failed.\n");
        pthread_mutex_unlock(&heap_lock);
        return;
    }
    total_used_malloc += sizeof(MemUnit);

    unit_list->start = heap_base;
    unit_list->size = size;
    unit_list->is_free = true;
    unit_list->next = NULL;

    pthread_mutex_unlock(&heap_lock);
}

void* mem_alloc(size_t size) {
    pthread_mutex_lock(&heap_lock);
    MemUnit* best_fit = NULL;
    MemUnit* current = unit_list;

    while (current) {
        if (current->is_free && current->size >= size) {
            if (!best_fit || current->size < best_fit->size) {
                best_fit = current;
            }
        }
        current = current->next;
    }

    if (!best_fit) {
        pthread_mutex_unlock(&heap_lock);
        return NULL;
    }

    size_t leftover = best_fit->size - size;
    if (leftover > sizeof(MemUnit)) {
        if (total_used_malloc + sizeof(MemUnit) > max_heap_size + max_heap_size / 5) {
            pthread_mutex_unlock(&heap_lock);
            return NULL;
        }

        MemUnit* new_unit = malloc(sizeof(MemUnit));
        if (!new_unit) {
            pthread_mutex_unlock(&heap_lock);
            return NULL;
        }

        total_used_malloc += sizeof(MemUnit);
        new_unit->start = (char*)best_fit->start + size;
        new_unit->size = leftover;
        new_unit->is_free = true;
        new_unit->next = best_fit->next;

        best_fit->size = size;
        best_fit->next = new_unit;
    }

    best_fit->is_free = false;

    pthread_mutex_unlock(&heap_lock);
    return best_fit->start;
}

void mem_free(void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&heap_lock);
    MemUnit* current = unit_list;

    while (current) {
        if (current->start == ptr) {
            current->is_free = true;

            MemUnit* next_unit = current->next;
            if (next_unit && next_unit->is_free) {
                current->size += next_unit->size;
                current->next = next_unit->next;
                free(next_unit);
                total_used_malloc -= sizeof(MemUnit);
            }
            break;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&heap_lock);
}

void* mem_resize(void* ptr, size_t new_size) {
    if (!ptr) return mem_alloc(new_size);

    pthread_mutex_lock(&heap_lock);

    MemUnit* current = unit_list;
    while (current) {
        if (current->start == ptr) {
            if (current->size >= new_size) {
                pthread_mutex_unlock(&heap_lock);
                return ptr;
            } else {
                void* new_block = mem_alloc(new_size);
                if (new_block) {
                    memcpy(new_block, ptr, current->size);
                    mem_free(ptr);
                }
                pthread_mutex_unlock(&heap_lock);
                return new_block;
            }
        }
        current = current->next;
    }

    pthread_mutex_unlock(&heap_lock);
    return NULL;
}

void mem_deinit() {
    pthread_mutex_lock(&heap_lock);

    MemUnit* current = unit_list;
    while (current) {
        MemUnit* next = current->next;
        free(current);
        total_used_malloc -= sizeof(MemUnit);
        current = next;
    }

    free(heap_base);
    total_used_malloc -= max_heap_size;

    heap_base = NULL;
    unit_list = NULL;
    max_heap_size = 0;

    pthread_mutex_unlock(&heap_lock);
    pthread_mutex_destroy(&heap_lock);
}
