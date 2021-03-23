#include "setup_utilities.h"

void grim_reaper(int sig) { 
    int savedErrno; 
    savedErrno = errno; 
    while (waitpid(-1, NULL, WNOHANG) > 0) continue; 
    errno = savedErrno; 
}

int server_setup(char* server_port){    
    int lfd, optval;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    struct sigaction sa;
    
    /* Ignore the SIGPIPE signal, so that we find out about broken connection
       errors via a failure from write(). */

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR){
        fprintf(stderr, RED"FATAL ERROR : CANT'T SET SIGNAL DISPOSITION\n"RESET);
    }
    sigemptyset(&sa.sa_mask); 
    sa.sa_flags = SA_RESTART; 
    sa.sa_handler = grim_reaper; 
    if (sigaction(SIGCHLD, &sa, NULL) == -1) { 
        fprintf(stderr, RED"FATAL ERROR : CANT'T SET SIGNAL DISPOSITION\n"RESET);
    }

    /* Call getaddrinfo() to obtain a list of addresses that
       we can try binding to */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;        /* Allows IPv4 or IPv6 */
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV; /* Wildcard IP address; service name is numeric */

    if (getaddrinfo(NULL, server_port, &hints, &result) != 0){
        fprintf(stderr, RED"FATAL ERROR : CANT'T GET LIST OF IPs\n"RESET);
    }

    /* Walk through returned list until we find an address structure
       that can be used to successfully create and bind a socket */

    optval = 1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (lfd == -1)
            continue;                   /* On error, try next address */

        if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
            fprintf(stderr, RED"FATAL ERROR : CANT'T SET SOCKET OPTn"RESET);

        if (bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                      /* Success */

        /* bind() failed: close this socket and try next address */
        close(lfd);
    }

    if(rp == NULL){
        fprintf(stderr, RED"FATAL ERROR : CANT'T BIND ANY SOCKET\n"RESET);
        exit(EXIT_FAILURE);

    }

    if(listen(lfd, BACKLOG) == -1){
        fprintf(stderr, RED"FATAL ERROR : CANT'T LISTEN ON ANY SOCKET\n"RESET);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);
    return lfd;
}   


int client_setup(char* server_ip, char* server_port){
    int cfd;
    ssize_t numRead;
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    /* Call getaddrinfo() to obtain a list of addresses that
       we can try connecting to */

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;                /* Allows IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    if (getaddrinfo(server_ip, server_port, &hints, &result) != 0)
        fprintf(stderr, RED"FATAL ERROR : CANT'T GET LIST OF IPs\n"RESET);

    /* Walk through returned list until we find an address structure
       that can be used to successfully connect a socket */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (cfd == -1)
            continue;                           /* On error, try next address */

        if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                              /* Success */

        /* Connect failed: close this socket and try next address */

        close(cfd);
    }

    if (rp == NULL){
        fprintf(stderr, RED"FATAL ERROR : CANT'T CONNECT ON ANY SOCKET\n"RESET);
        exit(EXIT_FAILURE);
    }
        

    freeaddrinfo(result);
    return cfd;
}