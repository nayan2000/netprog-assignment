#include "hashmap.h"

size_t hash(char *str, size_t size){
	size_t hcode = 0;
	char ch = *str;

	while((ch = *(str++))){
		hcode += (int) ch;
	}

	hcode = hcode % size;

	return hcode;
}

void create_hm(hashmap *map, size_t cap){
	map->buckets = (bucket_node**) malloc(sizeof(bucket_node*)*cap);
	map->capacity = cap;
	size_t i;

	for(i=0;i<cap;i++){
		map->buckets[i] = NULL;
	}
}

void free_bucket(bucket_node *n){
	bucket_node *ptr = n;
    bucket_node *next = NULL;

	while(ptr){
        next = ptr->next;
		free(ptr);
		ptr = next;
	}
}

void free_hm(hashmap *map){
	int i = 0;

	while(i < map->capacity){
		free_bucket(map->buckets[i]);
		i++;
	}
    free(map);
}

void insert(hashmap *map, char *key, int val){
	size_t loc = hash(key, map->capacity);

	bucket_node *n = (bucket_node*) malloc(sizeof(bucket_node));
	strcpy(n->key, key);
	n->val = val;
	n->next = map->buckets[loc];

	map->buckets[loc] = n;
}

bucket_node* get_bucket(hashmap *map, char *key){
	size_t loc = hash(key, map->capacity);

	return map->buckets[loc];
}

int get(hashmap *map, char *key){
	size_t loc = hash(key, map->capacity);

	bucket_node *ptr = map->buckets[loc];

	while(ptr){
		if(!strcmp(ptr->key, key))
			return ptr->val;

		ptr = ptr->next;
	}

	return -1;
}

int has_key(hashmap *map, char *key){
	return get(map, key) == -1 ? 0 : 1;
}




/*Reverse Map */



size_t hash_r(int key, size_t size){
	size_t hcode = 0;
	hcode = hcode % size;

	return hcode;
}

void create_hm_r(hashmap_r *map, size_t cap){
	map->buckets = (bucket_node_r**) malloc(sizeof(bucket_node_r*)*cap);
	map->capacity = cap;
	size_t i;

	for(i=0;i<cap;i++){
		map->buckets[i] = NULL;
	}
}

void free_bucket_r(bucket_node_r *n){
	bucket_node_r *ptr = n;
    bucket_node_r *next = NULL;

	while(ptr){
        next = ptr->next;
		free(ptr);
		ptr = next;
	}
}

void free_hm_r(hashmap_r *map){
	int i = 0;

	while(i < map->capacity){
		free_bucket_r(map->buckets[i]);
		i++;
	}
    free(map);
}

void insert_r(hashmap_r *map, int key, char* val){
	size_t loc = hash_r(key, map->capacity);

	bucket_node_r *n = (bucket_node_r*) malloc(sizeof(bucket_node_r));
	n->key = key;
	strcpy(n->val, val);
	n->next = map->buckets[loc];

	map->buckets[loc] = n;
}

bucket_node_r* get_bucket_r(hashmap_r *map, int key){
	size_t loc = hash_r(key, map->capacity);

	return map->buckets[loc];
}

char* get_r(hashmap_r *map, int key){
	size_t loc = hash_r(key, map->capacity);
    
	bucket_node_r *ptr = map->buckets[loc];

	while(ptr){
		if(ptr->key == key){
			char* uname = (char*)malloc(sizeof(char) * 20);
            strcpy(uname, ptr->val);
            return uname;
        }

		ptr = ptr->next;
	}

	return NULL;
}

int has_key_r(hashmap_r *map, int key){
	return get_r(map, key) == NULL ? 0 : 1;
}