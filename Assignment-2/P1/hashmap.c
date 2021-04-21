#include "hashmap.h"

size_t hash(int key, size_t size){
	size_t hcode = 0;
	hcode = key % size;
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

void insert(hashmap *map, int key, host_det val){
	size_t loc = hash(key, map->capacity);

	bucket_node *n = (bucket_node*) malloc(sizeof(bucket_node));
	n->key = key;
	n->val = val;
	n->next = map->buckets[loc];

	map->buckets[loc] = n;
}

bucket_node* get_bucket(hashmap *map, int key){
	size_t loc = hash(key, map->capacity);

	return map->buckets[loc];
}

host_det* get(hashmap *map, int key){
	size_t loc = hash(key, map->capacity);

	bucket_node *ptr = map->buckets[loc];

	while(ptr){
		if(ptr->key == key)
			return &(ptr->val);

		ptr = ptr->next;
	}

	return NULL;
}

int has_key(hashmap *map, int key){
	return get(map, key) == NULL ? 0 : 1;
}

bool remove_key(hashmap *map, int key){
	size_t loc = hash(key, map->capacity);
	bucket_node *ptr = map->buckets[loc];
	bucket_node* prev = NULL;

	while(ptr){
		if(ptr->key == key){
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

void print_hm(hashmap* hm, int hashdim){
	for(int i = 0; i < hashdim; i++){
		if(hm->buckets[i]){
			bucket_node* temp = hm->buckets[i];
			printf("%d\n", temp->key);
			printf("%s\n", (temp->val).ip);
		}
	}
}
