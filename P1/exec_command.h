/* Prevent accidental double inclusion */
#ifndef EXEC_COMMAND_H 
#define EXEC_COMMAND_H

#include "parse_command.h"
#include "job_control.h"
#define MAX_SIZE_SINGLE_CMD 5


extern size_t max_cmd_sz;
extern command_details* j_table[MAX_CMD];


void print_broken_cmd(char** args);
char** break_loner_cmds(char* cmd);
bool run_job(char* command);
#endif