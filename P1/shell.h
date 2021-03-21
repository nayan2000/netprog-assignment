/* Prevent accidental double inclusion */
#ifndef SHELL_H 
#define SHELL_H

#include "basic.h"
#include "job_table.h"
#include "exec_command.h"


#define PROMPT BLUE"\n>> "RESET
#define CMD_SZ 512
#define MSGQ_PATH "./job_control.h"

typedef struct my_msgbuf{
  long mtype;
  pid_t mtext;
}my_msgbuf;


/* Variable Declarations */
extern size_t max_cmd_sz;
extern command_details* j_table[MAX_CMD];

/* Function Declarations */
void remove_queue();
void handler(int sig);
void initial_setup();
extern bool process_command(bool *isfg, char * command);
extern int main(int argc, char* argv[]);


#endif