/* Abdul Kadir Salim Khimani 2017B4A70696P */
/* Date created : 13 Feb, 2021 */

/* Type definitions used by many programs */
#include<sys/wait.h>
#include<signal.h>
#include<stdio.h>       /* Standard I/O functions */
#include<stdlib.h>      /* Prototypes of commonly used library functions */

#include <unistd.h>     /* Prototypes for many system calls */
#include <errno.h>      /* Declares errno and defines error constants */
#include <stdbool.h>    /* 'bool' type plus 'true' and 'false' constants */

/* Color codes for printing to terminal */
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define RESET "\033[0m"

#define PRIME 13
int numSignals = 0;     /* Keeps tracks of signals recieved during 2 second interval */

/* Defining signal handler */
void sigHandler(int sig){
    if(sig == SIGUSR1){
        numSignals += 1;
    }
    else{ /* SIGALRM */

        pid_t myPid = getpid();

        /* Sending SIGUSR1 signals to all neighbours */
        /* For non existent process, it simply returns error which we ignore*/
        for(size_t i = 1; i <= 12; i++){
            kill(myPid + i, SIGUSR1);
        }

        printf(YELLOW"%d process recieved %d signals\n"RESET, myPid, numSignals);

        /* Exit the process if no signals are recieved */
        if(numSignals == 0){
            printf(GREEN"Exiting: 0 signals recieved for %d\n"RESET, getpid());
            _exit(EXIT_SUCCESS);

        /* Not waiting for all child processes to finish before exiting */
        /* Leads to creation of zombie processes */
        /* To avoid zombies, uncomment the for loop, and comment the above lines */
        /*  for(;;){
            pid_t childPid = wait(0);
                if(childPid == -1){
                    if(errno == ECHILD){
                        printf(GREEN"Exiting: 0 signals recieved for %d\n"RESET, getpid());
                        exit(EXIT_SUCCESS);
                    }else{
                        perror("Wait error");
                        printf(RED"Unexpected error while waiting for process %d\n"RESET, getpid());
                        _exit(EXIT_FAILURE);
                    }
                }
            }
        */
        }
        numSignals = 0;
        alarm(2);
    }
}

/* Driver function */
signed main(int argc, char* argv[]){
    int n;
    /* Disable buffereing of library I/O */
    setbuf(stdout, NULL);

    /* Input for number of processes */
    printf(YELLOW"Enter the number of processes : "RESET);
    scanf("%d", &n);
    if(n < 0){
        printf(RED"Number of processes cannot be negative.\n"RESET);
        exit(EXIT_FAILURE);
    }

    pid_t child;
    for(size_t i = 0; i < n; i++){
        child = fork();

        /* Child functions here */
        if(child == 0){
            
            printf("\nCreated "CYAN"[son] pid %d"RESET" from "YELLOW"[parent] pid %d\n"RESET,getpid(),getppid()); 

            /* Setting signal disposition */
            if(signal(SIGUSR1, sigHandler) == SIG_ERR)
                printf(RED"Unable to set SIGUSR1 disposition for %d\n"RESET, getpid());
            if(signal(SIGALRM, sigHandler) == SIG_ERR)
                printf(RED"Unable to set SIGALRM disposition for %d\n"RESET, getpid());

            /* Setting alarm for 2 seconds */
            alarm(2);

            pid_t myPid = getpid();

            printf("Checking existence of PIDs for "CYAN"[son] %d\n"RESET, myPid);
            for(int i = 1; i <= 12; i++){
                if(kill(myPid + i, 0) == 0){
                    printf("Process "PURPLE"%d"RESET" exists for "CYAN"[son] %d\n"RESET""RESET, myPid + i, myPid);
                }
            }

            size_t z = getpid()%PRIME;
            printf("z value : %lu for "CYAN"[son] %d\n"RESET, z, myPid);

            for(size_t i = 0; i < z; i++){
                pid_t gChild = fork();

                /* Grand children functions here */
                if(gChild == 0){
                    printf(GREEN"[gson] pid %d "RESET"from "CYAN"[son] pid %d"RESET"\n",getpid(),getppid());
                    
                    /* Setting signal disposition */
                    if(signal(SIGUSR1, sigHandler) == SIG_ERR)
                        printf(RED"Unable to set SIGUSR1 disposition for %d\n"RESET, getpid());
                    if(signal(SIGALRM, sigHandler) == SIG_ERR)
                        printf(RED"Unable to set SIGALRM disposition for %d\n"RESET, getpid());
                    
                    /* Setting alarm for 2 seconds */
                    alarm(2);

                    pid_t myPid = getpid();
                    printf("Checking existence of PIDs for "GREEN"[gson] %d\n"RESET, myPid);
                    for(int i = 1; i <= 12; i++){
                        if(kill(myPid + i, 0) == 0){
                            printf("Process "PURPLE"%d"RESET" exists for "GREEN"[gson] %d\n"RESET""RESET, myPid + i, myPid);
                        }
                    }
                    /* Suspending to save CPU time */
                    while(1)
                        pause();
                }
            }
            /* Suspending to save CPU time */
            while(1)
                pause();
            
        }
    }

    /* Waiting for N processes to exit */
    int status;
    for(;;){
        pid_t childPid = wait(&status);
        if(childPid == -1){
            if(errno == ECHILD){
                printf(GREEN"Exiting: No active children for main process %d\n"RESET, getpid());
                printf("Exiting.\n");
                _exit(EXIT_SUCCESS);
            }else{
                printf("Unexpected error while waiting for main process %d\n", getpid());
                _exit(EXIT_FAILURE);
            }
        }
    }

    return 0;
}