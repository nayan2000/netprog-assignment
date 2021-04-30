#include <stdio.h>       
#include <stdlib.h> 
#include <string.h>     
#include <stdbool.h>  
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>      


#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define RESET "\033[0m"

#define min(m,n) ((m) < (n) ? (m) : (n))
#define max(m,n) ((m) > (n) ? (m) : (n))

#define SERV_PORT 12345
#define MAX_PENDING 10
#define MAX_EVENTS 10

void error_exit(char*);
void tv_sub(struct timeval*, struct timeval*);
int create_server(struct sockaddr_in*);


void error_exit(char* str){
    perror(str);
    exit(EXIT_FAILURE);
}

void tv_sub(struct timeval *out, struct timeval *in){
	if((out->tv_usec -= in->tv_usec) < 0){	/* out -= in */
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

int create_server(struct sockaddr_in* servAddr){
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sfd < 0){
       error_exit(RED"socket()"RESET);
    }

    /* Creating Server Address Structure */
    memset(servAddr, 0, sizeof(*servAddr));

    servAddr->sin_family = AF_INET;
    servAddr->sin_port = htons(SERV_PORT);
    servAddr->sin_addr.s_addr = htonl(INADDR_ANY);
    
    int optval = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1){
        error_exit(RED"setsockopt()"RESET);
    }

    int b = bind(sfd, (struct sockaddr*)servAddr, sizeof(*servAddr));

    if(b < 0){
        error_exit(RED"bind()"RESET);
    }
    
    int lfd = listen(sfd, MAX_PENDING);
    if(lfd < 0){
        error_exit(RED"listen()"RESET);
    }
    return sfd;
}

