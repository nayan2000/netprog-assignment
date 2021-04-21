#include "rtt.h"
static int datalen = 56;		/* Data that goes with ICMP echo request */
pid_t pid;

void tv_sub(struct timeval *out, struct timeval *in){
	if ( (out->tv_usec -= in->tv_usec) < 0) {	/* out -= in */
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

void err_exit(char* str){
	perror(str);
	exit(EXIT_FAILURE);
}
struct addrinfo* host_serv(const char *host, const char *serv, int family, int socktype){
	int	n;
	struct addrinfo	hints, *res;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_CANONNAME;	/* always return canonical name */
	hints.ai_family = family;		/* 0, AF_INET, AF_INET6, etc. */
	hints.ai_socktype = socktype;	/* 0, SOCK_STREAM, SOCK_DGRAM, etc. */

	if((n = getaddrinfo(host, serv, &hints, &res)) != 0){
		perror("host_serv error");
		exit(EXIT_FAILURE);
	}

	return(res);	/* return pointer to first on linked list */
}

char* sock_ntop_host(const struct sockaddr *sa, socklen_t salen){
    static char str[128];

	switch (sa->sa_family) {
		case AF_INET: {
			struct sockaddr_in	*sin = (struct sockaddr_in *) sa;
			if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
				return(NULL);
			return(str);
		}

		case AF_INET6: {
			struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

			if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
				return(NULL);
			return(str);
		}
	}
	return NULL;
}

uint16_t in_cksum(uint16_t *addr, int len){
	int	nleft = len;
	uint32_t sum = 0;
	uint16_t *w = addr;
	uint16_t answer = 0;
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *)(&answer) = *(unsigned char *)w ;
		sum += answer;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);			
	answer = ~sum;
	return (answer);
}

void send_v4(int fd, struct proto *pr){
	int	len;
	struct icmp	*icmp;
	char sendbuf[BUFSIZE] = {0};

	icmp = (struct icmp *)sendbuf;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = pid;

	for(int i = 0; i < 3; i++){
		icmp->icmp_seq = i;
		memset(icmp->icmp_data, 0xa5, datalen);	/* fill with pattern */
		gettimeofday((struct timeval *) icmp->icmp_data, NULL);

		len = 8 + datalen;		/* checksum ICMP header and data */
		icmp->icmp_cksum = 0;
		icmp->icmp_cksum = in_cksum((u_short *)icmp, len);

		sendto(fd, sendbuf, len, 0, pr->sasend, pr->salen);

	}
}

void send_v6(int fd, struct proto *pr){
	int	len;
	struct icmp6_hdr *icmp6;
	char sendbuf[BUFSIZE] = {0};

	icmp6 = (struct icmp6_hdr *) sendbuf;
	icmp6->icmp6_type = ICMP6_ECHO_REQUEST;
	icmp6->icmp6_code = 0;
	icmp6->icmp6_id = pid;
	for(int i = 0; i < 3; i++){
		icmp6->icmp6_seq = i;
		memset((icmp6 + 1), 0xa5, datalen);	/* fill with pattern */
		gettimeofday((struct timeval*)(icmp6 + 1), NULL);

		len = 8 + datalen;		/* 8-byte ICMPv6 header */
		sendto(fd, sendbuf, len, 0, pr->sasend, pr->salen);
	}
	/* kernel calculates and stores checksum for us */
}

void init_v6(int sockfd){
	int on = 1;
	
	/* install a filter that only passes ICMP6_ECHO_REPLY unless verbose */
	struct icmp6_filter myfilt;
	ICMP6_FILTER_SETBLOCKALL(&myfilt);
	ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY, &myfilt);
	setsockopt(sockfd, IPPROTO_IPV6, ICMP6_FILTER, &myfilt, sizeof(myfilt));
	/* ignore error return; the filter is an optimization */

	/* ignore error returned below; we just won't receive the hop limit */
	#ifdef IPV6_RECVHOPLIMIT
		/* RFC 3542 */
		setsockopt(sockfd, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on, sizeof(on));
	#else
		/* RFC 2292 */
		setsockopt(sockfd, IPPROTO_IPV6, IPV6_HOPLIMIT, &on, sizeof(on));
	#endif
}

