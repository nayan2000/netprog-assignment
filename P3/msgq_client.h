#ifndef MSGQ_CLIENT_H
#define MSGQ_CLIENT_H

#include "basic.h"

#define SERVER_KEY 0x1aaaaaa1          
#define MAX_SIZE 40

typedef struct request_msg {               
    long mtype;                        
    int  client_qid;                      /* ID of client's message queue */
    char* uname;
    char command;
    char args[MAX_SIZE];
    char data[MAX_SIZE];
}request_msg;

int getReqSize(request_msg* req);
int main();

#endif