
#include "memory_manager.h"
#include "linked_list.h"
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

static pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

void list_init(Node ** head, size_t pool_size) {
    pthread_mutex_lock(&list_mutex);
    mem_init(pool_size);
    *head = NULL;
    pthread_mutex_unlock(&list_mutex);
}

void list_insert(Node ** head, uint16_t data) {
    pthread_mutex_lock(&list_mutex);

    Node * node = (Node *)mem_alloc(sizeof(Node ));
    if (!node) {
        printf("Failed to allocate new node.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }
    node->data = data;
    node->next = NULL;

    if (*head == NULL) {
        *head = node;
    } else {
        Node * current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = node;
    }

    pthread_mutex_unlock(&list_mutex);
}

void list_insert_after(Node * node, uint16_t data) {
    pthread_mutex_lock(&list_mutex);

    if (!node) {
        printf("Cannot insert after a NULL node.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Node * new_node = (Node *)mem_alloc(sizeof(Node ));
    if (!new_node) {
        printf("Allocation failed.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    new_node->data = data;
    new_node->next = node->next;
    node->next = new_node;

    pthread_mutex_unlock(&list_mutex);
}

void list_insert_before(Node ** head, Node * node, uint16_t data) {
    pthread_mutex_lock(&list_mutex);

    if (!head || !*head || !node) {
        printf("Invalid input.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Node * new_node = (Node *)mem_alloc(sizeof(Node ));
    if (!new_node) {
        printf("Allocation failed.\n");
        pthread_mutex_unlock(&list_mutex);
        return;
    }
    new_node->data = data;

    if (*head == node) {
        new_node->next = *head;
        *head = new_node;
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Node * current = *head;
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

void list_delete(Node ** head, uint16_t data) {
    pthread_mutex_lock(&list_mutex);

    if (!head || !*head) {
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Node * current = *head;
    Node * previous = NULL;

    if (current->data == data) {
        *head = current->next;
        mem_free(current);
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    while (current && current->data != data) {
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

Node * list_search(Node ** head, uint16_t data) {
    pthread_mutex_lock(&list_mutex);

    Node * current = *head;
    while (current) {
        if (current->data == data) {
            pthread_mutex_unlock(&list_mutex);
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&list_mutex);
    return NULL;
}

void list_display(Node ** head) {
    pthread_mutex_lock(&list_mutex);

    Node * current = *head;
    printf("[");
    while (current) {
        printf("%d", current->data);
        if (current->next) printf(", ");
        current = current->next;
    }
    printf("]\n");

    pthread_mutex_unlock(&list_mutex);
}

void list_display_range(Node ** head, Node * start, Node * end) {
    pthread_mutex_lock(&list_mutex);

    Node * current = *head;
    if (!start) start = *head;

    while (current && current != start) {
        current = current->next;
    }

    printf("[");
    bool first = true;
    while (current) {
        if (!first) printf(", ");
        printf("%d", current->data);
        if (current == end) break;
        first = false;
        current = current->next;
    }
    printf("]\n");

    pthread_mutex_unlock(&list_mutex);
}

int list_count_nodes(Node ** head) {
    pthread_mutex_lock(&list_mutex);

    int count = 0;
    Node * current = *head;
    while (current) {
        count++;
        current = current->next;
    }

    pthread_mutex_unlock(&list_mutex);
    return count;
}

void list_cleanup(Node ** head) {
    pthread_mutex_lock(&list_mutex);

    Node * current = *head;
    while (current) {
        Node * next = current->next;
        mem_free(current);
        current = next;
    }

    *head = NULL;
    mem_deinit();

    pthread_mutex_unlock(&list_mutex);
}