void proc_v4(char *ptr, ssize_t len, struct msghdr *msg, struct timeval *tvrecv, host_det *hd){
	int				hlen1, icmplen;
	double			rtt;
	struct ip		*ip;
	struct icmp		*icmp;
	struct timeval	*tvsend;

	ip = (struct ip *) ptr;		/* start of IP header */
	hlen1 = ip->ip_hl << 2;		/* length of IP header */
	if(ip->ip_p != IPPROTO_ICMP)
		return;				/* not ICMP */

	icmp = (struct icmp *) (ptr + hlen1);	/* start of ICMP header */
	if((icmplen = len - hlen1) < 8)
		return;				/* malformed packet */

	if (icmp->icmp_type == ICMP_ECHOREPLY) {
		if (icmp->icmp_id != pid)
			return;			/* not a response to our ECHO_REQUEST */
		if (icmplen < 16)
			return;			/* not enough data to use */

		tvsend = (struct timeval *) icmp->icmp_data;
		tv_sub(tvrecv, tvsend);
		rtt = tvrecv->tv_sec * 1000.0 + tvrecv->tv_usec / 1000.0;
		hd->RTT[hd->count] = rtt;
		// printf("%d bytes from %s: seq=%u, ttl=%d, rtt=%.3f ms\n",
		// 		icmplen, hd->ip,
		// 		icmp->icmp_seq, ip->ip_ttl, rtt);

	}
}

void proc_v6(char *ptr, ssize_t len, struct msghdr *msg, struct timeval* tvrecv, host_det* hd){
	double				rtt;
	struct icmp6_hdr	*icmp6;
	struct timeval		*tvsend;

	icmp6 = (struct icmp6_hdr *) ptr;
	if (len < 8)
		return;				/* malformed packet */

	if (icmp6->icmp6_type == ICMP6_ECHO_REPLY) {
		if (icmp6->icmp6_id != pid)
			return;			/* not a response to our ECHO_REQUEST */
		if (len < 16)
			return;			/* not enough data to use */

		tvsend = (struct timeval *) (icmp6 + 1);
		tv_sub(tvrecv, tvsend);
		rtt = tvrecv->tv_sec * 1000.0 + tvrecv->tv_usec / 1000.0;

		
		hd->RTT[hd->count] = rtt;
		// printf(", rtt=%.3f ms\n", rtt);
	}
}

void* readloop(void *arg){
	pthread_args *args = (pthread_args*)arg;
	hashmap *hm = args->hm;
    int epoll_fd = args->epoll_fd;
	struct epoll_event ev_list[MAX_EVENTS];
	struct msghdr msg;
	struct iovec iov;
	ssize_t	n;
	struct timeval tval;
	
	while (1){
        int ready = epoll_wait(epoll_fd, ev_list, MAX_EVENTS, EPOLL_TIMEOUT);
		if (ready == -1){ 
			if (errno == EINTR) 
				continue; /* Restart if interrupted by signal */ 
			else 
				err_exit(RED"EPOLL WAIT ERROR"RESET); 
		}        
        if(ready == 0) {
            printf(GREEN"TIMED OUT!\n"RESET);
            break;
        }
        else if(ready == -1)
            err_exit(RED"EPOLL WAIT ERROR"RESET);
		for (int i = 0; i < ready; ++i){
			if(ev_list[i].events & EPOLLIN){
				char recvbuf[BUFSIZE] = {0};
				char controlbuf[BUFSIZE] = {0};
				int sock = ev_list[i].data.fd;

				iov.iov_base = recvbuf;
				iov.iov_len = sizeof(recvbuf);
				msg.msg_name = (struct sockaddr*)calloc(1, sizeof(struct sockaddr));
				msg.msg_namelen = sizeof(struct sockaddr);
				msg.msg_iov = &iov;
				msg.msg_iovlen = 1;
				msg.msg_control = controlbuf;
				msg.msg_controllen = sizeof(controlbuf);
				
				host_det *hd = get(hm, sock);
				
				n = recvmsg(sock, &msg, 0);
				if (n < 0) {
					if (errno == EINTR)
						continue;
					else
						perror(RED"RECVMSG ERROR"RESET);
				}
				gettimeofday(&tval, NULL);
				if(hd->type == 0)
					proc_v4(recvbuf, n, &msg, &tval, hd);
				else
					proc_v6(recvbuf, n, &msg, &tval, hd);
				hd->count++;
				if(hd->count == 3){
					printf(YELLOW"%s : %lf, %lf, %lf\n"RESET, hd->ip, hd->RTT[0], hd->RTT[1], hd->RTT[2]);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock, &ev_list[i]);
					close(sock);
				}
			}
		}
	}
	return NULL;
}

