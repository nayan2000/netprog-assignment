#include "basic.h"
#include "client_utilities.h"
int main(){

    // char cmd_out[] = {'w', 'c', '\0', 'd', '\n', 'c', '\n', 'a','\n', '$'};
    // if(cmd_out[10 - 1] == '$')
	// 		cmd_out[10-1] = 0; 		/*Remove $*/

    char command[] = "sort -k9";
    // char* temp = cmd_out + strlen(command) + 1;
    char* input = "s\na\nc\nb\n";
    puts("Seperated values :");
    puts(command);
    puts(input);
    char** args = tokenise(command);
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
                    fprintf(stderr, "Redirected Input\n");
                }
                redirect_desc_io(rpipes[1], STDOUT_FILENO);
               
                fprintf(stderr, PURPLE"Executing current command : %s\n"RESET, command);
                // if(strlen(input))
                //     command = check_redirection(command, wpipes[0], rpipes[1]);
                // else
                //     command = check_redirection(command, STDIN_FILENO, rpipes[1]);
                
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
                fputs(output, stderr);
                // fprintf(stderr, "%ld\n", strlen(output)); 
        }
        strcat(output, "$");
        puts(output+2);
    }
}