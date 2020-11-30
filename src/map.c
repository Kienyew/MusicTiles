#include "map.h"
#include "list.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// initialize a map struct
void map_init(Map *map, size_t key_size, size_t value_size)
{
    list_init(&map->list);
    map->key_size = key_size;
    map->value_size = value_size;
}

// insert a key-value pair into the map, key and value will be copied
void map_insert(Map *map, const void *key, const void *value)
{
    void *_key = memcpy(malloc(map->key_size), key, map->key_size);
    void *_value = memcpy(malloc(map->value_size), value, map->value_size);
    KeyValuePair kvpair = {_key, _value};
    list_append_tail(&map->list, &kvpair, sizeof(kvpair));
}

// delete a key-value pair in map
void map_delete(Map *map, const void *key, int (*keycmp)(const void *, const void *), void key_free(void *),
                void value_free(void *))
{
    for (ListNode *node = map->list.head; node != NULL; node = node->prev)
    {
        KeyValuePair *kvpair = (KeyValuePair *)node->data;
        void *_key = kvpair->key;
        void *_value = kvpair->value;
        if (keycmp(key, _key) == 0)
        {
            key_free(_key);
            value_free(_value);
        }
        free(kvpair);
        list_pop_node(&map->list, node);
        break;
    }
}

// find a value by key and returns the pointer to its value
void *map_find(Map *map, const void *key, int (*keycmp)(const void *, const void *))
{
    for (ListNode *node = map->list.head; node != NULL; node = node->prev)
    {
        void *_key = ((KeyValuePair *)node->data)->key;
        if (keycmp(key, _key) == 0)
        {
            return ((KeyValuePair *)node->data)->value;
        }
    }

    return NULL;
}

void map_clear(Map *map, void key_free(void *), void value_free(void *))
{
    if (map == NULL)
        return;

    while (map->list.size > 0)
    {
        KeyValuePair *kvpair = (KeyValuePair *)list_pop_head(&map->list);
        key_free(kvpair->key);
        value_free(kvpair->value);
        free(kvpair);
    }
}
