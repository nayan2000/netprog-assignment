/* Prevent accidental double inclusion */
#ifndef SHELL_H 
#define SHELL_H

#include "basic.h"
#include "job_table.h"
#include "exec_command.h"


#define PROMPT BLUE"\n>> "RESET
#define CMD_SZ 512
#define MSGQ_PATH "./job_control.h"

/* Variable Declarations */
extern size_t max_cmd_sz;
extern command_details* j_table[MAX_CMD];

/* Function Declarations */
void remove_queue();
void clear_screen();
void grim_reaper(int sig);
void shortcut_handler(int sig);
void initial_setup();
bool process_command(bool *isfg, char * command);
int main(int argc, char* argv[]);


#endif