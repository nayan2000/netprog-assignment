#ifndef MSGQ_CLIENT_H
#define MSGQ_CLIENT_H

#include "basic.h"

#define SERVER_KEY 0x1aaaaaa1          
#define MAX_SIZE 40
#define RESP_MSG_SIZE 2048
#define MAX_GROUPS 40
#define MAX_MSGS 100
#define MAX_USERS 40
#define MAX_MSG_SIZE 100
#define MAX_GROUP_SZ 40
#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) /* Permissions for our IPC objects */

typedef struct request_msg {               
    long mtype;                        
    int  client_qid;                      /* ID of client's message queue */
    char uname[MAX_SIZE];
    char command;
    char args[MAX_SIZE];
    char data[MAX_SIZE];
    int t;
}request_msg;

typedef struct response_msg{                 /* Responses (server to client) */
    long mtype;                              /* One of RESP_MT_* values below */
    char data[RESP_MSG_SIZE];                /* File content / response message */
}response_msg;

typedef struct group{
    char groupname[20];
    char msgs[MAX_MSGS][MAX_MSG_SIZE];
    int msg_cnt;
}group;
typedef struct G{
    group g_list[MAX_GROUPS];
    int cap;
}G;
/* Types for response messages sent from server to client */
#define RESP_MT_CHECK_USER_EXIST 1
#define RESP_MT_CHECK_USER_NO_EXIST 2             /* User doesn't exist. Can't send private message */

#define RESP_MT_NOT_MEMBER 3                /* User not a member oof given group */
#define RESP_MT_GROUP_EXISTS 4              /* Group already exixts. Can't create group */
#define RESP_MT_GROUP_NO_EXIST 5            /* Group doesn't exist. Make new and join */
#define RESP_MT_ACK 6                       /* Message contains successful execution acknowlegement */
#define RESP_MT_DATA 7                      /* Message contains data */
#define RESP_MT_USER_EXIST 8
#define RESP_MT_JOIN 9
#define RESP_MT_GMSG 10
#define RESP_MT_CREAT 11


#define SHM_KEY 0x1234 

int getReqSize(request_msg* req);
int main();

#endif