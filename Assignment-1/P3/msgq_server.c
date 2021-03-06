#include "msgq_server.h"

hashmap* map;
group_list groups;
user_list users;
int serverId;
bool stop = false;

void sighandler(int sig){
    stop = true;
}

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
    for(int j = 0; j < groups.list[i].size; j++){
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

    g.msg_cnt = 0;
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

void remove_user_from_group(int cid) {
    // CHECK ALL GROUPS
    for(int i = 0; i < groups.size; ++i) {
        for(int j = 0; j < groups.list[i].size; ++j) {
            if(cid == groups.list[i].users[j]) {
                // SWAP ID WITH CLIENT ID STORED AT LAST INDEX AND REDUCE SIZE BY 1
                printf("\nRemoved %d from group %s\n", cid, groups.list[i].groupname);
                int temp = groups.list[i].users[j];
                groups.list[i].users[j] = groups.list[i].users[groups.list[i].size - 1];
                groups.list[i].users[groups.list[i].size - 1] = temp;
                groups.list[i].size--;
                break;
            }
        }
    }
}

void serve_request(const request_msg *req){
    response_msg resp;
    bzero(&resp, sizeof(resp));

    char command = req->command;
    char *data = NULL, *args = NULL;
    if(strlen(req->data))
        data = strdup(req->data);
    if(strlen(req->args))
        args = strdup(req->args);
    int i;
    switch(command){
        case 'n':
            if(!has_key(map, req->uname)){
                resp.mtype = RESP_MT_CHECK_USER_NO_EXIST;
                users.list[users.size] = req->client_qid;
                (users.size)++;
                setup_client_msgq(req);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            else{
                resp.mtype = RESP_MT_CHECK_USER_EXIST;
                int qid = get(map, req->uname);
                sprintf(resp.data, "%d", qid);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            break;
        case 'c':
            resp.mtype = RESP_MT_CREAT;
            /* data contains group name */
            i = create_and_add_group(1, data, req->client_qid);
            if(i >= 0){
                resp.mtype = RESP_MT_GROUP_EXISTS;
                sprintf(resp.data, "Group %s already exists\n---\n", data);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            else{
                sprintf(resp.data, "Group Created %s\n---\n", data);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            break;
        case 'l':
            resp.mtype = RESP_MT_DATA;
            /* data contains null */
            char suffix[80] = {0};
            for(int i = 0; i < groups.size; i++) {
                if(is_group_member(groups.list[i].groupname, req->client_qid))
                    sprintf(suffix, GREEN"Group name : %s - Member\n---\n"RESET, groups.list[i].groupname);
                else
                    sprintf(suffix, RED"Group name : %s - Not Member\n---\n"RESET, groups.list[i].groupname);
                strcat(resp.data, suffix);
            }
            msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            break;
        case 'j':
            resp.mtype = RESP_MT_ACK;
            i = group_to_id(data);
            /* data contains group name */
            printf("Client ID :%d, uname %s, group %s, group id : %d\n", req->client_qid, req->uname, data, i);
            if(is_group_member(data, req->client_qid)){
                sprintf(resp.data, "Already a member of the group : %s\n---\n", data);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            else{
                i = create_and_add_group(1, data, req->client_qid);
                if(i >= 0){ /* Group Exists */
                    int j = groups.list[i].size;
                    groups.list[i].users[j] = req->client_qid;
                    (groups.list[i].size)++;
                    sprintf(resp.data, "Added to the group %s\n---\nMessages:\n", data);
                    for(int j = 0; j < groups.list[i].msg_cnt; ++j) {
                        strcat(resp.data, groups.list[i].msgs[j].data);
                    }
                    resp.mtype = RESP_MT_JOIN;
                    msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
                }
                else{
                    resp.mtype = RESP_MT_CREAT;
                    sprintf(resp.data, "Group Created %s\n---\n", data);
                    msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
                }
            }
            break;
        case 'g':
            resp.mtype = RESP_MT_ACK;
            /* data contains the group message */
            /* args contain group name */
            /* t contains auto delete timeout */
            int uid = req->client_qid;
            i = create_and_add_group(1, args, req->client_qid);
            if(i >= 0){
               if(is_group_member(args, uid)){
                   // ADD RECEIVED MESSAGE TO MSG ARRAY OF GROUP
                    gmsg g;
                    strcpy(g.data, req->data);
                    strcat(g.data, " - ");
                    strcat(g.data, req->uname);
                    strcat(g.data, "\n---\n");
                    g.t = req->t;
                    g.ct = time(NULL); // GIVES TIMESTAMP IN SEC
                    groups.list[i].msgs[groups.list[i].msg_cnt] = g;
                    (groups.list[i].msg_cnt)++;

                    response_msg others;
                    bzero(&others, sizeof(others));
                    
                    others.mtype = RESP_MT_GMSG;
                    sprintf(others.data, "\nGroup: %s\n---\n", 
                            groups.list[i].groupname);
                    strcat(others.data, g.data);
                    for(int j = 0; j < groups.list[i].size; j++) {
                        // if(groups.list[i].users[j] == uid) {
                        //     continue;
                        // }
                        
                        msgsnd(groups.list[i].users[j], &others, strlen(others.data) + 1, IPC_NOWAIT);
                    }
                    sprintf(resp.data, "\nSent messages to the group %s\n---\n", args);
                    msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
               }
               else{
                    resp.mtype = RESP_MT_NOT_MEMBER;
                    sprintf(resp.data, "\nCan't send messages : Not a member of the group %s\n---\n", args);
                    msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
               }
            }
            else{
                resp.mtype = RESP_MT_GROUP_NO_EXIST;
                sprintf(resp.data, "\nGroup does not exist\nNew Group Created %s\n---\n", args);
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
                    sprintf(others.data, "\nMessage from user : %s\n---\n", req->uname);
                    strcat(others.data, data);
                    strcat(others.data, "\n---\n");
                    others.mtype = RESP_MT_DATA;
                    msgsnd(qid, &others, strlen(others.data) + 1, IPC_NOWAIT);
                    break;
                }
            }
            if(found){
                sprintf(resp.data, "\nSent messages to the user %s\n", args);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            else{
                resp.mtype = RESP_MT_USER_NO_EXIST;
                sprintf(resp.data, "\nUser %s does not exist\n", args);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            break;
        case 'r':
            remove_user_from_group(req->client_qid);
            remove_key(map, req->uname);
            break;
        case 's':
            resp.mtype = RESP_MT_DATA;
            /* data contains group name */
            i = group_to_id(data);
            if(i < 0){
                resp.mtype = RESP_MT_GROUP_NO_EXIST;
                sprintf(resp.data, "\nGroup %s does not exist\n---\n", args);
                msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
            }
            else{
                if(is_group_member(data, req->client_qid)){
                    for(int j = 0; j < groups.list[i].msg_cnt; ++j) {
                        strcat(resp.data, groups.list[i].msgs[j].data);
                    }
                    msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
                }else{
                    resp.mtype = RESP_MT_NOT_MEMBER;
                    sprintf(resp.data, "\nCan't retrieve messages : Not a member of the group %s\n---\n", args);
                    msgsnd(req->client_qid, &resp, strlen(resp.data) + 1, IPC_NOWAIT);
                }
            }
            break;
        default:
            break;    
    }
}

void delete_msgqs(int serverId) {
    for(int i = 0; i < users.size; i++){
        msgctl(users.list[i], IPC_RMID, NULL);
    }
    printf("Server ID: %d\n", serverId);
    if(msgctl(serverId, IPC_RMID, NULL) == -1){
        perror("Server Delete");
    }
}

int main(int argc, char *argv[]){
    
    struct request_msg req;
    ssize_t msgLen;int serverId;
    
    map = (hashmap*)malloc(sizeof(hashmap));
    create_hm(map, MAX_USERS);
    bzero(&groups, sizeof(groups));
    bzero(&users, sizeof(users));

    
    /* Create server message queue */
    serverId = msgget(SERVER_KEY, IPC_CREAT | IPC_EXCL | 0777);
    printf("Server ID: %d\n", serverId);
    if (serverId == -1){
        perror("Server MQ exists");
        exit(0);
    }
    signal(SIGINT, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGTSTP, sighandler);

    /* Read requests, handle iteratively */
    printf(YELLOW"**********SERVER STARTED*********\n"RESET);
    for(;;){
        if(stop) break;
        // CHECK IF TIMER OF ANY AUTO DELETE MSG EXPIRES
        for(int i = 0; i < groups.size; ++i) {
            for(int j = 0; j < groups.list[i].msg_cnt; ++j) {
                if(!groups.list[i].msgs[j].t) continue;
                unsigned long curr = time(NULL), elapsed;
                elapsed = curr - groups.list[i].msgs[j].ct;
                if(elapsed > groups.list[i].msgs[j].t) {
                    // TIMER HAS EXPIRED, DELETE MSG
                    // SWAP MSG TO BE DELETED WITH THE MSG STORED AT LAST INDEX AND REDUCE SIZE BY 1
                    printf("Deleting message %d from group %d\n", j, i);
                    gmsg tmsg = groups.list[i].msgs[j];
                    groups.list[i].msgs[j] = groups.list[i].msgs[groups.list[i].msg_cnt - 1];
                    groups.list[i].msgs[groups.list[i].msg_cnt - 1] = tmsg;
                    (groups.list[i].msg_cnt)--;
                }
            }
        }

        msgLen = msgrcv(serverId, &req, sizeof(request_msg) - sizeof(long), 0, IPC_NOWAIT);
        if(stop) break;
        if (!stop && msgLen != -1) {
            printf("Request Recieved\n");
            serve_request(&req);
        }
                
    }

    /* If msgrcv() fails, remove server MQ and Client MQs and exit */
    delete_msgqs(serverId);
    exit(EXIT_SUCCESS);
}