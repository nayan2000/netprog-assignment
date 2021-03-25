#ifndef SETUP_UTILITIES_H
#define SETUP_UTILITIES_H

#include "basic.h"


int server_setup(char* server_port);
void grim_reaper(int sig);
int client_setup(char* server_ip, char* server_port);

#endif