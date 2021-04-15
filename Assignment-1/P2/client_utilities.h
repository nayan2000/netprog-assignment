#ifndef CLIENT_UTILITIES_H
#define CLIENT_UTILITIES_H

#include "basic.h"
#include "inet_sockets.h"
#include <sys/prctl.h> // prctl(), PR_SET_PDEATHSIG

#define MAX_BUF_SZ 2048
#define MAX_OUTPUT 2048
#define MAX_INPUT 2048
#define CMD_SZ 512

void trim(char *s, bool space);
char* get_path(char* exe);
char** tokenise(char* command);
void redirect_desc_io(int oldfd, int newfd);
void clear_screen();
bool process_command(char *command);

#endif