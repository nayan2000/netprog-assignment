#ifndef SETUP_UTILITIES_H
#define SETUP_UTILITIES_H

#include "basic.h"

#define SERV_PORT "50000"
#define CLIENT_PORT "40000"

#define BACKLOG 5

int server_setup(char* server_port);
void grim_reaper(int sig);
int client_setup(char* server_ip, char* server_port);

#endif