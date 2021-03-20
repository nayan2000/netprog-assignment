/* Prevent accidental double inclusion */
#ifndef PARSE_COMMAND_H 
#define PARSE_COMMAND_H

#include "basic.h"
#include "job_table.h"
typedef struct token_node{
    char* token;
    struct token_node* next;
}token_node;

typedef struct token_list{
    token_node* head;
    int size;
}token_list;

typedef enum pipe_type{
    SPIPE, DPIPE, TPIPE
}pipe_type;

typedef struct pipe_details{
    pipe_type t;
    int rfd;
    int wfd;
}pipe_details;

/* Variable Declarations */
// extern size_t max_cmd_sz;
// extern command_details* j_table[MAX_CMD];

/* Function Declarations */
char* substr(char*, int, int);
token_list* add_command(token_list*, char*);
void print_list(token_list*);
token_list* parse_cmd(char*);

#endif