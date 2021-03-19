/* Prevent accidental double inclusion */
#ifndef SHELL_H 
#define SHELL_H

#include "basic.h"
#include "job_table.h"

#define PROMPT BLUE">> "RESET
#define CMD_SZ 512


/* Variable Declarations */
extern size_t max_cmd_sz;
extern command_details* j_table[MAX_CMD];

/* Function Declarations */
extern bool process_command(bool *isfg, char * command);
extern int main(int argc, char* argv[]);


#endif