int main(int argc, char* argv[]){
    
    if(argc < 2){
        error_exit(RED"usage error: ./<executable> <integer(N - number of processes)>"RESET);
    }
    int N = atoi(argv[1]);
    struct sockaddr_in servAddr;
    int lfd = create_server(&servAddr);
    signal(SIGPIPE, SIG_IGN);

    printf("\n*********SERVER STARTED*********\n");
    for(int x = 10; x <= 1000; x *= 10){
        /* Synchronise pipeline */
        int p[2];
        pipe(p);

        struct sockaddr_in cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));
                    
        int	nready, client[FD_SETSIZE], maxfd, maxi, cfd, sockfd;
        fd_set rset, allset;
        socklen_t clilen;
        maxfd = -1;			    /* initialize */
        maxi = -1;				/* index into client[] array */
        FD_ZERO(&allset);
        for (int i = 0; i < FD_SETSIZE; i++)
            client[i] = -1;		/* -1 indicates available entry */
    
        int totalfds = x; //atoi(argv[2]);
        int remainder = totalfds % N;
        
        for(int i = 0; i < N; i++){
            int nfds = totalfds/N + ((remainder - i) > 0);
            pid_t ch = fork();
            switch(ch){
                case -1:{
                    error_exit(RED"fork()"RESET);
                    break;
                }
                case 0:{ /* child */
                    close(p[1]);
                    int* cfd = (int*)malloc(sizeof(int)*nfds);
                    for(int j = 0; j < nfds; j++){
                        cfd[j] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                        if(cfd[j] < 0){
                            error_exit("socket()");
                        }
                        
                        int c = connect(cfd[j], (struct sockaddr*)&servAddr, sizeof(servAddr));
                        if(c < 0){
                            error_exit("connect");
                        }
                    }
                    int dummy;
                    read(p[0], &dummy, 1);
                    close(p[0]);
                    for(int j = 0; j < nfds; j++){
                        int sock = cfd[j];
                        close(sock);
                    }
                    _exit(0);

                    break;
                }
                default:{ /* parent */
                    while(nfds--){
                        clilen = sizeof(struct sockaddr_in);
                        cfd = accept(lfd, (struct sockaddr*) &cliaddr, &clilen);
                        if(cfd == -1 && errno == EINTR) continue;
                        // printf("New client : port %d, cfd %d\n", cliaddr.sin_port, cfd);

                        int m;
                        for (m = 0; m < FD_SETSIZE; m++){
                            if (client[m] < 0) {
                                client[m] = cfd;	    /* save descriptor */
                                break;
                            }
                        }
                        if (m == FD_SETSIZE)
                            error_exit(RED"too many clients"RESET);

                        FD_SET(cfd, &allset);	    /* add new descriptor to set */
                        if (cfd > maxfd)
                            maxfd = cfd;			/* for select */
                        if (m > maxi)
                            maxi = m;				/* max index in client[] array */
                    }
                }
            }
        }
        close(p[0]);

        // printf("Parent : maxfd - %d, maxi - %d\n", maxfd, maxi);
        close(p[1]);
        int count = totalfds;
        struct timeval st, ed;
        gettimeofday(&st, NULL);
        int i;
        for(;count > 0;){
            rset = allset;		/* structure assignment */
            struct timeval tv;
            tv.tv_sec = 5;
            tv.tv_usec = 0;

            nready = select(maxfd+1, &rset, NULL, NULL, &tv);
            if(nready == -1 && errno == EINTR) continue;
            if(nready == 0){
                printf(RED"TIMED OUT\n"RESET);
                break;
            }

            for (i = 0; i <= maxi; i++) {	/* check all clients for data */
                if((sockfd = client[i]) < 0)
                    continue;
                if(FD_ISSET(sockfd, &rset)){
                    count--;
                    char buf[20] = {0};
                    int n;
                    if((n = read(sockfd, buf, 20)) == 0){
                        close(sockfd);
                        FD_CLR(sockfd, &allset);
                        client[i] = -1;
                    }else{

                    }
                    if(--nready <= 0)
                        break;				/* no more readable descriptors */
                }
            }
        }
        gettimeofday(&ed, NULL);
        tv_sub(&ed, &st);
        float t = ed.tv_sec * 1000.0 + ed.tv_usec / 1000.0;
        printf(GREEN"select()\n"RESET);
        printf("nfds = %d, time : %.3f ms\n", totalfds, t);
    }
    
    printf(GREEN"\nWait for 60 seconds before epoll() so that\n");
    printf(GREEN"sockets opened for testing select() exit the TIME_WAIT state\n\n"RESET);
    sleep(90);
    int epoll_fd = epoll_create1(0);
    if(epoll_fd == -1){
        error_exit(RED"epoll_create1()"RESET);
	}

    for(int x = 10; x <= 1000; x *= 10){
        /* Synchronise pipeline */
        int p[2];
        pipe(p);
        signal(SIGPIPE, SIG_IGN);

        struct sockaddr_in cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));
                    
        int	nready, cfd;
        socklen_t clilen;
    
        int totalfds = x; //atoi(argv[2]);
        int remainder = totalfds % N;
        
        for(int i = 0; i < N; i++){
            int nfds = totalfds/N + ((remainder - i) > 0);
            pid_t ch = fork();
            switch(ch){
                case -1:{
                    error_exit(RED"fork()"RESET);
                    break;
                }
                case 0:{ /* child */
                    close(p[1]);
                    int* cfd = (int*)malloc(sizeof(int)*nfds);
                    for(int j = 0; j < nfds; j++){
                        cfd[j] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                        if(cfd[j] < 0){
                            error_exit("socket()");
                        }
                        
                        int c = connect(cfd[j], (struct sockaddr*)&servAddr, sizeof(servAddr));
                        if(c < 0){
                            error_exit("connect");
                        }
                    }
                    int dummy;
                    read(p[0], &dummy, 1); //wait for parent to finish accepting all connections
                    close(p[0]);
                    for(int j = 0; j < nfds; j++){
                        int sock = cfd[j];
                        close(sock);
                    }
                    _exit(0);

                    break;
                }
                default:{ /* parent */
                    while(nfds--){
                        clilen = sizeof(struct sockaddr_in);
                        cfd = accept(lfd, (struct sockaddr*) &cliaddr, &clilen);
                        if(cfd == -1 && errno == EINTR) continue;
                        // printf("New client : port %d, cfd %d\n", cliaddr.sin_port, cfd);
                        struct epoll_event ev;
                        ev.events = EPOLLIN;
                        ev.data.fd = cfd;

                        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cfd, &ev) == -1) {
                            close(epoll_fd);
                            error_exit(RED"epoll_ctl()"RESET);
		                }
                    }
                }
            }
        }
        close(p[0]);

        // printf("Parent : maxfd - %d, maxi - %d\n", maxfd, maxi);
        // printf("Start monitoring epoll() time..\n");
        close(p[1]);
        struct epoll_event ev_list[MAX_EVENTS];
        int count = totalfds;
        struct timeval st, ed;
        gettimeofday(&st, NULL);
        int i;
        for(;count > 0;){
            nready = epoll_wait(epoll_fd, ev_list, MAX_EVENTS, 100);
            if (nready == -1){ 
                if (errno == EINTR) 
                    continue; /* Restart if interrupted by signal */ 
                else 
                    error_exit(RED"epoll_wait()"RESET); 
            }        
            if(nready == 0) {
                // printf(GREEN"TIMED OUT!\n"RESET);
                break;
            }

            for (i = 0; i < nready; ++i){
			    if(ev_list[i].events & EPOLLIN){
                    int sockfd = ev_list[i].data.fd; 
                    count--;
                    char buf[20] = {0};
                    int n;
                    if((n = read(sockfd, buf, 20)) == 0){
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, &ev_list[i]);
					    close(sockfd);
                    }else{

                    }
                }
            }
        }
        gettimeofday(&ed, NULL);
        tv_sub(&ed, &st);
        float t = ed.tv_sec * 1000.0 + ed.tv_usec / 1000.0;
        printf(GREEN"epoll()\n"RESET);
        printf("nfds = %d, time : %.3f ms\n", totalfds, t);
    }
    exit(EXIT_SUCCESS);     
}