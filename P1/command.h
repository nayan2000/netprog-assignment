/* Prevent accidental double inclusion */
#ifndef COMMAND_H 
#define COMMAND_H

#include "common.h"

typedef struct token_stream{
    char* token;
    struct tokens* next;
}token_stream;

typedef struct token_list{
    token_stream* head;
    int size;
}token_list;

/* Variable Declarations */
extern size_t max_cmd_sz;

/* Function Declarations */


#endif