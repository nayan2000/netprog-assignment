#include "msgq_client.h"

int getReqSize(request_msg* req) {
    return sizeof(req->client_qid) + sizeof(req->uname) + sizeof(req->command) + sizeof(req->args) + sizeof(req->data);
}

int main() {
    int ch = -1;
    // SETUP MESSAGE QUEUE
    char uid[20];
    printf("Enter your user ID: ");
    scanf("%s", uid);
    
    // ATTEMPT TO CREATE NEW MSGQ
    int clientId = msgget(uid, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWGRP);
    if(clientId == -1 && errno == EEXIST) {
        // MSGQ ALREADY EXISTS, RETRIEVE
        clientId = msgget(uid, 0);
        printf("New user, creating message queue...\n");
    } else {
        printf("Welcome back %s!\n", uid);
    }

    // MAIN LOOP
    while(1) {
        request_msg req;
        req.client_qid = clientId;

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
    }

    return 0;
}