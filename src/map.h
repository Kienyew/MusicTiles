// A simple mapping data structure

#ifndef __H_MAP
#define __H_MAP

#include "list.h"
#include <stddef.h>

typedef struct Map
{
    List list;
    size_t key_size;
    size_t value_size;
} Map;

typedef struct KeyValuePair
{
    void *key;
    void *value;
} KeyValuePair;

static void (*next_key_free)(void *);
static void (*next_value_free)(void *);

void key_value_pair_free(KeyValuePair *kvpair);

void map_init(Map *map, size_t key_size, size_t value_size);
void map_insert(Map *map, const void *key, const void *value);
void map_delete(Map *map, const void *key, int (*keycmp)(const void *, const void *), void key_free(void *),
                void value_free(void *));
void *map_find(Map *map, const void *key, int (*keycmp)(const void *, const void *));
void map_clear(Map *map, void key_free(void *), void value_free(void *));

#endif // __H_MAP
