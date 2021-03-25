#include "inet_sockets.h"       /* Declares functions defined here */

/* The following arguments are common to several of the routines
   below:

        'host':         NULL for loopback IP address, or
                        a host name or numeric IP address
        'service':      either a name or a port number
        'type':         either SOCK_STREAM or SOCK_DGRAM
*/

/* Create socket and connect it to the address specified by
  'host' + 'service'/'type'. Return socket descriptor on success,
  or -1 on error */

int inet_connect(const char *host, const char *service, int type){
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_INET;        /* Allows IPv4 or IPv6 */
    hints.ai_socktype = type;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
    s = getaddrinfo(host, service, &hints, &result);
    if (s != 0) {
        errno = ENOSYS;
        return -1;
    }

    /* Walk through returned list until we find an address structure
       that can be used to successfully connect a socket */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;                   /* On error, try next address */

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                      /* Success */

        /* Connect failed: close this socket and try next address */

        close(sfd);
    }

    freeaddrinfo(result);

    return (rp == NULL) ? -1 : sfd;
}

/* Create an Internet domain socket and bind it to the address
   { wildcard-IP-address + 'service'/'type' }.
   If 'doListen' is TRUE, then make this a listening socket (by
   calling listen() with 'backlog'), with the SO_REUSEADDR option set.
   If 'addrLen' is not NULL, then use it to return the size of the
   address structure for the address family for this socket.
   Return the socket descriptor on success, or -1 on error. */

static int inet_passive_socket(const char *service, int type, socklen_t *addrlen,
                  bool doListen, int backlog){
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, optval, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = type;
    hints.ai_family = AF_INET;        /* Allows IPv4 or IPv6 */
    hints.ai_flags = AI_PASSIVE;        /* Use wildcard IP address */
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    s = getaddrinfo(NULL, service, &hints, &result);
    if (s != 0)
        return -1;

    /* Walk through returned list until we find an address structure
       that can be used to successfully create and bind a socket */

    optval = 1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;                   /* On error, try next address */

        if (doListen) {
            if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                    sizeof(optval)) == -1) {
                close(sfd);
                freeaddrinfo(result);
                return -1;
            }
        }

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                      /* Success */

        /* bind() failed: close this socket and try next address */

        close(sfd);
    }

    if (rp != NULL && doListen) {
        if (listen(sfd, backlog) == -1) {
            freeaddrinfo(result);
            return -1;
        }
    }

    if (rp != NULL && addrlen != NULL)
        *addrlen = rp->ai_addrlen;      /* Return address structure size */

    freeaddrinfo(result);

    return (rp == NULL) ? -1 : sfd;
}

/* Create stream socket, bound to wildcard IP address + port given in
  'service'. Make the socket a listening socket, with the specified
  'backlog'. Return socket descriptor on success, or -1 on error. */

int inet_listen(const char *service, int backlog, socklen_t *addrlen){
    return inet_passive_socket(service, SOCK_STREAM, addrlen, true, backlog);
}


/* Given a socket address in 'addr', whose length is specified in
   'addrlen', return a null-terminated string containing the host and
   service names in the form "(hostname, port#)". The string is
   returned in the buffer pointed to by 'addrStr', and this value is
   also returned as the function result. The caller must specify the
   size of the 'addrStr' buffer in 'addrStrLen'. */

char* inet_address_str(const struct sockaddr *addr, socklen_t addrlen,
               char *addrStr, int addrStrLen){
    char host[NI_MAXHOST], service[NI_MAXSERV];

    if (getnameinfo(addr, addrlen, host, NI_MAXHOST,
                    service, NI_MAXSERV, NI_NUMERICSERV|NI_NUMERICHOST) == 0)
        snprintf(addrStr, addrStrLen, "(%s, %s)", host, service);
    else
        snprintf(addrStr, addrStrLen, "(?UNKNOWN?)");

    return addrStr;
}

void                    /* Initialize a ReadLineBuf structure */
readline_buf_init(int fd, struct read_buf *rlbuf)
{
    rlbuf->fd = fd;
    rlbuf->len = 0;
    rlbuf->next = 0;
}

/* Return a line of input from the buffer 'rlbuf', placing the characters in
   'buffer'. The 'n' argument specifies the size of 'buffer'. If the line of
   input is larger than this, then the excess characters are discarded. */

ssize_t readline_buf(read_buf *rlbuf, char *buffer, size_t n)
{
    size_t cnt;
    char c;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    cnt = 0;

    /* Fetch characters from rlbuf->buf, up to the next new line. */
    for (;;) {

        /* If there are insufficient characters in 'tlbuf', then obtain
           further input from the associated file descriptor. */

        if (rlbuf->next >= rlbuf->len) {
            rlbuf->len = read(rlbuf->fd, rlbuf->buf, RL_MAX_BUF);
            if (rlbuf->len == -1)
                return -1;

            if (rlbuf->len == 0)        /* End of file */
                break;

            rlbuf->next = 0;
        }

        c = rlbuf->buf[rlbuf->next];
        rlbuf->next++;
        
        if (cnt < n){
            buffer[cnt] = c;
            if(c == '\t')
                buffer[cnt] = '\n';
            cnt++;
        }
        if (c == '$')
            break;
    }
    printf("Read Size : %ld\n", cnt);
    printf("Read Value :\n%s\n", buffer);
    return cnt;
}