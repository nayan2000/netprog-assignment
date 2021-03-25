#ifndef SERVER_UTILITIES_H
#define SERVER_UTILITIES_H

#include "basic.h"
#include "get_config.h"
#include "parse_input.h"
#include "inet_sockets.h"

#define MAX_BUF_SZ 2048
#define MAX_OUTPUT 2048
#define MAX_INPUT 2048

bool handle_nodes_cmd(char* cmd, char* buf);
void handle_request(int cfd, char * address_string);

#endif