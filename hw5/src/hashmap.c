#include "hashmap.h"
#include "utils.h"
#include <errno.h>


#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {

    if (hash_function == NULL || destroy_function == NULL){
        errno = EINVAL;
        return NULL;
    }

    hashmap_t *myHashMap = calloc(1, sizeof(hashmap_t));        // Calloc HashMap
    if (myHashMap == NULL)  return NULL;

    myHashMap->capacity = capacity;
    myHashMap->size = 0;

    myHashMap->nodes = calloc(capacity, sizeof(map_node_t));    // Calloc Capacity * MapNodes
    if (myHashMap->nodes == NULL)   return NULL;
    for (int initNode = 0; initNode < capacity; initNode++){    // Initialize each node (Only tombstone)
        myHashMap->nodes->tombstone = 0;
    }
    myHashMap->hash_function = hash_function;
    myHashMap->destroy_function = destroy_function;

    myHashMap->num_readers = 0;
    if (pthread_mutex_init(&myHashMap->write_lock, NULL) != 0)
        return NULL;
    if (pthread_mutex_init(&myHashMap->fields_lock, NULL) != 0)
        return NULL;

    myHashMap-> invalid = 0;

    return myHashMap;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {

    if (self == NULL || key.key_base == NULL || key.key_len == 0 || val.val_base == NULL || val.val_len == 0){
        errno = EINVAL;
        return false;
    }

    map_node_t *iterNode = self->nodes;
    int countNode = get_index(self, key);
    do{
        if (iterNode[countNode].key.key_base == NULL){
            iterNode[countNode].key = key;
            iterNode[countNode].val = val;
            iterNode[countNode].tombstone = 0;
            self->size++;   // Adding new
            return true;
        }
        else if (iterNode[countNode].key.key_base == key.key_base && iterNode[countNode].key.key_len == key.key_len){
            iterNode[countNode].val = val;     // tombstone no need?
            return true;
        }
        else
            countNode = (countNode+1) % self->capacity;

    }while (countNode != get_index(self, key));     // Terminates when traversing all vector and gets to get_index(key)

    if (force == true){
        /* Overwrite & Evict */
        self->destroy_function(iterNode[countNode].key, iterNode[countNode].val);
        iterNode[countNode].key = key;
        iterNode[countNode].val = val;
        return true;
    }
    else {
        errno = ENOMEM;
        return false;
    }
}

map_val_t get(hashmap_t *self, map_key_t key) {
    return MAP_VAL(NULL, 0);
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
}

bool clear_map(hashmap_t *self) {
	return false;
}

bool invalidate_map(hashmap_t *self) {
    return false;
}
