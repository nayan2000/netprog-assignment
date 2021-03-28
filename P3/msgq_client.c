#include "msgq_client.h"
int clientId;
int stop = false;
void do_nothing(int sig){
    if(sig == SIGUSR2)
        stop = true;
}
void exit_handler(){
    msgctl(clientId, IPC_RMID, NULL);
}
/* Read characters from 'fd' until a newline is encountered. If a newline
  character is not encountered in the first (n - 1) bytes, then the excess
  characters are discarded. The returned string placed in 'buf' is
  null-terminated and includes the newline character if it was read in the
  first (n - 1) bytes. The function return value is the number of bytes
  placed in buffer (which includes the newline character if encountered,
  but excludes the terminating null byte). */

ssize_t read_line(int fd, void *buffer, size_t n){
    ssize_t numRead;                    /* # of bytes fetched by last read() */
    size_t totRead;                     /* Total bytes read so far */
    char *buf;
    char ch;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = buffer;                       /* No pointer arithmetic on "void *" */

    totRead = 0;
    for (;;) {
        numRead = read(fd, &ch, 1);

        if (numRead == -1) {
            if (errno == EINTR)         /* Interrupted --> restart read() */
                continue;
            else
                return -1;              /* Some other error */

        } else if (numRead == 0) {      /* EOF */
            if (totRead == 0)           /* No bytes read; return 0 */
                return 0;
            else                        /* Some bytes read; add '\0' */
                break;

        } else {   
            if (ch == '\n')
                break;                  /* 'numRead' must be 1 if we get here */
            if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
                totRead++;
                *buf++ = ch;
            }
            
        }
    }

    *buf = '\0';
    return totRead;
}

