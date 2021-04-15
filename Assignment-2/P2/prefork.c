#include "prefork.h"

static int maxIdleServers, minIdleServers, maxRequestsPerChild;
child_details** cd;
static int maxIndex = -1;
static int nidle, nchildren;

void print_details(char* action, int i){
    printf("Total Active Children : %d\n", nchildren);
    int numCl = 0;
    for(int j = 0; j <= maxIndex; j++){
        if(cd[j] && cd[j]->status == 1){
            numCl++;
        }
    }
    printf("Number of clients being Handled : %d\n", numCl);
    printf("Action : %s\n", action);
    if(!strcmp(action, "MARK BUSY")){
        printf("Child %d accepted a client\n", cd[i]->pid);
    }
    else if(!strcmp(action, "MARK FREE")){
        printf("Child %d finished a request\n", cd[i]->pid);
    }
    else if(!strcmp(action, "RECYCLE CHILD")){
        printf("Child %d recycled\n", cd[i]->pid);
    }
    else if(!strcmp(action, "ADD TO POOL") && i != -1){
        printf("Child %d added\n", cd[i]->pid);
    }
    else if(!strcmp(action, "REMOVE FROM POOL") && i != -1){
        printf("Child %d removed\n", cd[i]->pid);
    }
}

void signal_handler(int sig){
    printf("Total Active Children : %d\n", nchildren);
    printf("Total Idle Children: %d\n", nidle);
    printf(YELLOW"%8s | %s", "PID", "CLIENTS HANDLED\n"RESET);
    for(int i = 0; i <= maxIndex; i++){
        if(cd[i]){
            printf(GREEN"%8d | %5d\n"RESET, cd[i]->pid, cd[i]->count);
        }
    }
}

int get_free_index(){
    for(int i = 0; i < 2*maxIdleServers + 33; i++){
        if(cd[i] == NULL)
            return i;
    }
    return -1;
}

void main_child(int j, int listenfd, socklen_t addrlen){
    int connfd;
    socklen_t clilen;
    struct sockaddr *cliaddr;
    cliaddr = (struct sockaddr*)malloc(addrlen);

    printf("Child %d : PID %ld starting\n", j, (long) getpid());
    for(int i = 0; i < maxRequestsPerChild; i++) {
        clilen = addrlen;
        connfd = accept(listenfd, cliaddr, &clilen);
        write(STDERR_FILENO, 'b', 1);
        if(connfd == -1 && errno == EINTR) continue;
        // handle_request(connfd);
        close(connfd);
        write(STDERR_FILENO, 'f', 1);
    }
}

pid_t make_child(int i, int listenfd, int addrlen){ 
    int sockfd[2]; 
    pid_t pid; 
    child_details* det = (child_details*)mallox(sizeof(child_details)); 
    socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd); 
    if((pid = fork()) > 0){ /* parent */ 
        close(sockfd[1]);
        det->pid = pid; 
        det->pipefd = sockfd[0];
        det->count = 0;
        det->status = 0;
        cd[i] = det;
        return pid; 
    }
    if(sockfd[1] != STDERR_FILENO){
        dup2(sockfd[1], STDERR_FILENO); /* child's stream pipe to parent */ 
        close(sockfd[1]);
    }

    close(sockfd[0]);
    main_child(i, listenfd, addrlen);
    _exit(0);
}

int main(int argc, char **argv){ 
    int listenfd, i;
    socklen_t addrlen;

    if(argc != 4){
        printf(RED"USAGE ERROR\n"RESET);
        printf(YELLOW"Usage : ./<executable> <maxIdleServers> <minIdleServers> <maxRequestsPerChild>"RESET);
    }

    listenfd = inet_listen(SERV_PORT, BACKLOG, &addrlen); 
    if(listenfd == -1){
        perror(RED"Listen Error"RESET);
        exit(EXIT_FAILURE);
    }

    maxIdleServers = atoi(argv[1]);
    minIdleServers = atoi(argv[2]);
    maxRequestsPerChild = atoi(argv[3]); 
    nchildren = maxIdleServers;
    cd = (child_details**)malloc(sizeof(child_details*) * 2*maxIdleServers + 33);

    pid_t ch;
    fd_set rset, masterset;
    FD_ZERO(&masterset);
    FD_ZERO(&rset);
    int maxfd = -1;

    for (i = 0; i < nchildren; i++){
        make_child(i, listenfd, addrlen); /* parent returns */ 
        FD_SET(cd[i]->pipefd, &masterset); 
        maxfd = max(maxfd, cd[i]->pipefd);
        maxIndex = max(maxIndex, i);
    }
    print_details("ADD TO POOL", -1);
    /* find any newly-available children */ 
    int n;
    char rc;
    nidle = nchildren;
    for(;;){
        signal(SIGINT, signal_handler);
        rset = masterset;
        int nsel = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if(nsel == -1 && errno == EINTR) continue;
        for(i = 0; i <= maxIndex; i++){
            if(cd[i] && FD_ISSET(cd[i]->pipefd, &rset)){
                if((n = read(cd[i]->pipefd, &rc, 1)) == 0){
                    FD_CLR(cd[i]->pipefd, &masterset);
                    nchildren--;
                    print_details("RECYCLE CHILD", i);
                    free(cd[i]);
                    cd[i] = NULL;
                    
                }
                if(rc == 'b'){
                    nidle--;
                    cd[i]->count++;
                    cd[i]->status = 1;
                    print_details("MARK BUSY", i);
                }
                else if(rc == 'f'){
                    nidle++;
                    cd[i]->count--;
                    cd[i]->status = 0;
                    print_details("MARK FREE", i);
                }
                if(--nsel == 0) break; /* all done with select() results */
            } 
        }
        int limit = 1;
        while(nidle < minIdleServers){
            for(int spawn = 0; spawn < limit; spawn++){
                i = get_free_index();
                make_child(i, listenfd, addrlen); /* parent returns */ 
                FD_SET(cd[i]->pipefd, &masterset); 
                maxfd = max(maxfd, cd[i]->pipefd);
                maxIndex = max(maxIndex, i);
                nidle++;
                print_details("ADD TO POOL", i);
            }
            limit = min(32, 2*limit);
            sleep(1);
        }

        i = 0;
        while(nidle > maxIdleServers){
            if(cd[i] && cd[i]->status == 0){
                kill(cd[i]->pid, SIGTERM);
                FD_CLR(cd[i]->pipefd, &masterset);
                nidle--;
                nchildren--;
                print_details("REMOVE FROM POOL", i);
                free(cd[i]);
                cd[i] = NULL;
            }
            i++;
        }
    }
}