#include "hashmap.h"

size_t hash(const char *str, size_t size){
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

void insert(hashmap *map, const char *key, const char *val){
	size_t loc = hash(key, map->capacity);

	bucket_node *n = (bucket_node*) malloc(sizeof(bucket_node));
	strcpy(n->key, key);
	strcpy(n->val, val);
	n->next = map->buckets[loc];

	map->buckets[loc] = n;
}

bucket_node* get_bucket(hashmap *map, const char *key){
	size_t loc = hash(key, map->capacity);

	return map->buckets[loc];
}

char* get(hashmap *map, const char *key){
	size_t loc = hash(key, map->capacity);

	bucket_node *ptr = map->buckets[loc];

	while(ptr){
		if(!strcmp(ptr->key, key))
			return ptr->val;

		ptr = ptr->next;
	}

	return "-1";
}

int has_key(hashmap *map, const char *key){
	return strcmp(get(map, key), "-1") == 0 ? 1 : 0;
}

bool remove_key(hashmap *map, const char* key){
	size_t loc = hash(key, map->capacity);
	bucket_node *ptr = map->buckets[loc];
	bucket_node* prev = NULL;

	while(ptr){
		if(!strcmp(ptr->key, key)){
			if(prev == NULL){
				map->buckets[loc] = ptr->next;
			}
			else
				prev->next = ptr->next;
			free(ptr);
			return true;
		}
		prev = ptr;
		ptr = ptr->next;
	}

	return false;
}
