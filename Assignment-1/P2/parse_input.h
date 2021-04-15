/* Prevent accidental double inclusion */
#ifndef PARSE_INPUT_H 
#define PARSE_INPUT_H

#include "basic.h"

typedef struct cmd_node{
    char* node;
    char* command;
    struct cmd_node* next;
}cmd_node;

typedef struct cmd_list{
    cmd_node* head;
    int size;
}cmd_list;


/* Function Declarations */

void print_list(cmd_list*);
cmd_list* parse_inp(char*);

#endif