int main() {
    signal(SIGUSR1, do_nothing);
    setbuf(stdout, NULL);
    
    char uname[20] = {0};

    /* Enter Username to login */
    printf("Enter your username: ");
    scanf("%s", uname);
    printf("%s - uname\n", uname);

    /* Get Server Message Queue */
    int serverId = msgget(SERVER_KEY, 0755);
    if (serverId == -1){
        perror(RED"server:msgget"RESET);
        exit(EXIT_FAILURE);
    }

    /* Setup Client Message Queue */
    clientId = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0777);

    /* Create Initial Message to inform server of the client queue */
    request_msg initreq;
    bzero(&initreq, sizeof(initreq));

    initreq.mtype = 1;
    initreq.client_qid = clientId;
    strcpy(initreq.uname, uname);
    initreq.command = 'n';
    
    /* Send message to server containing Client Queue ID*/    
    if(msgsnd(serverId, &initreq, sizeof(request_msg)-sizeof(long), 0) == -1){
        perror("initial msgsnd to server");
        exit(EXIT_FAILURE);
    }

    response_msg res;
    bzero(&res, sizeof(res));

    /* Wait for Server response */
    /* 1. Client MSGQ exists? 
            -Delete the newly formed queue
            -Retrieve messages when client was offline */
    /* 2. Client MSGQ does not exist?
            - Welcome Client to the sysytem */
    msgrcv(clientId, &res, RESP_MSG_SIZE, -2, 0);

    
    if(res.mtype == RESP_MT_CHECK_USER_EXIST){ /* User logged out and then logs in again */ 
        msgctl(clientId, IPC_RMID, NULL);
        clientId = atoi(res.data);

        /* Retrieve all pending messages */
        while(msgrcv(clientId, &res, RESP_MSG_SIZE, 0, IPC_NOWAIT) != -1){
            
            printf(YELLOW"%s\n"RESET, res.data);
        }
    }
    if(res.mtype == RESP_MT_CHECK_USER_NO_EXIST){ /* New User */
        printf(GREEN"Welcome to the message system !\n"RESET);
    }

    int ch = -1;
    char* msg = (char*) malloc(MAX_SIZE);
    /* Create Child to continuously read messages */
    pid_t child = fork();
    if(child == 0) {
        signal(SIGUSR2, do_nothing);
        response_msg res;
        bzero(&res, sizeof(res));
        while(1){
            if(!stop && msgrcv(clientId, &res, RESP_MSG_SIZE, 0, 0) != -1 && !stop){
                printf(GREEN"---"RESET"\n");
                printf(YELLOW"%s"RESET"\n", res.data);
            }
            if(stop) break;
            while(!stop && msgrcv(clientId, &res, RESP_MSG_SIZE, 0, IPC_NOWAIT) != -1){
                printf(GREEN"---"RESET"\n");
                printf(YELLOW"%s"RESET"\n", res.data);
            }
            if(stop) break;
            kill(getppid(), SIGUSR1);            
            
        }
        _exit(0);
    }
   
    /* Main Loop */
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
        printf("%s\n", "4. List group messages\n");
        printf("%s\n", "5. Send message to user\n");
        printf("%s\n", "6. Send message to group\n");
        printf("%s\n", "7. Logout\n");
        printf("%s\n", "8. Deregister\n");
        printf(GREEN"\n>> "RESET);
        fflush(stdout);
        char opt[3] = {0};
        read_line(STDIN_FILENO, opt, 3);
        if(strlen(opt) == 0) continue;
        int ch = atoi(opt);
        fflush(stdin);

        if(ch == 1) {
            // CREATE NEW GROUP
            char gname[MAX_SIZE] = {0};
            printf("Enter group name to create: ");
            fflush(stdout);
            read_line(STDIN_FILENO, gname, MAX_SIZE);
            strcpy(req.data, gname);
            req.command = 'c';
        }else if(ch == 2){
            // LIST GROUPS
            req.command = 'l';
        }else if(ch == 3){
            // JOIN GROUP
            char gname[MAX_SIZE] = {0};
            printf("Enter group name to join: ");
            fflush(stdout);
            read_line(STDIN_FILENO, gname, MAX_SIZE);
            strcpy(req.data, gname);
            req.command = 'j';
        }else if(ch == 4){
            // LIST GROUP MSGS
            char gname[MAX_SIZE] = {0};
            printf("Enter group name to display messages from: ");
            fflush(stdout);
            read_line(STDIN_FILENO, gname, MAX_SIZE);
            strcpy(req.data, gname);
            req.command = 's';
        } else if(ch == 5) {
            // SEND MSG TO USR
            char uname[MAX_SIZE] = {0};
            printf("Enter user name to send message to: ");
            fflush(stdout);
            read_line(STDIN_FILENO, uname, MAX_SIZE);
            printf("Enter message to send: ");
            fflush(stdout);
            read_line(STDIN_FILENO, msg, MAX_SIZE);
            strcpy(req.args, uname);
            strcpy(req.data, msg);
            req.command = 'u';
        }else if(ch == 6){
            // SEND MSG TO GROUP
            char gname[MAX_SIZE] = {0};
            printf("Enter group name to send message to: ");
            fflush(stdout);
            read_line(STDIN_FILENO, gname, MAX_SIZE);
            printf("Enter message to send: ");
            fflush(stdout);
            read_line(STDIN_FILENO, msg, MAX_SIZE);
            printf("Auto delete timer duration(0 for no timer): ");
            scanf("%d", &req.t);
            strcpy(req.args, gname);
            strcpy(req.data, msg);
            req.command = 'g';
        }else if(ch == 7){
            // LOGOUT
            kill(child, SIGUSR2);
            break;
        } else if(ch == 8) {
            // DEREGISTER
            request_msg exit_req;
            exit_req.client_qid = clientId;
            strcpy(exit_req.uname, uname);
            exit_req.mtype = 1;
            exit_req.command = 'r';
            msgsnd(serverId, &exit_req, sizeof(request_msg) - sizeof(long), IPC_NOWAIT);
            kill(child, SIGUSR2);
            break;

        } else{
            printf("Invalid option. Try again\n");
            fflush(stdout);
            continue;
        }

        msgsnd(serverId, &req, sizeof(request_msg) - sizeof(long), IPC_NOWAIT);
        pause();
        
        fflush(stdout);

    }
    if(ch == 6){
        exit(0);
    }
    if(ch == 7){ 
        atexit(exit_handler);
        exit(0);
    }

    return 0;
}