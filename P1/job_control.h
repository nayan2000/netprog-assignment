#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include "basic.h"
#include "job_table.h"
extern command_details* j_table[MAX_CMD];

void make_foreground(char* broken_cmd);
void make_background(char* broken_cmd);
void print_jobs();
#endif