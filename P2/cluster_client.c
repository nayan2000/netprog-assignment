#include "client_utilities.h"

void handle_request(int cfd){
    ssize_t nread = 0;
	while(1){
		char cmd_out[MAX_BUF_SZ] = {0};
		read_buf reader;
		bzero(&reader, sizeof(reader));
		readline_buf_init(cfd, &reader);

        /* Read <command>\0<input>$ from the server */
		nread = readline_buf(&reader, cmd_out, MAX_BUF_SZ);
		if(nread <= 0) break; /* Break if we see EOF or error */

		if(cmd_out[nread - 1] == '$')
			cmd_out[nread-1] = 0; 		/*Remove $*/

        char* command = strdup(cmd_out); /* Get command */
        char* temp = cmd_out + strlen(command) + 1; /* Skip <command>\0 */
        char* input = strdup(temp); /* Get input */

        char** args = tokenise(command);
        char* output = (char*)malloc(sizeof(char)*MAX_BUF_SZ);

        /* Handle cd command */
        if(command[0] == 'c' && command[1] == 'd'){
            if(strcmp("cd", command) == 0){ /* Home directory */
                strcat(command, " ");
                strcat(command, getenv("HOME"));
            }
            else if(command[3] == '~' && command[2] == ' '){ /* Home directory */
                strcat(command, " ");
                strcat(command, getenv("HOME"));
            }
            char *path = command + 3;
            if (chdir(path) < 0) { /* Execute cd */
                perror(RED"CD EXECUTION ERROR"RESET);
                _exit(EXIT_FAILURE);
            }
            
            strcpy(output, "Executed cd\n$");
            write(cfd, output, strlen(output) + 1);
        }
        else { /* Handle everything else */

            /* Use pipes for giving input from server and 
            recieving output to be forwarded to the server */
            int rpipes[2], wpipes[2];
            pipe(rpipes);
            pipe(wpipes);
            if(strlen(input)){
                write(wpipes[1], input, strlen(input) + 1);
            }
            output = (char*)malloc(sizeof(char)*MAX_BUF_SZ);
            int ch;
            switch(ch = fork()){
                case -1:;
                    perror(RED"CLIENT SIDE : FORK ERROR"RESET);
                    exit(EXIT_FAILURE);
                    break;
                case 0:;
                    /* Redirect Input Output */
                    signal(SIGPIPE, SIG_IGN);
                    close(wpipes[1]);
                    close(rpipes[0]);
                    if(strlen(input)){
                        redirect_desc_io(wpipes[0], STDIN_FILENO);
                    }
                    redirect_desc_io(rpipes[1], STDOUT_FILENO);
                                    
                    /* Get path of executable */
                    char *path = get_path(args[0]);
                    if (path == NULL){
                        fprintf(stderr, RED"ERROR : %s: PROGRAM NOT FOUND\n"RESET, args[0]);
                        _exit(EXIT_FAILURE);
                    }
                    if (execv(path, args) == -1) {
                        fprintf(stderr, RED"FATAL ERROR : %s: PROGRAM CANNOT BE EXECUTED\n"RESET, args[0]);
                    }
                    _exit(EXIT_FAILURE);
                    break;
                default:;
                    /* Close unused ends */
                    close(wpipes[1]);
                    close(wpipes[0]);
                    close(rpipes[1]);
                    waitpid(ch, NULL, WUNTRACED);
                    read(rpipes[0], output, MAX_BUF_SZ);
            }
            /* We seperately handle sort command output because 
            sort output has 2 '\0's at the start. */
            if(args && strcmp(args[0], "sort") == 0){
                strcat(output+2, "$");
                write(cfd, output+2, strlen(output+2) + 1);
            }
            else{
                strcat(output, "$");
                write(cfd, output, strlen(output) + 1);
            }
           
        }
        free(output);
    }
}

int main(){   
    /* Listen for execution requests from server in parent */
    /* For testing purposes */
    int lfd = inet_listen(CLIENT_PORT1, BACKLOG, NULL);

    /* For non-testing purposes */
    /* int lfd = inet_listen(CLIENT_PORT, BACKLOG, NULL); */
    if(lfd == -1){
        perror(RED"CLIENT SIDE: LISTEN ERROR"RESET);
        exit(EXIT_FAILURE);
    }
    printf(YELLOW"CLIENT SIDE: SERVER STARTED\n"RESET);

    
    pid_t ch = fork();
    switch(ch){
        case -1:;
            perror(RED"CLIENT SIDE : FORK ERROR"RESET);
            exit(EXIT_FAILURE);
            break;
        case 0:; /* Child runs shell for input */
             /* Connect to server for sending commands */
            printf("Connecting to main server:\n");
            int connfd = inet_connect(SERV_IP, SERV_PORT, SOCK_STREAM);
            if(connfd == -1){
                perror(RED"MAIN SERVER DOWN"RESET);
                exit(0);
            }
            clear_screen();
            printf("**************** INPUT WINDOW *******************\n");
            /* Loop forever */
            for(;;){
                /* Display prompt */
                printf(GREEN"Enter command : \n"RESET);
                printf(BLUE">> "RESET);
                fflush(stdout);

                /* Preprocess command to */
                char command[CMD_SZ + 1] = {0};
                bool ignore = process_command(command);
                if(ignore) continue;    

                if(strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0)
                    break;
                    
                printf("%s\n", command);

                command[strlen(command)] = '$';       /*Delimit by end $ marker*/ 


                /* Send command execution request to server */
                /* Command of the form n[<id>,*].<command> | n[<id>,*].<command> | ...*/
                /* [id,*] indicates one of the two options */
                /* <command> should not have redirection */
                write(connfd, command, strlen(command) + 1);

                char out[MAX_OUTPUT] = {0};
                read_buf reader;
                bzero(&reader, sizeof(reader));

                readline_buf_init(connfd, &reader);

                /* Read command output from server */
                size_t nread = readline_buf(&reader, out, MAX_OUTPUT);
                out[nread-1] = 0; /* Remove $ */

                if(nread < 0){
                    perror("Read");
                    break;
                }
                if(nread == 0){
                    perror(RED"MAIN SERVER ABNORMAL TERMINATION"RESET);
                    break;
                    
                }
                /* Print command output */
                out[nread] = '\0';
                printf("-------------Output-------------\n\n");
                printf(YELLOW"%s"RESET"\n", out);
                printf("--------------------------------\n");
                fflush(stdin);
            }
           /* If error or forcefully closed */
            close(connfd);
            kill(getppid(), SIGINT);
            _exit(EXIT_FAILURE);
            break;
        default:; /* Parent accepts server requests iteratively */
            /* Use iterative handling because of cd command */
            /* Concurrent handling can be used as well if there were no cd 
            command. Using cd changes the current directory for the program in which
            it is run. Thus, it will change the directory of child and not 
            the parent. We can seperately handle cd in the parent but we don't
            for a small scale project. */
            int cfd;
            for(;;){  /* Handle each request one after another */
                if((cfd = accept(lfd, NULL, NULL)) < 0) {
                    if (errno == EINTR) break; /* back to for() */ 
                    else{
                        perror(RED"CLIENT SIDE: ACCEPT ERROR"RESET);
                        exit(0);
                    }
                }
                handle_request(cfd);			
                close(cfd);
            }
            break;
    }
}