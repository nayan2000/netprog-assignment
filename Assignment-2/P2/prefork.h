#ifndef PREFORK_H
#define PREFORK_H

#include "inet_sockets.h"

#define SERV_PORT "12345"
#define BACKLOG 10
#define MAP_SIZE 40

typedef struct child_details_t{
    pid_t pid;         			/* process ID */
    int pipefd;      			/* parent's stream pipe to/from child */    
    long count;       			/* # connections handled */
	int status;      			/* 0 = ready */
}child_details;


#endif