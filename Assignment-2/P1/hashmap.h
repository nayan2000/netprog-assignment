#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct host_det{
    char ip[41];
    bool type;		//0 - ipv4 and 1- ipv6
	int count;
    double RTT[3]; // in milliseconds
}host_det;

typedef struct bucket_node_t {
	int key;
	host_det val;
	struct bucket_node_t *next;
}bucket_node;

typedef struct hashmap_t {
	bucket_node **buckets;
	size_t capacity;
} hashmap;

size_t hash(int, size_t);
void create_hm(hashmap*, size_t);
void free_bucket(bucket_node*);
void free_hm(hashmap*);
void insert(hashmap*, int, host_det);
bucket_node* get_bucket(hashmap*, int);
host_det* get(hashmap*, int);
int has_key(hashmap*, int);
bool remove_key(hashmap *map, int key);


#endif