
#ifndef linked_list_h
#define linked_list_h

#include <stdint.h>

typedef struct Element {
    uint16_t value;
    struct Element* next;
} Element;

void init_list(Element** head, size_t pool_size);
void append(Element** head, uint16_t value);
void insert_after(Element* node, uint16_t value);
void insert_before(Element** head, Element* node, uint16_t value);
void remove_element(Element** head, uint16_t value);
Element* find(Element** head, uint16_t value);
void print_list(Element** head);
void print_range(Element** head, Element* start, Element* end);
int count_elements(Element** head);
void clear_list(Element** head);

#endif // CUSTOM_LINKED_LIST_H
