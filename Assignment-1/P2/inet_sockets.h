/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Listing 59-8 */

/* inet_sockets.h

   Header file for inet_sockets.c.
*/
#ifndef INET_SOCKETS_H
#define INET_SOCKETS_H          /* Prevent accidental double inclusion */

#include "basic.h"


#define SERV_PORT "50000"

/* For non-testing purposes */
#define CLIENT_PORT1 "40000"
#define CLIENT_PORT2 "40001"

/* For non-testing purposes */
/* #define CLIENT_PORT "40005" */

#define SERV_IP "127.0.0.1"
#define BACKLOG 5

#define RL_MAX_BUF 100

typedef struct read_buf{
    int     fd;                 /* File descriptor from which to read */
    char    buf[RL_MAX_BUF];    /* Current buffer from file */
    int     next;               /* Index of next unread character in 'buf' */
    ssize_t len;                /* Number of characters in 'buf' */
}read_buf;

void readline_buf_init(int fd, struct read_buf *rlbuf);

ssize_t readline_buf(read_buf *rlbuf, char *buffer, size_t n);

int inet_connect(const char *host, const char *service, int type);

int inet_listen(const char *service, int backlog, socklen_t *addrlen);

char *inet_address_str(const struct sockaddr *addr, socklen_t addrlen,
                char *addrStr, int addrStrLen);

#define IS_ADDR_STR_LEN 100
                        /* Suggested length for string buffer that caller
                           should pass to inetAddressStr(). Must be greater
                           than (NI_MAXHOST + NI_MAXSERV + 4) */
#endif