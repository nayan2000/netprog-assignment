/* Prevent accidental double inclusion */
#ifndef EXEC_COMMAND_H 
#define EXEC_COMMAND_H

#include "parse_command.h"
#include "job_control.h"
typedef enum pipe_type{
    SPIPE, DPIPE, TPIPE
}pipe_type;



void trim(char *s, bool space);
char* check_redirection(char* command, int in, int out);
void redirect_desc_io(int oldfd, int newfd);
bool preprocess_pipe_io(int in, int out);
int execute(char* command);

#endif