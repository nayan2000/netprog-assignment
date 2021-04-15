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

/* Type definitions used by many programs */
#include<sys/wait.h>
#include<signal.h>
#include<stdio.h>       /* Standard I/O functions */
#include<stdlib.h>      /* Prototypes of commonly used library functions */

#include <unistd.h>     /* Prototypes for many system calls */
#include <errno.h>      /* Declares errno and defines error constants */
#include <stdbool.h>    /* 'bool' type plus 'true' and 'false' constants */

#endif

