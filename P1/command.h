/* Prevent accidental double inclusion */
#ifndef COMMAND_H 
#define COMMAND_H

#include "basic.h"
#include "job_table.h"
typedef struct token_stream{
    char* token;
    struct tokens* next;
}token_stream;

typedef struct token_list{
    token_stream* head;
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


#endif