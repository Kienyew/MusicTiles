#ifndef __H_LIST
#include <stddef.h>

typedef struct List
{
	struct ListNode* head;
	struct ListNode* tail;
	size_t size;
} List;

typedef struct ListNode
{
	struct ListNode* prev;
	struct ListNode* next;
	void* data;
} ListNode;

void list_append_tail(List * list, const void* data, size_t data_size);
void* list_pop_head(List * list);
void* list_pop_node(List * list, ListNode * target);
void list_clear(List * list, void(*free_function)(void*));

#endif // __H_LIST
