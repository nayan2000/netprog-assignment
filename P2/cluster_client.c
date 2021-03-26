#include "client_utilities.h"
void handler(int sig){
    // if(sig == SIGCHLD){
    //     int status;
    //     while(waitpid(-1, &status, WNOHANG) > 0);
    //     raise(SIGTERM);
    // }
}

void handle_request(int cfd){
    ssize_t nread = 0;
	while(1){
		char cmd_out[MAX_BUF_SZ] = {0};
		read_buf reader;
		bzero(&reader, sizeof(reader));
		readline_buf_init(cfd, &reader);
		nread = readline_buf(&reader, cmd_out, MAX_BUF_SZ);
		if(nread <= 0) break;

		if(cmd_out[nread - 1] == '$')
			cmd_out[nread-1] = 0; 		/*Remove $*/

        char* command = strdup(cmd_out);
        char* temp = cmd_out + strlen(command) + 1;
        char* input = strdup(temp);
        // puts("Seperated values :");
        // puts(command);
        // puts(input);
        if(command[0] == 'c' && command[1] == 'd' && command[2] == ' '){
            char *path = command + 3;
            if (chdir(path) < 0) {
                perror(RED"CD EXECUTION ERROR"RESET);
                _exit(EXIT_FAILURE);
            }
        }
        else {
            int rpipes[2], wpipes[2];
            pipe(rpipes);
            pipe(wpipes);
            if(strlen(input)){
                write(wpipes[1], input, strlen(input) + 1);
            }
            char output[MAX_BUF_SZ] = {'\0'};
            int ch;
            switch(ch = fork()){
                case 0:;
                    signal(SIGPIPE, SIG_IGN);
                    close(wpipes[1]);
                    close(rpipes[0]);
                    if(strlen(input)){
                        redirect_desc_io(wpipes[0], STDIN_FILENO);
                        // fprintf(stderr, "Redirected Input\n");
                    }
                    redirect_desc_io(rpipes[1], STDOUT_FILENO);
                
                    fprintf(stderr, PURPLE"Executing current command : %s\n"RESET, command);
                    if(strlen(input))
                        command = check_redirection(command, wpipes[0], rpipes[1]);
                    else
                        command = check_redirection(command, STDIN_FILENO, rpipes[1]);
                    char** args = tokenise(command);
                    char *path = get_path(args[0]);
                    if (path == NULL){
                        fprintf(stderr, RED"ERROR : %s: PROGRAM NOT FOUND\n"RESET, args[0]);
                        _exit(EXIT_FAILURE);
                    }
                    if (execv(path, args) == -1) {
                        fprintf(stderr, RED"FATAL ERROR : %s: PROGRAM CANNOT BE EXECUTED\n"RESET, args[0]);
                    }
                    _exit(0);
                    break;
                default:;
                    close(wpipes[1]);
                    close(wpipes[0]);
                    close(rpipes[1]);
                    waitpid(ch, NULL, WUNTRACED);
                    read(rpipes[0], output, MAX_BUF_SZ);
                    // fputs(output, stderr);
                    // fprintf(stderr, "%ld\n", strlen(output)); 
            }
            strcat(output, "$");
            write(cfd, output, strlen(output) + 1);
        }
    }
    if(nread == 0){
        fprintf(stderr, "Here\n");
        close(cfd);
    }
}

int main(){
    signal(SIGCHLD, handler);
    int p[2];
    pipe(p);
    pid_t ch = fork();
    switch(ch){
        case -1:;
            perror(RED"CLIENT SIDE : FORK ERROR"RESET);
            exit(EXIT_FAILURE);
            break;
        case 0:;
            int connfd = inet_connect(NULL, SERV_PORT, SOCK_STREAM);
            if(connfd == -1){
                perror(RED"CLIENT SIDE: MAIN SERVER DOWN"RESET);
                exit(0);

            }
            close(p[1]);
            int dummy;
            read(p[0], &dummy, 1);
            close(p[0]);
            clear_screen();
            printf("**************** INPUT WINDOW *******************\n");

            for(;;){
                printf(GREEN"Enter command : \n"RESET);
                printf(BLUE">> "RESET);
                fflush(stdout);

                char command[CMD_SZ + 1] = {0};
                bool ignore = process_command(command);
                if(strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0)
                    break;
                if(ignore) continue;    
                printf("Command : %s - Length %ld\n", command, strlen(command));
                fflush(stdin);

                command[strlen(command)] = '$';       /*Delimit end marker*/ 
                
                write(connfd, command, strlen(command) + 1);
                char out[MAX_OUTPUT] = {0};
                read_buf reader;
                bzero(&reader, sizeof(reader));
                readline_buf_init(connfd, &reader);
                size_t nread = readline_buf(&reader, out, MAX_OUTPUT);
                out[nread-1] = 0;
                if(nread < 0){
                    perror("Read");
                    break;
                }
                if(nread == 0){
                    perror(RED"CLIENT SIDE: MAIN SERVER ABNORMAL TERMINATION"RESET);
                    exit(EXIT_FAILURE);
                }
                out[nread] = '\0';
                printf("-------------Output-------------\n\n");
                printf("%s\n", out);
                printf("--------------------------------\n");
                fflush(stdin);
            }
            close(connfd);
            kill(getppid(), SIGTERM);
            _exit(0);
            break;
        default:;
            close(p[0]);
            int lfd = inet_listen(CLIENT_PORT, BACKLOG, NULL);
            if(lfd == -1){
                perror(RED"CLIENT SIDE: LISTEN ERROR"RESET);
                exit(EXIT_FAILURE);
            }
            printf(YELLOW"CLIENT SIDE: SERVER STARTED\n"RESET);
            
            close(p[1]);
            int cfd;
            for(;;) {
                if((cfd = accept(lfd, NULL, NULL)) < 0) {
                    if (errno == EINTR) continue; /* back to for() */ 
                    else{
                        perror(RED"CLIENT SIDE: ACCEPT ERROR"RESET);
                        exit(0);
                    }
                }
               
                printf(GREEN"Handling server request\n"RESET);
                fflush(stdout);
                fflush(stdin);
                pid_t child = fork();
                switch(child){
                    case -1:;
                        printf(RED"CLIENT SIDE: CHILDREN CAN'T BE CREATED\n"RESET);	
                        exit(EXIT_FAILURE);

                    case 0:;
                        int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
                        if (r == -1) { perror(0); exit(1); }
                        if (getppid() == 1)
                            exit(1);
                        close(lfd);	
                        handle_request(cfd);			
                        _exit(EXIT_SUCCESS);
                    default:
                        close(cfd);
                        break;
                }
            }
            
            break;
    }
}