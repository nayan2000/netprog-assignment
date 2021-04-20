#include "ping.h"

proto *pr;
proto proto_v4 = { proc_v4, send_v4, NULL, NULL, NULL, 0, IPPROTO_ICMP };
proto proto_v6 = { proc_v6, send_v6, init_v6, NULL, NULL, 0, IPPROTO_ICMPV6 };

int	datalen = 56;		/* Data that goes with ICMP echo request */

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
	int				nleft = len;
	uint32_t		sum = 0;
	uint16_t		*w = addr;
	uint16_t		answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

		/* 4mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(unsigned char *)(&answer) = *(unsigned char *)w ;
		sum += answer;
	}

		/* 4add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}

void send_v4(void){
	int			len;
	struct icmp	*icmp;

	icmp = (struct icmp *) sendbuf;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = pid;
	icmp->icmp_seq = nsent++;
	memset(icmp->icmp_data, 0xa5, datalen);	/* fill with pattern */
	gettimeofday((struct timeval *) icmp->icmp_data, NULL);

	len = 8 + datalen;		/* checksum ICMP header and data */
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = in_cksum((u_short *) icmp, len);

	sendto(sockfd, sendbuf, len, 0, pr->sasend, pr->salen);
}

void send_v6(){
	int					len;
	struct icmp6_hdr	*icmp6;

	icmp6 = (struct icmp6_hdr *) sendbuf;
	icmp6->icmp6_type = ICMP6_ECHO_REQUEST;
	icmp6->icmp6_code = 0;
	icmp6->icmp6_id = pid;
	icmp6->icmp6_seq = nsent++;
	memset((icmp6 + 1), 0xa5, datalen);	/* fill with pattern */
	gettimeofday((struct timeval *) (icmp6 + 1), NULL);

	len = 8 + datalen;		/* 8-byte ICMPv6 header */

	sendto(sockfd, sendbuf, len, 0, pr->sasend, pr->salen);
	/* kernel calculates and stores checksum for us */
}

void init_v6(){
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

void sig_alrm(int signo){
	(*pr->fsend)();

	alarm(1);
	return;
}

void tv_sub(struct timeval *out, struct timeval *in){
	if ( (out->tv_usec -= in->tv_usec) < 0) {	/* out -= in */
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

void proc_v4(char *ptr, ssize_t len, struct msghdr *msg, struct timeval *tvrecv){
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

		printf("%d bytes from %s: seq=%u, ttl=%d, rtt=%.3f ms\n",
				icmplen, sock_ntop_host(pr->sarecv, pr->salen),
				icmp->icmp_seq, ip->ip_ttl, rtt);

	}
}

void proc_v6(char *ptr, ssize_t len, struct msghdr *msg, struct timeval* tvrecv){
	double				rtt;
	struct icmp6_hdr	*icmp6;
	struct timeval		*tvsend;
	struct cmsghdr		*cmsg;
	int					hlim;

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

		hlim = -1;
		for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL; cmsg = CMSG_NXTHDR(msg, cmsg)) {
			if (cmsg->cmsg_level == IPPROTO_IPV6 &&
				cmsg->cmsg_type == IPV6_HOPLIMIT) {
				hlim = *(u_int32_t *)CMSG_DATA(cmsg);
				break;
			}
		}
		printf("%d bytes from %s: seq=%u, hlim=",
				len, sock_ntop_host(pr->sarecv, pr->salen),
				icmp6->icmp6_seq);
		if (hlim == -1)
			printf("???");	/* ancillary data missing */
		else
			printf("%d", hlim);
		printf(", rtt=%.3f ms\n", rtt);
	}
}

void readloop(void){
	int				size;
	char			recvbuf[BUFSIZE];
	char			controlbuf[BUFSIZE];
	struct msghdr	msg;
	struct iovec	iov;
	ssize_t			n;
	struct timeval	tval;

	sockfd = socket(pr->sasend->sa_family, SOCK_RAW, pr->icmpproto);
	setuid(getuid());		/* don't need special permissions any more */
	if (pr->finit)
		(*pr->finit)();

	size = 60 * 1024;		/* OK if setsockopt fails */
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

	sig_alrm(SIGALRM);		/* send first packet */

	iov.iov_base = recvbuf;
	iov.iov_len = sizeof(recvbuf);
	msg.msg_name = pr->sarecv;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = controlbuf;
	for ( ; ; ) {
		msg.msg_namelen = pr->salen;
		msg.msg_controllen = sizeof(controlbuf);
		n = recvmsg(sockfd, &msg, 0);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			else
				err_sys("recvmsg error");
		}

		gettimeofday(&tval, NULL);
		(*pr->fproc)(recvbuf, n, &msg, &tval);
	}
}

int main(int argc, char **argv)
{
	struct addrinfo	*ai;
	char *h;

	pid = getpid() & 0xffff;	/* ICMP ID field is 16 bits */
	struct sigaction act;
	act.sa_handler = sig_alrm;
	sigemptyset(&act.sa_mask);
	if(sigaction(SIGALRM, &act, NULL) < 0){
		perror(RED"SIG HANDLER"RESET);
		return SIG_ERR;
	}
	ai = host_serv(host, NULL, 0, 0);
	// if(ai == NULL) continue;
	h = sock_ntop_host(ai->ai_addr, ai->ai_addrlen);

	printf("PING %s (%s): %d data bytes\n",
			ai->ai_canonname ? ai->ai_canonname : h,
			h, datalen);

	/* initialize according to protocol */
	if (ai->ai_family == AF_INET) {
		pr = &proto_v4;
	}
	else if (ai->ai_family == AF_INET6) {
		pr = &proto_v6;
		if (IN6_IS_ADDR_V4MAPPED(&(((struct sockaddr_in6 *)
								 ai->ai_addr)->sin6_addr)))
			perror("cannot ping IPv4-mapped IPv6 address");
	}else
		perror("unknown address family");

	pr->sasend = ai->ai_addr;
	pr->sarecv = (struct sockaddr*)calloc(1, ai->ai_addrlen);
	pr->salen = ai->ai_addrlen;

	readloop();

	exit(0);
}
