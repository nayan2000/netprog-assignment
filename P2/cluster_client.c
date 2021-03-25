#include "basic.h"
#include "setup_utilities.h"
#include "get_config.h"
#include "parse_input.h"
#include "inet_sockets.h"

#define MAX_BUF_SZ 2048
#define MAX_OUTPUT 2048
#define MAX_INPUT 2048
#define CMD_SZ 512


size_t max_cmd_sz = CMD_SZ + 1;
void clear_screen(){
  const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
  write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}
bool process_command(char *command){
    bool ignore;

    /* Get command from the terminal */
    memset(command, '\0',sizeof(command));
    int cmd_sz = getline(&command, &max_cmd_sz, stdin);
    if(cmd_sz == -1 || cmd_sz == 0 || strcmp(command, "\n") == 0)
        return ignore = true;
        
    /* Remove newline character */
    command[cmd_sz - 1] = '\0';
    
    if(strcmp("quit", command) && strcmp("exit", command))
        printf("Command : %s\n", command);
    return ignore = false;
}

int main(){
    pid_t ch = fork();
    switch(ch){
        case -1:
            perror("FORK ERROR");
            exit(EXIT_FAILURE);
            break;
        case 0:
            break;

        default:;
        
            int connfd = inet_connect(NULL, SERV_PORT, SOCK_STREAM);
            if(connfd == -1){
                perror("Server Down");
                exit(0);

            }
            clear_screen();
            printf("**************** INPUT WINDOW *******************\n");

            for(;;){
                printf(GREEN"Enter command : \n"RESET);
                printf(BLUE">> "RESET);
                fflush(stdout);

                char command[CMD_SZ + 1] = {0};
                bool ignore = process_command(command);
                if(ignore) continue;

                write(connfd, command, 6);
                char out[MAX_OUTPUT + 1];
                size_t nread = read(connfd, out, MAX_OUTPUT);

                if(nread < 0){
                    perror("Read");
                    break;
                }
                if(nread == 0){
                    perror("Server abnormal termination");
                    exit(EXIT_FAILURE);
                }
                out[nread] = '\0';

                printf("%s\n", out);
                printf("--------------------------------\n");
            }
            close(connfd);
    }
}