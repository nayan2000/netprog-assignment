#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdlib.h>
#include <string.h>

typedef struct bucket_node_t {
	char key[20];
	int val;
	struct bucket_node_t *next;
} bucket_node;

typedef struct hashmap_t {
	bucket_node **buckets;
	size_t capacity;
} hashmap;

size_t hash(char*, size_t);
void create_hm(hashmap*, size_t);
void free_bucket(bucket_node*);
void free_hm(hashmap*);
void insert(hashmap*, char*, int);
bucket_node* get_bucket(hashmap*, char*);
int get(hashmap*, char*);
int has_key(hashmap*, char*);


typedef struct bucket_node_t_r{
    int key;
	char val[20];
	struct bucket_node_t_r *next;
}bucket_node_r;

typedef struct hashmap_r_t{
	bucket_node_r **buckets;
	size_t capacity;
}hashmap_r;

size_t hash_r(int, size_t);
void create_hm_r(hashmap_r*, size_t);
void free_bucket_r(bucket_node_r*);
void free_hm_r(hashmap_r*);
void insert_r(hashmap_r*, int, char*);
bucket_node_r* get_bucket_r(hashmap_r*, int);
char* get_r(hashmap_r*, int);
int has_key_r(hashmap_r*, int);

#endif