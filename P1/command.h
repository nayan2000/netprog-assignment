/* Prevent accidental double inclusion */
#ifndef COMMAND_H 
#define COMMAND_H

#include "common.h"

typedef struct token_node{
    char* token;
    struct token_node* next;
}token_node;

typedef struct token_list{
    token_node* head;
    int size;
}token_list;

/* Variable Declarations */
extern size_t max_cmd_sz;

/* Function Declarations */
char* substr(char*, int, int);
token_list* addCommand(token_list*, char*);
void printList(token_list*);
token_list* parseCommand(char*);

#endif