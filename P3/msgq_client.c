#include "msgq_client.h"

int getReqSize(request_msg* req) {
    return sizeof(req->client_qid) + sizeof(req->uname) + sizeof(req->command) + sizeof(req->args) + sizeof(req->data);
}

int getResSize(response_msg* res) {
    return sizeof(res->data);
}

int main() {
    int ch = -1;
    // SETUP MESSAGE QUEUE
    char uname[20];
    printf("Enter your username: ");
    scanf("%s", uname);
    
    // CREATE NEW MSGQ
    int clientId = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWGRP);
    request_msg initreq;
    initreq.client_qid = clientId;
    strcpy(initreq.uname, uname);
    initreq.command = 'n';
    msgsnd(SERVER_KEY, &initreq, getReqSize(&initreq), IPC_NOWAIT);

    // CHILD - HANDLE MESSAGES
    //int p[2]; // 0 - READ, 1 - WRITE
    //pipe(p);
    if(!fork()) {
        //close(1);
        //dup(p[1]);

        response_msg res;

        while(1) {
            msgrcv(clientId, &res, getResSize(&res), RESP_MT_DATA, 0);
            printf("%s", res.data);
        }

    }
    //close(0);
    //dup(p[0]);

    // MAIN LOOP
    while(1) {
        request_msg req;
        req.client_qid = clientId;

        response_msg res;

        printf("%s\n", "-----------------------------------------------");
        printf("%s\n", "                  CLIENT MENU                  ");
        printf("%s\n", "-----------------------------------------------");
        printf("%s\n", "1. Create new group\n");
        printf("%s\n", "2. List groups\n");
        printf("%s\n", "3. Join group\n");
        printf("%s\n", "4. Send message to user\n");
        printf("%s\n", "5. Send message to group\n");
        printf("%s\n", "6. Exit\n");
        scanf("%d", &ch);
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
            break;
        } else {
            printf("Invalid option. Try again\n");
        }

        msgsnd(SERVER_KEY, &req, getReqSize(&req), IPC_NOWAIT);

        // WAIT FOR REPLY FROM SERVER
        msgrcv(clientId, &res, getResSize(&res), -5, 0);

        printf("%s", res.data);

    }

    return 0;
}