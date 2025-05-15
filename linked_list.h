
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdint.h>
#include <stddef.h>


typedef struct Node  {
    uint16_t data;
    struct Node * next;
} Node ;

void list_init(Node ** head, size_t pool_size);
void list_insert(Node ** head, uint16_t data);
void list_insert_after(Node * node, uint16_t data);
void list_insert_before(Node ** head, Node * node, uint16_t data);
void list_delete(Node ** head, uint16_t data);
Node * list_search(Node ** head, uint16_t data);
void list_display(Node ** head);
void list_display_range(Node ** head, Node * start, Node * end);
int list_count_nodes(Node ** head);
void list_cleanup(Node ** head);

#endif // LINKED_LIST_H
