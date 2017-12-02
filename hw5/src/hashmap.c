#include "hashmap.h"
#include "utils.h"
#include <errno.h>
#include <string.h>
#include "stdio.h"


#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

void reader_lock(hashmap_t *self){
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers++;
    if (self->num_readers == 1)   pthread_mutex_lock(&self->write_lock);
    pthread_mutex_unlock(&self->fields_lock);
    return;
}

void reader_unlock(hashmap_t *self){
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers--;
    if (self->num_readers == 0)   pthread_mutex_unlock(&self->write_lock);
    pthread_mutex_unlock(&self->fields_lock);
    return;
}



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
        (myHashMap->nodes)[initNode] = MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
        // myHashMap->nodes->tombstone = false;
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

    pthread_mutex_lock(&self->write_lock);

    if (self->invalid || self->capacity == 0 || self->nodes == NULL){
        errno = EINVAL;
        pthread_mutex_unlock(&self->write_lock);
        return false;
    }
    map_node_t *iterNode = self->nodes;
    int countNode = get_index(self, key);
    do{
        if (iterNode[countNode].key.key_base == NULL){
            // iterNode[countNode].key = key;
            // iterNode[countNode].val = val;
            // iterNode[countNode].tombstone = false;
            iterNode[countNode] =  MAP_NODE(key, val, false);
            self->size++;   // Adding new
            pthread_mutex_unlock(&self->write_lock);
            //printf("put in %lu\n", key.key_len);
            return true;
        }
        else if (!memcmp(iterNode[countNode].key.key_base, key.key_base, iterNode[countNode].key.key_len)){
            self->destroy_function(iterNode[countNode].key, iterNode[countNode].val);
            iterNode[countNode].key = key;
            iterNode[countNode].val = val;     // tombstone no need?
            pthread_mutex_unlock(&self->write_lock);
            return true;
        }
        else
            countNode = (countNode+1) % self->capacity;

    }while (countNode != get_index(self, key));     // Terminates when traversing all vector and gets to get_index(key)

    if (force == true){
        /* Overwrite & Evict */
        self->destroy_function(iterNode[countNode].key, iterNode[countNode].val); // ?????
        iterNode[countNode].key = key;
        iterNode[countNode].val = val;
        pthread_mutex_unlock(&self->write_lock);
        return true;
    }
    else {
        errno = ENOMEM;
        pthread_mutex_unlock(&self->write_lock);
        return false;
    }
}

map_val_t get(hashmap_t *self, map_key_t key) {
    if (self == NULL || key.key_base == NULL || key.key_len == 0){
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }

    reader_lock(self);

    if (self->invalid || self->capacity == 0 || self->nodes == NULL){
        errno = EINVAL;
        reader_unlock(self);
        return MAP_VAL(NULL, 0);
    }

    map_node_t *iterNode = self->nodes;
    int countNode = get_index(self, key);
    do{
        if (!memcmp(iterNode[countNode].key.key_base, key.key_base, iterNode[countNode].key.key_len)){
            reader_unlock(self);
            //printf("VAL in get: %lu\n", iterNode[countNode].val.val_len);
            return iterNode[countNode].val;
        }
        else if (iterNode[countNode].key.key_base == NULL && iterNode[countNode].tombstone == false){
            reader_unlock(self);
            return MAP_VAL(NULL, 0);    // empty node & no tomb -> Not found
        }
        countNode = (countNode+1) % self->capacity;     // Not found yet or Empty but have tombstone -> continues next

    }while (countNode != get_index(self, key));

    reader_unlock(self);
    return MAP_VAL(NULL, 0);    // All map not found
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    if (self == NULL || key.key_base == NULL || key.key_len == 0){
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    pthread_mutex_lock(&self->write_lock);

    if (self->invalid || self->capacity == 0 || self->nodes == NULL){
        errno = EINVAL;
        pthread_mutex_unlock(&self->write_lock);
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    /*
        1. save node in stack
        2. change map to zero
        3. set tombstone to 1
        4. return MAP_NODE

        Not found:  return NULL,0
        Logic same as get.
    */

    map_node_t *iterNode = self->nodes;
    int countNode = get_index(self, key);
    do{
        if (!memcmp(iterNode[countNode].key.key_base, key.key_base, iterNode[countNode].key.key_len) && iterNode[countNode].key.key_len){
            /* Got It */
            map_node_t toDelete = iterNode[countNode];
            iterNode[countNode] =  MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), true);    // Clear and have tombstone
            self->size--;
            pthread_mutex_unlock(&self->write_lock);
            //printf("To delete: %lu\n", toDelete.key.key_len);
            return toDelete;
        }
        else if (iterNode[countNode].key.key_base == NULL && iterNode[countNode].tombstone == false){
            pthread_mutex_unlock(&self->write_lock);
            //fprintf(stderr,"To TO:");
            return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);    // empty node & no tomb -> Not found
        }
        countNode = (countNode+1) % self->capacity;     // Not found yet or Empty but have tombstone -> continues next

    }while (countNode != get_index(self, key));

    pthread_mutex_unlock(&self->write_lock);
    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);

}

bool clear_map(hashmap_t *self) {

    if (self == NULL){
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->write_lock);

    if (self->invalid || self->capacity == 0 || self->nodes == NULL){
        errno = EINVAL;
        pthread_mutex_unlock(&self->write_lock);
        return false;
    }

    map_node_t *ptr = self->nodes;
    for (int freeNode = 0; freeNode < self->capacity; freeNode++){
        if (ptr[freeNode].tombstone == false && ptr[freeNode].key.key_base != NULL){
            //printf("Destroy %d\n", self->size);
            self->destroy_function(ptr[freeNode].key, ptr[freeNode].val);
            ptr[freeNode].tombstone = true;
            self->size--;
        }
    }
    pthread_mutex_unlock(&self->write_lock);
	return true;
}

bool invalidate_map(hashmap_t *self) {

    if (self == NULL){
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->write_lock);

    if (self->invalid || self->capacity == 0 || self->nodes == NULL){
        errno = EINVAL;
        pthread_mutex_unlock(&self->write_lock);
        return false;
    }

    map_node_t *ptr = self->nodes;
    for (int freeNode = 0; freeNode < self->capacity; freeNode++){
        if (ptr[freeNode].tombstone == false && ptr[freeNode].key.key_base != NULL){
            self->destroy_function(ptr[freeNode].key, ptr[freeNode].val);
            ptr[freeNode].tombstone = true;
            self->size--;
        }
    }
    free(self->nodes);
    self->invalid = 1;

    pthread_mutex_unlock(&self->write_lock);
    return true;
}

