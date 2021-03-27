#ifndef MSGQ_SERVER_H
#define MSGQ_SERVER_H

#include "basic.h"
#include "hashmap.h"
#define SERVER_KEY 0x1aaaaaa1          
#define MAX_SIZE 40
#define MAX_USERS 40
#define MAX_GROUP_SZ 40
#define MAX_GROUPS 40

typedef struct request_msg {               
    long mtype;                        
    int  client_qid;                      /* ID of client's message queue */
    char uname[MAX_SIZE];
    char command;
    char args[MAX_SIZE];
    char data[MAX_SIZE];
}request_msg;

typedef struct group{
    int size;
    int users[MAX_GROUP_SZ];
    char groupname[20];
}group;

typedef struct group_list{
    group list[MAX_GROUPS];
    int size;
}group_list;

typedef struct user_list{
    int list[MAX_USERS];
    int size;
}user_list;

#define RESP_MSG_SIZE 8192

typedef struct response_msg{                 /* Responses (server to client) */
    long mtype;                              /* One of RESP_MT_* values below */
    char data[RESP_MSG_SIZE];                /* File content / response message */
}response_msg;

/* Types for response messages sent from server to client */
#define RESP_MT_CHECK_USER_EXIST 1
#define RESP_MT_CHECK_USER_NO_EXIST 2             /* User doesn't exist. Can't send private message */

#define RESP_MT_NOT_MEMBER 3                /* User not a member oof given group */
#define RESP_MT_GROUP_EXISTS 4              /* Group already exixts. Can't create group */
#define RESP_MT_GROUP_NO_EXIST 5            /* Group doesn't exist. Make new and join */
#define RESP_MT_ACK 6                       /* Message contains successful execution acknowlegement */
#define RESP_MT_DATA 7                      /* Message contains data */
#define RESP_MT_USER_NO_EXIST 8

int group_to_id(char*);
char* id_to_group(int);
bool is_group_member(char*, int);
int create_and_add_group(int, char*, int);
void setup_client_msgq(const request_msg*);
void serve_request(const request_msg*);
int main(int, char**);

#endif