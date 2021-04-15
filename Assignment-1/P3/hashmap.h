#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct bucket_node_t {
	char key[20];
	int val;
	struct bucket_node_t *next;
} bucket_node;

typedef struct hashmap_t {
	bucket_node **buckets;
	size_t capacity;
} hashmap;

size_t hash(const char*, size_t);
void create_hm(hashmap*, size_t);
void free_bucket(bucket_node*);
void free_hm(hashmap*);
void insert(hashmap*, const char*, int);
bucket_node* get_bucket(hashmap*, const char*);
int get(hashmap*, const char*);
int has_key(hashmap*, const char*);
bool remove_key(hashmap *map, const char* key);


#endif