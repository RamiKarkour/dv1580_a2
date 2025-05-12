
#include "memory_manager.h"
#include "linked_list.h"
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

static pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_list(Element** head, size_t pool_size) {
    pthread_mutex_lock(&list_mutex);
    mem_init(pool_size);
    *head = NULL;
    pthread_mutex_unlock(&list_mutex);
}

void append(Element** head, uint16_t value) {
    pthread_mutex_lock(&list_mutex);

    Element* node = (Element*)mem_alloc(sizeof(Element));
    if (!node) {
        printf("Failed to allocate new node.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }
    node->value = value;
    node->next = NULL;

    if (*head == NULL) {
        *head = node;
    } else {
        Element* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = node;
    }

    pthread_mutex_unlock(&list_mutex);
}

void insert_after(Element* node, uint16_t value) {
    pthread_mutex_lock(&list_mutex);

    if (!node) {
        printf("Cannot insert after a NULL node.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Element* new_node = (Element*)mem_alloc(sizeof(Element));
    if (!new_node) {
        printf("Allocation failed.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    new_node->value = value;
    new_node->next = node->next;
    node->next = new_node;

    pthread_mutex_unlock(&list_mutex);
}

void insert_before(Element** head, Element* node, uint16_t value) {
    pthread_mutex_lock(&list_mutex);

    if (!head || !*head || !node) {
        printf("Invalid input.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Element* new_node = (Element*)mem_alloc(sizeof(Element));
    if (!new_node) {
        printf("Allocation failed.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }
    new_node->value = value;

    if (*head == node) {
        new_node->next = *head;
        *head = new_node;
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Element* current = *head;
    while (current && current->next != node) {
        current = current->next;
    }

    if (!current) {
        mem_free(new_node);
        printf("Target node not found.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    new_node->next = node;
    current->next = new_node;

    pthread_mutex_unlock(&list_mutex);
}

void remove_element(Element** head, uint16_t value) {
    pthread_mutex_lock(&list_mutex);

    if (!head || !*head) {
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Element* current = *head;
    Element* previous = NULL;

    if (current->value == value) {
        *head = current->next;
        mem_free(current);
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    while (current && current->value != value) {
        previous = current;
        current = current->next;
    }

    if (!current) {
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    previous->next = current->next;
    mem_free(current);

    pthread_mutex_unlock(&list_mutex);
}

Element* find(Element** head, uint16_t value) {
    pthread_mutex_lock(&list_mutex);

    Element* current = *head;
    while (current) {
        if (current->value == value) {
            pthread_mutex_unlock(&list_mutex);
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&list_mutex);
    return NULL;
}

void print_list(Element** head) {
    pthread_mutex_lock(&list_mutex);

    Element* current = *head;
    printf("[");
    while (current) {
        printf("%d", current->value);
        if (current->next) printf(", ");
        current = current->next;
    }
    printf("]\n");

    pthread_mutex_unlock(&list_mutex);
}

void print_range(Element** head, Element* start, Element* end) {
    pthread_mutex_lock(&list_mutex);

    Element* current = *head;
    if (!start) start = *head;

    while (current && current != start) {
        current = current->next;
    }

    printf("[");
    bool first = true;
    while (current) {
        if (!first) printf(", ");
        printf("%d", current->value);
        if (current == end) break;
        first = false;
        current = current->next;
    }
    printf("]\n");

    pthread_mutex_unlock(&list_mutex);
}

int count_elements(Element** head) {
    pthread_mutex_lock(&list_mutex);

    int count = 0;
    Element* current = *head;
    while (current) {
        count++;
        current = current->next;
    }

    pthread_mutex_unlock(&list_mutex);
    return count;
}

void clear_list(Element** head) {
    pthread_mutex_lock(&list_mutex);

    Element* current = *head;
    while (current) {
        Element* next = current->next;
        mem_free(current);
        current = next;
    }

    *head = NULL;
    mem_deinit();

    pthread_mutex_unlock(&list_mutex);
}