int main(int argc, char **argv) {
	FILE *fp;
	struct addrinfo	*ai;
	
    fp = fopen(FILENAME, "r");
	if(fp == NULL){
        printf(RED"FILE OPEN ERROR : %s\n"RESET, FILENAME);
        return EXIT_FAILURE;
    }
	hashmap* hm = (hashmap*)malloc(sizeof(hashmap));
	create_hm(hm, HASH_DIM);
	pid = getpid() & 0xffff;	/* ICMP ID field is 16 bits */
	int epoll_fd = epoll_create1(0);
    if(epoll_fd == -1){
        err_exit(RED"EPOLL CREATE ERROR"RESET);
	}
	printf(GREEN"PID: %d, PID_TRUNC: %d, EPOLL: %d\n"RESET, getpid(), pid, epoll_fd);
	
	pthread_t thread_id;
    pthread_args args = {hm, epoll_fd};
    pthread_create(&thread_id, NULL, readloop, (void*)&args);

    char *ip = (char*)malloc(sizeof(char)*41);
    size_t len = 41;
    int read;
	int sock_fd;
    while((read = getline(&ip, &len, fp)) != -1){
		ip[read-1] = 0;
        host_det h;
		bzero(&h, sizeof(h));
		strcpy(h.ip, ip);

		struct proto proto_v4 = { proc_v4, send_v4, NULL, NULL, NULL, 0, IPPROTO_ICMP };
		struct proto proto_v6 = { proc_v6, send_v6, init_v6, NULL, NULL, 0, IPPROTO_ICMPV6 };
		struct proto *pr;
		ai = host_serv(ip, NULL, 0, 0);

		char* host = sock_ntop_host(ai->ai_addr, ai->ai_addrlen);
		// printf("PING %s (%s): %d data bytes\n",
		// 		ai->ai_canonname ? ai->ai_canonname : host,
		// 		host, datalen);

		if (ai->ai_family == AF_INET) {
			pr = &proto_v4;
			h.type = 0;
		} else if (ai->ai_family == AF_INET6) {
			pr = &proto_v6;
			h.type = 1;
			if (IN6_IS_ADDR_V4MAPPED(&(((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr))){
				perror(RED"CANNOT PING IPv4-MAPPED IPv6 ADDRESS"RESET);
				continue;
			}
		}else{
			perror(RED"UNKNOWN ADDRESS FAMILY"RESET);
			continue;
		}
		

		pr->sasend = ai->ai_addr;
		pr->sarecv = calloc(1, ai->ai_addrlen);
		pr->salen = ai->ai_addrlen;
		int	size;
		

		sock_fd = socket(pr->sasend->sa_family, SOCK_RAW, pr->icmpproto);
		if (pr->finit)
			(*pr->finit)(sock_fd);

		size = 60 * 1024;		/* OK if setsockopt fails */
		setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = sock_fd;

		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev) == -1) {
			close(epoll_fd);
			err_exit(RED"EPOLL CTL ADD ERROR\n"RESET);
		}
		h.count = 0;
		h.valid = true;
		insert(hm, sock_fd, h);

		(*pr->fsend)(sock_fd, pr);

		freeaddrinfo(ai);
    }

    pthread_join(thread_id, NULL);
    close(epoll_fd);
	exit(0);
}
