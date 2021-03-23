#include "get_config.h"

char **load_config_file(const char *filename) {
	char **arr = malloc(sizeof(char *) * (MAX_NODES + 2));
	FILE *fp = fopen(filename, "r");
	if(fp == NULL) {
		fprintf(stderr, RED"ERROR: CAN'T OPEN CONFIG FILE\n"RESET);	
		exit(0);
	}
	char ip[20];
	int i;
	while(!feof(fp)) {
        fscanf(fp, "%*c%d %s", &i, ip);
        arr[i++] = strdup(ip);
	}
	arr[i] = (char*)NULL;

	return arr;
}

