/* Prevent accidental double inclusion */
#ifndef SHELL_H 
#define SHELL_H

#include "common.h"
#define PROMPT BLUE">> "RESET
#define CMD_SZ 512

/* Variable Declarations */
extern size_t max_cmd_sz = CMD_SZ + 1;

/* Function Declarations */
extern bool process_command(bool *isfg, char * command);
extern int main(int argc, char* argv[]);

#endif