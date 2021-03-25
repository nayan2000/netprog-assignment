#include "msgq_server.h"

hashmap* map;
group_list groups;
user_list users;

int group_to_id(char* groupname){
    for(int i = 0; i < groups.size; i++){
        if(strcmp(groupname, groups.list[i].groupname) == 0){
            return i;
        }
    }
    return -1;
}
char* id_to_group(int i){
    if(i >= groups.size)
        return NULL;
    char* grp = strdup(groups.list[i].groupname);
    return grp;
}

bool is_group_member(char* groupname, int client){
    int i = group_to_id(groupname);
    if(i == -1) return false;
    for(int j = 0; j < groups.list[i].size; i++){
        if(client == groups.list[i].users[j])
            return true;
    }
    return false;
}

int create_and_add_group(int size, char* groupname, int client){
    int i = group_to_id(groupname);
    if(i >= 0) return i;
    group g;
    bzero(&g, sizeof(g));
    g.size = size;
    strcpy(g.groupname, groupname);
    g.users[0] = client;

    groups.list[groups.size] = g;
    (groups.size)++;
    return -1;
}
void setup_client_msgq(const request_msg *req){
    char* username = strdup(req->uname);
    if(!has_key(map, username)){
        insert(map, username, req->client_qid);
        return;
    }
    free(username);
}

void serve_request(const request_msg *req){
    int fd, i;
    ssize_t numRead;
    response_msg resp;
    bzero(&resp, sizeof(resp));

    char command = req->command;
    char *data = NULL, *args = NULL;
    if(strlen(req->data))
        data = strdup(req->data);
    if(strlen(req->args))
        args = strdup(req->args);

    switch(command){
        case 'c':
            resp.mtype = RESP_MT_ACK;
            /* data contains group name */
            int ret = create_and_add_group(1, data, req->client_qid);
            if(ret >= 0){
                resp.mtype = RESP_MT_GROUP_EXISTS;
                sprintf(resp.data, "Group %s already exists - ID %d\n---\n", data, ret);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            else{
                sprintf(resp.data, "Group Created %s - ID %d\n---\n", data, ret);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            break;
        case 'l':
            resp.mtype = RESP_MT_DATA;
            /* data contains null */
            char suffix[80] = {0};
            for(int i = 0; i < groups.size; i++) {
                if(is_group_member(groups.list[i].groupname, req->client_qid))
                    sprintf(suffix, GREEN"Group name : %s - ID %d - Member\n---\n"RESET, groups.list[i].groupname, i);
                else
                    sprintf(suffix, RED"Group name : %s - ID %d - Not Member\n---\n"RESET, groups.list[i].groupname, i);
                strcat(resp.data, suffix);
            }
            msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            break;
        case 'j':
            resp.mtype = RESP_MT_ACK;
            i = group_to_id(data);
            /* data contains group name */
            if(is_group_member(data, req->client_qid)){
                sprintf(resp.data, "Already a member of the group : %s - ID %d\n---\n", data, i);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            else{
                i = create_and_add_group(1, data, req->client_qid);
                if(i >= 0){ /* Group Exists */
                    int j = groups.list[i].size;
                    groups.list[i].users[j] = req->client_qid;
                    (groups.list[i].size)++;
                    sprintf(resp.data, "Added to the group %s - ID %d\n---\n", data, i);
                    msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
                }
                else{
                    resp.mtype = RESP_MT_GROUP_NO_EXIST;
                    sprintf(resp.data, "New Group Created %s - ID %d\n---\n", data, i);
                    msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
                }
            }
            break;
        case 'g':
            resp.mtype = RESP_MT_ACK;
            /* data contains the group message */
            /* args contain group name */
            int uid = req->client_qid;
            i = create_and_add_group(1, args, req->client_qid);
            if(i >= 0){
               if(is_group_member(args, uid)){
                    response_msg others;
                    bzero(&others, sizeof(others));
                    sprintf(others.data, "\nMessage from group : %s - ID %d\n\t Username : %s - ID %d\n---\n", 
                            groups.list[i].groupname, i, req->uname, req->client_qid);
                    strcat(others.data, data);
                    strcat(others.data, "\n---\n");
                    for(int j = 0; j < groups.list[i].size; j++) {
                        if(groups.list[ret].users[j] == uid) {
                            continue;
                        }
                        others.mtype = RESP_MT_DATA;
                        msgsnd(groups.list[i].users[j], &others, strlen(others.data) + 1, IPC_NOWAIT);
                    }
                    sprintf(resp.data, "\nSent messages to the group %s : %d\n---\n", args, i);
                    msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
               }
               else{
                    resp.mtype = RESP_MT_NOT_MEMBER;
                    sprintf(resp.data, "\nCan't send messages : Not a member of the group %s : %d\n---\n", args, ret);
                    msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
               }
            }
            else{
                resp.mtype = RESP_MT_GROUP_NO_EXIST;
                sprintf(resp.data, "\nGroup does not exist\nNew Group Created %s : %d\n---\n", args, ret);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            break;
        case 'u':
            resp.mtype = RESP_MT_ACK;
            /* data contains the user message */
            /* args contain user name */
            int qid = get(map, args);
            bool found = false;
            for(int i = 0; i < users.size; i++){
                if(users.list[i] == qid){
                    found = true;
                    response_msg others;
                    bzero(&others, sizeof(others));
                    sprintf(others.data, "\nMessage from user : %s - ID %d\n---\n", req->uname, req->client_qid);
                    strcat(others.data, data);
                    strcat(others.data, "---\n");
                    others.mtype = RESP_MT_DATA;
                    msgsnd(qid, &others, strlen(others.data) + 1, IPC_NOWAIT);
                    break;
                }
            }
            if(found){
                sprintf(resp.data, "\nSent messages to the user %s : %d\n", args, ret);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            else{
                resp.mtype = RESP_MT_USER_NO_EXIST;
                sprintf(resp.data, "\nUser %s does not exist : %d\n", args, ret);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
        default:
            break;    
    }
}

int main(int argc, char *argv[]){
    struct request_msg req;
    ssize_t msgLen;
    int serverId;
    
    printf("here\n");

    create_hm(map, MAX_USERS);
    bzero(&groups, sizeof(groups));
    bzero(&users, sizeof(users));

    
    /* Create server message queue */
    serverId = msgget(SERVER_KEY, IPC_CREAT | IPC_EXCL |
                            S_IRUSR | S_IWUSR | S_IWGRP);
    if (serverId == -1){
        perror(RED"msgget"RESET);
        exit(0);
    }

    /* Read requests, handle iteratively */

    for(;;){
        msgLen = msgrcv(serverId, &req, sizeof(request_msg) - sizeof(long), 0, 0);
        if (msgLen == -1) {
            if (errno == EINTR)         /* Interrupted by SIGCHLD handler? */
                continue;               /* ... then restart msgrcv() */
            perror(RED"msgrcv"RESET);           /* Some other error */
            break;                      /* ... so terminate loop */
        }
        users.list[users.size] = req.client_qid;
        (users.size)++;
        setup_client_msgq(&req);
        serve_request(&req);
    }

    /* If msgrcv()  fails, remove server MQ and exit */

    if (msgctl(serverId, IPC_RMID, NULL) == -1)
        perror("msgctl");
    exit(EXIT_SUCCESS);
}