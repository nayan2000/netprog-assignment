#include "client_utilities.h"

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

        char** args = tokenise(command);
        char* output = (char*)malloc(sizeof(char)*MAX_BUF_SZ);
        // puts("Seperated values :");
        // puts(command);
        // puts(input);
        if(command[0] == 'c' && command[1] == 'd'){
            if(strcmp("cd", command) == 0){
                strcat(command, " ");
                strcat(command, getenv("HOME"));
            }
            else if(command[3] == '~' && command[2] == ' '){
                strcat(command, " ");
                strcat(command, getenv("HOME"));
            }
            char *path = command + 3;
            if (chdir(path) < 0) {
                perror(RED"CD EXECUTION ERROR"RESET);
                _exit(EXIT_FAILURE);
            }
            
            strcpy(output, "Executed cd\n$");
            write(cfd, output, strlen(output) + 1);
        }
        else {
            int rpipes[2], wpipes[2];
            pipe(rpipes);
            pipe(wpipes);
            if(strlen(input)){
                write(wpipes[1], input, strlen(input) + 1);
            }
            output = (char*)malloc(sizeof(char)*MAX_BUF_SZ);
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
                    // fprintf(stderr, "%ld\n", strlen(output)); 
            }
            strcat(output+2, "$");
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
    // if(nread == 0){
    //     close(cfd);
    // }
}

int main(){
    clear_screen();
    int lfd = inet_listen(CLIENT_PORT, BACKLOG, NULL);
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
        case 0:;
            int connfd = inet_connect(NULL, SERV_PORT, SOCK_STREAM);
            if(connfd == -1){
                perror(RED"CLIENT SIDE: MAIN SERVER DOWN"RESET);
                exit(0);
            }
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
                out[nread-1] = 0; /* Remove $ */
                if(nread < 0){
                    perror("Read");
                    break;
                }
                if(nread == 0){
                    perror(RED"CLIENT SIDE: MAIN SERVER ABNORMAL TERMINATION"RESET);
                    break;
                    
                }
                out[nread] = '\0';
                printf("-------------Output-------------\n\n");
                printf(YELLOW"%s"RESET"\n", out);
                printf("--------------------------------\n");
                fflush(stdin);
            }
            close(connfd);
            kill(getppid(), SIGTERM);
            _exit(EXIT_FAILURE);
            break;
        default:;
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
                
                handle_request(cfd);			
                close(cfd);
            }
        
    }
}