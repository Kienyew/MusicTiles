#include "list.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


// data will be copied.
void list_append_tail(List * list, const void* data, size_t data_size)
{
	if (list->size == 0)
	{
		assert(list->head == NULL && list->tail == NULL);
		list->head = list->tail = calloc(1, sizeof(ListNode));
		list->tail->data = memcpy(malloc(data_size), data, data_size);
	}
	else
	{
		assert(list->tail != NULL && list->tail->prev == NULL);
		list->tail->prev = calloc(1, sizeof(ListNode));
		list->tail->prev->data = memcpy(malloc(data_size), data, data_size);
		list->tail->prev->next = list->tail;
		list->tail = list->tail->prev;
	}

	list->size++;
}

// User is responsible to free the returned pointer by free()
void* list_pop_head(List * list)
{
	assert(list->head != NULL);
	assert(list->head->next == NULL);

	void* ret = list->head->data;
	ListNode * old_head = list->head;
	if (list->size == 1)
	{
		list->head = list->tail = NULL;
	}
	else
	{
		list->head = old_head->prev;
		list->head->next = NULL;
	}
	free(old_head);
	list->size--;
	return ret;
}

// User is responsible to free the returned pointer
void* list_pop_node(List * list, ListNode * target)
{
	assert(list != NULL && target != NULL);
	assert(list->size != 0 && "What do you want to pop from an empty list?");

	if (target->prev) target->prev->next = target->next;
	if (target->next) target->next->prev = target->prev;
	if (target == list->head) list->head = target->prev;
	if (target == list->tail) list->tail = target->next;

	list->size--;
	void* ret = target->data;
	free(target);
	return ret;
}

void list_clear(List * list, void(*free_function)(void*))
{
	if (list == NULL) return;
	while (list->size > 0)
		free_function(list_pop_head(list));
}
