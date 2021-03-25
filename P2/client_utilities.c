#include "client_utilities.h"

void trim(char *s, bool space) {
    char *p = s;
    int l = strlen(p);
    if(space){
        while(isspace(p[l - 1])) p[--l] = 0;
        while(*p && isspace(* p)) ++p, --l;
    }
    else{
        while(p[l - 1] == '"') p[--l] = 0;
        while(*p && *p == '"') ++p, --l;
    }

    memmove(s, p, l + 1);
} 

char* get_path(char* exe){
    char* path = NULL;
    if(exe[0] == '.' || exe[0]== '/') {
		path = strdup(exe);
        return path;
	}
    char* path_env = getenv("PATH");
    char *token = strtok(path_env, ":");
    while(token != NULL) {
		path = strdup(token);
        strcat(path, "/");
        strcat(path, exe);
        if(access(path, X_OK) == 0) {
            return path;
        }
        token = strtok(NULL, ":");
        free(path);
    }
    return NULL;
}

char** tokenise(char* command){
    char *process = strdup(command);
	int num_tokens = 0;
    char **args = NULL;
	if(process != NULL){
		char *token = strtok(process, " ");
		num_tokens = 0;
		while (token != NULL) {
			num_tokens++;
			token = strtok(NULL, " ");
		}

		args = (char**)malloc(sizeof(char*)*(num_tokens + 1));
        process = strdup(command);
        token = strtok(process, " ");
		int i = 0;
		while (token != NULL) {
			args[i++] = strdup(token);
			token = strtok(NULL, " ");
		}
		args[i] = (char*)NULL;
    }
    return args;
}

void redirect_desc_io(int oldfd, int newfd) {
    if (oldfd != newfd) {
        if (dup2(oldfd, newfd) != -1)
            close(oldfd); /* successfully redirected */
        else{
            perror("dup2");
            _exit(EXIT_FAILURE);
        }
    }
}

char* check_redirection(char* command, int in, int out){
	char *process = strdup(command);
	char *token = strtok(process, "><");
    char* actual_command = strdup(token);

    if(strcmp(token, command) == 0){
        free(process);
        trim(command, true);
        return actual_command;
    }
    char *file;
    trim(actual_command, true);
    
    if(*(command + strlen(token) + 1) == '>'){ /* >> */
        token = strtok(NULL, ">");
        file = token;
        trim(file, true);
        trim(file, false);

        int fd = open(file, O_CREAT | O_WRONLY | O_APPEND, 0664);
        
        fprintf(stderr, "File name : %s\n", file);
        fprintf(stderr, "\tOriginal Write fd : %d\n", out);
        if (fd < 0) 
            fprintf(stderr, RED"ERROR: CANNOT OPEN FILE : %s\n"RESET, file);
        else {
            fprintf(stderr, "\tRemapped Write fd : %d\n", fd);
            redirect_desc_io(fd, out);
        }
    }
    else if(*(command + strlen(token)) == '>'){
        token = strtok(NULL, ">");
        file = token;
        trim(file, true);
        trim(file, false);
        int fd = open(file, O_CREAT | O_WRONLY, 0664);

        fprintf(stderr, "File name : %s\n", file);
        fprintf(stderr, "\tOriginal Write fd : %d\n", out);
        if (fd < 0) 
            fprintf(stderr, RED"ERROR: CANNOT OPEN FILE : %s\n"RESET, file);
        else {
            fprintf(stderr, "\tRemapped Write fd : %d\n", fd);
            redirect_desc_io(fd, out);
        }
    }
    else if(*(command + strlen(token)) == '<'){
        token = strtok(NULL, ">");
        file = token;
        trim(file, true);
        trim(file, false);
    
        int fd = open(file, O_CREAT | O_RDONLY, 0664);

        fprintf(stderr, "File name : %s\n", file);
        fprintf(stderr, "\tOriginal fd : %d\n", in);
        if (fd < 0) 
            fprintf(stderr, RED"ERROR: CANNOT OPEN FILE : %s\n"RESET, file);
        else {
            fprintf(stderr, "\tRemapped fd : %d\n", fd);
            redirect_desc_io(fd, in);
        }
    }
    free(process);
	return actual_command;
}

void clear_screen(){
  const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
  write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}

bool process_command(char *command){
    bool ignore;
    size_t max_cmd_sz = CMD_SZ + 1;

    /* Get command from the terminal */
    int cmd_sz = getline(&command, &max_cmd_sz, stdin);
    if(cmd_sz == -1 || cmd_sz == 0 || strcmp(command, "\n") == 0)
        return ignore = true;
        
    command[cmd_sz -1] = 0; /*Remove new line*/
    return ignore = false;
}