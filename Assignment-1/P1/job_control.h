#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include "basic.h"
#include "job_table.h"
#include "parse_command.h"
#include "sc_table.h"
#define MAX_SIZE_SINGLE_CMD 40


extern size_t max_cmd_sz;
extern command_details* j_table[MAX_CMD];
extern char* sc_table[MAX_SC_TABLE_SIZE];


void make_foreground(char* broken_cmd);
void make_background(char* broken_cmd);
void print_jobs();
void print_broken_cmd(char** args);
char** break_loner_cmds(char* cmd);
bool run_job(char* command);
#endif