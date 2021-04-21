#ifndef PING_H
#define PING_H

#include "basic.h"
#include "hashmap.h"

#define	BUFSIZE	1500
#define MAX_EVENTS  10          // epoll_wait() list
#define EPOLL_TIMEOUT   3000    // Timeout for epoll_wait()
#define HASH_DIM 100
#define FILENAME "ip"


typedef struct pthread_args {
    hashmap* hm;
    int     epoll_fd;
}pthread_args;
typedef struct proto {
  void	 (*fproc)(char *, ssize_t, struct msghdr *, struct timeval *, host_det*);
  void	 (*fsend)(int, struct proto*);
  void	 (*finit)(int);
  struct sockaddr  *sasend;	/* sockaddr{} for send, from getaddrinfo */
  struct sockaddr  *sarecv;	/* sockaddr{} for receiving */
  socklen_t	    salen;		/* length of sockaddr{}s */
  int	   	    icmpproto;	/* IPPROTO_xxx value for ICMP */
}proto;

/* function prototypes */
void err_exit(char*);
struct addrinfo* host_serv(const char *, const char *, int, int);
char* sock_ntop_host(const struct sockaddr *, socklen_t);
uint16_t in_cksum(uint16_t *, int);
void init_v6(int);
void proc_v4(char *, ssize_t, struct msghdr *, struct timeval *, host_det*);
void proc_v6(char *, ssize_t, struct msghdr *, struct timeval *, host_det*);
void send_v6(int, struct proto*);
void send_v4(int, struct proto*);
// void readloop(void);
void tv_sub(struct timeval *, struct timeval *);


#endif
