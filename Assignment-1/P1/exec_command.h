/* Prevent accidental double inclusion */
#ifndef EXEC_COMMAND_H 
#define EXEC_COMMAND_H

#include "parse_command.h"
#include "job_control.h"

#define BUFSZ 8092
typedef enum pipe_type{
    SPIPE, DPIPE, TPIPE
}pipe_type;

typedef struct my_msgbuf{
  long mtype;
  pid_t mtext;
}my_msgbuf;

extern int msqid;
extern key_t key;

char** tokenise(char* command);
char* get_path(char* exe);
void trim(char *s, bool space);
bool exec_curr_cmd(char* command, int t, int in, int out);
char* check_redirection(char* command, int in, int out);
void redirect_desc_io(int oldfd, int newfd);
bool preprocess_pipe_io(int in, int out);
int execute(char* command);

#endif