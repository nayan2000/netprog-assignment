/* Prevent accidental double inclusion */
#ifndef COMMON_H
#define COMMON_H

/* Color codes for printing to terminal */
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define RESET "\033[0m"
#define _DEFAULT_SOURCE        

/* Type definitions used by many programs */
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>       /* Standard I/O functions */
#include <stdlib.h>      /* Prototypes of commonly used library functions */
#include <math.h>		/* Math manipulations */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

#include <unistd.h>     /* Prototypes for many system calls */
#include <errno.h>      /* Declares errno and defines error constants */
#include <string.h>     /* Commonly used string-handling functions */
#include <stdbool.h>    /* 'bool' type plus 'true' and 'false' constants */

#include <netinet/in.h>
#include <netdb.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#define min(m,n) ((m) < (n) ? (m) : (n))
#define max(m,n) ((m) > (n) ? (m) : (n))


#endif

