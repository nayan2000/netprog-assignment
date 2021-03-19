/* Prevent accidental double inclusion */
#ifndef COMMAND_H 
#define COMMAND_H

#include "basic.h"
#include "job_table.h"
typedef struct token_stream{
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
extern size_t max_cmd_sz;
extern command_details* j_table;

/* Function Declarations */
char* substr(char*, int, int);
token_list* addCommand(token_list*, char*);
void printList(token_list*);
token_list* parseCommand(char*);

#endif