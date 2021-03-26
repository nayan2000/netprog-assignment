#include "msgq_client.h"
int clientId;
void sighandler(int sig){
    msgctl(clientId, IPC_RMID, NULL);
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
    msgrcv(clientId, &res, RESP_MSG_SIZE, 0, 0);

    if(res.mtype == RESP_MT_USER_EXIST){
        msgctl(clientId, IPC_RMID, NULL);
        clientId = atoi(res.data);
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
    while(1) {
        request_msg req;
        req.client_qid = clientId;

        req.mtype = 1;
        printf("%s\n", "-----------------------------------------------");
        printf("%s\n", "                  CLIENT MENU                  ");
        printf("%s\n", "-----------------------------------------------");
        printf("%s\n", "1. Create new group\n");
        printf("%s\n", "2. List groups\n");
        printf("%s\n", "3. Join group\n");
        printf("%s\n", "4. Send message to user\n");
        printf("%s\n", "5. Send message to group\n");
        printf("%s\n", "6. Exit\n");
        printf(GREEN">> "RESET);
        fflush(stdout);
        scanf("%d", &ch);
        if((char)ch == '\n') continue;
        fflush(stdout);
        if(ch == 1) {
            char gname[MAX_SIZE];
            printf("Enter group name to create: ");
            scanf("%s", gname);
            strcpy(req.data, gname);
            req.command = 'c';
        } else if(ch == 2) {
            req.command = 'l';
        } else if(ch == 3) {
            char gname[MAX_SIZE];
            printf("Enter group name to join: ");
            scanf("%s", gname);
            strcpy(req.data, gname);
            req.command = 'j';
        } else if(ch == 4) {
            char uname[MAX_SIZE];
            char msg[MAX_SIZE];
            printf("Enter user name to send message to: ");
            scanf("%s", uname);
            printf("Enter message to send: ");
            scanf("%s", msg);
            strcpy(req.args, uname);
            strcpy(req.data, msg);
            req.command = 'u';
        } else if(ch == 5) {
            char gname[MAX_SIZE];
            char msg[MAX_SIZE];
            printf("Enter group name to send message to: ");
            scanf("%s", gname);
            printf("Enter message to send: ");
            scanf("%s", msg);
            strcpy(req.args, gname);
            strcpy(req.data, msg);
            req.command = 'g';
        } else if(ch == 6) {
            exit(0);
            // kill(child, SIGTERM);
        } else{
            printf("Invalid option. Try again\n");
        }

        msgsnd(serverId, &req, sizeof(request_msg) - sizeof(long), IPC_NOWAIT);

        // WAIT FOR REPLY FROM SERVER
        msgrcv(clientId, &res, RESP_MSG_SIZE, 0, 0);

        printf("%s", res.data);

    }

    return 0;
}