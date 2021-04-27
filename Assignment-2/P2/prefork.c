#include "prefork.h"

static int maxIdleServers, minIdleServers, maxRequestsPerChild;
child_details** cd;
static int maxIndex = -1;
static int nidle, nchildren;

void print_details(char* action, int i){
    printf(GREEN"Total Active Children : %d"RESET"\n", nchildren);
    printf(GREEN"Total Idle Children : %d"RESET"\n", nidle);
    int numCl = 0;
    for(int j = 0; j <= maxIndex; j++){
        if(cd[j] && cd[j]->status == 1){
            numCl++;
        }
    }
    printf(GREEN"Number of clients being Handled : %d"RESET"\n", numCl);
    printf(GREEN"Action : %s\n"RESET, action);
    if(!strcmp(action, "MARK BUSY")){
        printf(GREEN"Child %d accepted a client"RESET"\n", cd[i]->pid);
    }
    else if(!strcmp(action, "MARK FREE")){
        printf(GREEN"Child %d finished a request"RESET"\n", cd[i]->pid);
    }
    else if(!strcmp(action, "RECYCLE CHILD")){
        printf(GREEN"Child %d recycled"RESET"\n", cd[i]->pid);
    }
    else if(!strcmp(action, "ADD TO POOL") && i != -1){
        printf(GREEN"Child %d added"RESET"\n", cd[i]->pid);
    }
    else if(!strcmp(action, "REMOVE FROM POOL") && i != -1){
        printf(GREEN"Child %d removed"RESET"\n", cd[i]->pid);
    }
    printf("\n");
}

void signal_handler(int sig){
    if(sig == SIGINT){
        printf("Total Active Children : %d\n", nchildren);
        printf("Total Idle Children: %d\n", nidle);
        printf(YELLOW"%8s | %s", "PID", "CLIENTS HANDLED\n"RESET);
        for(int i = 0; i <= maxIndex; i++){
            if(cd[i]){
                printf(GREEN"%8d | %5ld\n"RESET, cd[i]->pid, cd[i]->count);
            }
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

void handle_request(int connfd){
    char reply[] = 
    "HTTP/1.1 200 OK\n"
    "Date: Thu, 19 Feb 2009 12:27:04 GMT\n"
    "Server: Apache/2.2.3\n"
    "Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\n"
    "Content-Type: text/html\n"
    "Content-Length: 15\n"
    "Accept-Ranges: bytes\n"
    "Connection: close\n"
    "\n"
    "dummy\n";

    // printf("Client handle by %d\n", getpid());
    sleep(1);
    // send(connfd, reply, strlen(reply), 0);
}

void main_child(int j, int fd, int listenfd, socklen_t addrlen){
    int connfd;
    socklen_t clilen;
    struct sockaddr *cliaddr;
    cliaddr = (struct sockaddr*)malloc(addrlen);
    sleep(1);
    // printf("Child %d : PID %ld starting\n", j, (long) getpid());
    for(int i = 0; i < maxRequestsPerChild; i++) {
        clilen = addrlen;
        // connfd = accept(listenfd, cliaddr, &clilen);
        if(connfd == -1 && errno == EINTR) continue;

        write(fd, "b", 1);
        handle_request(connfd);
        close(connfd);
        if(i < maxRequestsPerChild - 1)
            write(fd, "f", 1);
    }
    close(fd);
}

pid_t make_child(int i, int listenfd, int addrlen){ 
    int sockfd[2]; 
    pid_t pid; 
    child_details* det = (child_details*)malloc(sizeof(child_details)); 
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
    

    close(sockfd[0]);
    main_child(i, sockfd[1], listenfd, addrlen);
    _exit(0);
}

int main(int argc, char **argv){ 
    int listenfd, i;
    socklen_t addrlen;

    if(argc < 4){
        printf(RED"USAGE ERROR\n"RESET);
        printf(GREEN"Usage : ./prefork <maxIdleServers> <minIdleServers> <maxRequestsPerChild>\n"RESET);
        exit(0);
    }

    listenfd = inet_listen(SERV_PORT, BACKLOG, &addrlen); 
    if(listenfd == -1){
        perror(RED"Listen Error"RESET);
        exit(EXIT_FAILURE);
    }

    maxIdleServers = atoi(argv[1]);
    minIdleServers = atoi(argv[2]);
    maxRequestsPerChild = atoi(argv[3]); 
    cd = (child_details**)malloc(sizeof(child_details*) * (2*maxIdleServers + 33));
    for(int i = 0; i < 2*maxIdleServers + 33; i++)
        cd[i] = NULL;
    pid_t ch;
    fd_set rset, masterset;
    FD_ZERO(&masterset);
    FD_ZERO(&rset);
    int maxfd = -1;

    nchildren = maxIdleServers;

    for (i = 0; i < nchildren; i++){
        make_child(i, listenfd, addrlen); /* parent returns */ 
        FD_SET(cd[i]->pipefd, &masterset); 
        maxfd = max(maxfd, cd[i]->pipefd);
        maxIndex = max(maxIndex, i);
    }
    for(int i = 0; i < 2*maxIdleServers+33; i++){
        if(cd[i]){
            printf("%d, %d\n", cd[i]->pid, cd[i]->pipefd);
        }
    }
    print_details("ADD TO POOL", -1);
    int n;
    char rc[2] = {0};
    nidle = nchildren;
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);
    for(;;){
        rset = masterset;
        int nsel = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if(nsel == -1 && errno == EINTR) continue;
        for(i = 0; i <= maxIndex; i++){
            if(cd[i] && FD_ISSET(cd[i]->pipefd, &rset)){
                if((n = read(cd[i]->pipefd, rc, 1)) == 0){ /* Child has exited */
                    FD_CLR(cd[i]->pipefd, &masterset);
                    nchildren--;
                    cd[i]->count = 0;
                    cd[i]->status = 0;
                    print_details("RECYCLE CHILD", i);
                    free(cd[i]);
                    cd[i] = NULL;
                }
                else if(n == -1){
                    perror(RED"READ ERROR"RESET);
                }
                else if(rc[0] == 'b'){
                    nidle--;
                    cd[i]->count++;
                    cd[i]->status = 1;
                    print_details("MARK BUSY", i);
                }
                else if(rc[1] == 'f'){
                    nidle++;
                    cd[i]->count--;
                    cd[i]->status = 0;
                    print_details("MARK FREE", i);
                }
                
            }
            int limit = 1;
            while(nidle < minIdleServers){
                for(int spawn = 0; spawn < limit; spawn++){
                    int l = get_free_index();
                    make_child(l, listenfd, addrlen); /* parent returns */ 
                    FD_SET(cd[l]->pipefd, &masterset); 
                    maxfd = max(maxfd, cd[l]->pipefd);
                    maxIndex = max(maxIndex, l);
                    nidle++;
                    nchildren++;
                    print_details("ADD TO POOL", l);
                }
                limit = min(32, 2*limit);
                sleep(1);
            }

            int l = 0;
            while(nidle > maxIdleServers){
                if(cd[l] && cd[l]->status == 0){
                    kill(cd[l]->pid, SIGTERM);
                    FD_CLR(cd[l]->pipefd, &masterset);
                    nidle--;
                    nchildren--;
                    cd[l]->count = 0;
                    cd[l]->status = 0;
                    print_details("REMOVE FROM POOL", l);
                    free(cd[l]);
                    cd[l] = NULL;
                }
                l++;
            }
            if(--nsel == 0) break; 
        }
    }
}