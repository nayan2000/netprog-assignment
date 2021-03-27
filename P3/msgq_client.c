#include "msgq_client.h"
int clientId;
void sighandler(int sig){
    msgctl(clientId, IPC_RMID, NULL);
    raise(SIGTERM);
}
void exit_handler(){
    msgctl(clientId, IPC_RMID, NULL);
}
int getReqSize(request_msg* req) {
    return sizeof(req->client_qid) + sizeof(req->uname) + sizeof(req->command) + sizeof(req->args) + sizeof(req->data);
}

int getResSize(response_msg* res) {
    return sizeof(res->data);
}

int main() {
    setbuf(stdout, NULL);
    atexit(exit_handler);
    signal(SIGINT, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGTSTP, sighandler);
    int ch = -1;
    // SETUP MESSAGE QUEUE
    char uname[20] = {0};
    printf("Enter your username: ");
    scanf("%s", uname);
    printf("%s - uname\n", uname);
    // CREATE NEW MSGQ
    clientId = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0777);
    request_msg initreq;
    bzero(&initreq, sizeof(initreq));
    initreq.mtype = 1;

    initreq.client_qid = clientId;
    strcpy(initreq.uname, uname);
    initreq.command = 'n';
    int serverId = msgget(SERVER_KEY, 
                            0755);

    if (serverId == -1){
        perror(RED"server:msgget"RESET);
        exit(0);
    }

    if(msgsnd(serverId, &initreq, sizeof(request_msg)-sizeof(long), 0) == -1){
        perror("msgsnd to server");
        exit(0);
    }

    response_msg res;
    bzero(&res, sizeof(res));

    msgrcv(clientId, &res, RESP_MSG_SIZE, -2, 0);

    /* User logged out and then logs in again */ 
    if(res.mtype == RESP_MT_CHECK_USER_EXIST){ 
        msgctl(clientId, IPC_RMID, NULL);
        clientId = atoi(res.data);

        /* Retrieve all pending messages */
        while(msgrcv(clientId, &res, RESP_MSG_SIZE, 0, IPC_NOWAIT) != -1){
            
            printf("%s\n", res.data);
        }
    }
    if(res.mtype == RESP_MT_CHECK_USER_NO_EXIST){ /* New User */
        
    }
    // CHILD - HANDLE MESSAGES
    // pid_t child = fork();
    // if(child == 0) {
    //     response_msg res;

    //     while(1){
    //         msgrcv(clientId, &res, RESP_MSG_SIZE, 0, 0);
    //         printf("%s\n", res.data);
    //     }

    // }
    // MAIN LOOP
    while(1){
        request_msg req;
        bzero(&req, sizeof(req));

        req.client_qid = clientId;
        strcpy(req.uname, uname);
        req.mtype = 1;
        printf("%s\n", "-----------------------------------------------");
        printf("%s\n", "                  CLIENT MENU                  ");
        printf("%s\n", "-----------------------------------------------");
        printf("%s\n", "1. Create new group\n");
        printf("%s\n", "2. List groups\n");
        printf("%s\n", "3. Join group\n");
        printf("%s\n", "4. Send message to user\n");
        printf("%s\n", "5. Send message to group\n");
        printf("%s\n", "6. Logout\n");
        printf("%s\n", "7. Deregister\n");
        printf(GREEN">> "RESET);
        fflush(stdout);
        scanf("%d", &ch);
        if((char)ch == '\n') continue;
        fflush(stdin);
        fflush(stdout);
        if(ch == 1) {
            char gname[MAX_SIZE] = {0};
            printf("Enter group name to create: ");
            scanf("%s", gname);
            strcpy(req.data, gname);
            req.command = 'c';
        }else if(ch == 2){
            req.command = 'l';
        }else if(ch == 3){
            char gname[MAX_SIZE] = {0};
            printf("Enter group name to join: ");
            scanf("%s", gname);
            strcpy(req.data, gname);
            req.command = 'j';
        }else if(ch == 4){
            char uname[MAX_SIZE] = {0};
            char msg[MAX_SIZE] = {0};
            printf("Enter user name to send message to: ");
            
            scanf("%s", uname);
            printf("Enter message to send: ");
            scanf("%s", msg);
            strcpy(req.args, uname);
            strcpy(req.data, msg);
            req.command = 'u';
        }else if(ch == 5){
            char gname[MAX_SIZE] = {0};
            char msg[MAX_SIZE] = {0};
            printf("Enter group name to send message to: ");
            scanf("%s", gname);
            printf("Enter message to send: ");
            scanf("%s", msg);
            strcpy(req.args, gname);
            strcpy(req.data, msg);
            req.command = 'g';
        }else if(ch == 6){
            break;
            
            // kill(child, SIGTERM);
        }else if(ch == 7){
            break;
            // kill(child, SIGTERM);
        }
        else{
            printf("Invalid option. Try again\n");
        }

        msgsnd(serverId, &req, sizeof(request_msg) - sizeof(long), IPC_NOWAIT);

        // WAIT FOR REPLY FROM SERVER
        msgrcv(clientId, &res, RESP_MSG_SIZE, 0, 0);

        printf("%s", res.data);
        fflush(stdout);

    }
    if(ch == 6){
        _exit(0);
    }
    if(ch == 7){
        exit(0);
    }

    return 0;
}