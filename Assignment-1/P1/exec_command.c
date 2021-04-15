#include "exec_command.h"
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

bool exec_curr_cmd(char* command, int t, int in, int out){
    
    if(t == (int)SPIPE || t == -1){
        printf(PURPLE"Executing current command : %s\n"RESET, command);
        preprocess_pipe_io(in, out);
        command = check_redirection(command, in, out);
        char** args = tokenise(command);
        char *path = get_path(args[0]);
        if (path == NULL){
            fprintf(stderr, RED"ERROR : %s: PROGRAM NOT FOUND\n"RESET, args[0]);
            return false;
        }
        if (execv(path, args) == -1) {
            fprintf(stderr, RED"FATAL ERROR : %s: PROGRAM CANNOT BE EXECUTED\n"RESET, args[0]);
        }
        _exit(0);
    }
    else if(t == (int)DPIPE){
        char buf[BUFSIZ] = {0};
        int nread = read(in, buf, BUFSZ);
        char* process = strdup(command);
        char *token = strtok(process, ",");
        int p[2][2];
        int i = 0;
        while(token != NULL){
            if(i >= 2){
                fprintf(stderr, RED"ERROR: TOO MANY ARGS FOR ||\n"RESET);
                _exit(EXIT_FAILURE);
            }
            pipe(p[i]);
            pid_t ch = fork();
           
            if(ch < 0){

            }
            else if(ch == 0){
                close(p[i][1]);
                command = strdup(token);
                printf("--------------------------------------------------\n");
                printf(PURPLE"Executing current command : %s\n"RESET, command);

                preprocess_pipe_io(p[i][0], STDOUT_FILENO);
                command = check_redirection(command, p[i][0], STDOUT_FILENO);
                trim(command, true);
                char** args = tokenise(command);
                char *path = get_path(args[0]);
                if (path == NULL){
                    fprintf(stderr, RED"ERROR : %s: PROGRAM NOT FOUND\n"RESET, args[0]);
                    return false;
                }
                free(command);
                if (execv(path, args) == -1) {
                    fprintf(stderr, RED"FATAL ERROR : %s: PROGRAM CANNOT BE EXECUTED\n"RESET, args[0]);
                }
                _exit(0);
            }
            else{
                close(p[i][0]);
                write(p[i][1], buf, nread);
                close(p[i][1]);
                wait(NULL);
            }
            token = strtok(NULL, ",");
            i++;
        }
    }
    else if(t == (int)TPIPE){
        char buf[BUFSIZ] = {0};
        int nread = read(in, buf, BUFSZ);
        char* process = strdup(command);
        char *token = strtok(process, ",");
        int p[3][2];
        int i = 0;
        while(token != NULL){
            if(i >= 3){
                fprintf(stderr, RED"ERROR: TOO MANY ARGS FOR |||\n"RESET);
                _exit(EXIT_FAILURE);
            }
            pipe(p[i]);
            pid_t ch = fork();
           
            if(ch < 0){

            }
            else if(ch == 0){
                close(p[i][1]);
                command = strdup(token);
                printf("--------------------------------------------------\n");
                printf(PURPLE"Executing current command : %s\n"RESET, command);

                preprocess_pipe_io(p[i][0], STDOUT_FILENO);
                command = check_redirection(command, p[i][0], STDOUT_FILENO);
                trim(command, true);
                char** args = tokenise(command);
                char *path = get_path(args[0]);
                if (path == NULL){
                    fprintf(stderr, RED"ERROR : %s: PROGRAM NOT FOUND\n"RESET, args[0]);
                    return false;
                }
                if (execv(path, args) == -1) {
                    fprintf(stderr, RED"FATAL ERROR : %s: PROGRAM CANNOT BE EXECUTED\n"RESET, args[0]);
                }
                _exit(0);
            }
            else{
                close(p[i][0]);
                write(p[i][1], buf, nread);
                close(p[i][1]);
                wait(NULL);
            }
            token = strtok(NULL, ",");
            i++;
        }
    }
    return false; /* useless */
}

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
    
    if(strstr(command, ">>")){ /* >> */
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
    else if(strstr(command, ">")){
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
    if(strstr(command, "<")){
        token = strtok(NULL, "<");
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

bool preprocess_pipe_io(int in, int out){
    int curr_pid = getpid();
	fprintf(stdout, GREEN"\tPID: %d\n"RESET, getpid());
	fprintf(stderr, GREEN"\tPGID: %d\n\n"RESET, getpgid(curr_pid));

	fprintf(stderr, GREEN"\tRead fd  : %d\n"RESET, in);
	redirect_desc_io(in, STDIN_FILENO);
    fprintf(stderr, GREEN"\tWrite fd : %d\n"RESET, out);
    fprintf(stderr, "--------------------------------------------------\n");
    redirect_desc_io(out, STDOUT_FILENO);
    return true;
}

int execute(char* command){
    token_list* list = parse_cmd(command);
    // printf("List size : %d\n", list->size);
    // print_list(list);

    token_node* temp = list->head;
    int node_no = 1;
    int in = STDIN_FILENO; 
    int prev_t = -1;
    char *pipe_token, *curr_cmd;
    while(temp && temp->next){
        printf("--------------------------------------------------\n");
        curr_cmd = temp->token;
        
        /* Move to one of |, ||, ||| */
        temp = temp->next;
        node_no++;

        pipe_type t;
        if(temp){
            pipe_token = temp->token;
            if(strcmp(pipe_token, "|") == 0){
                t = SPIPE;
            }
            else if(strcmp(pipe_token, "||") == 0){
                t = DPIPE;
            }
            else if(strcmp(pipe_token, "|||") == 0){
                t = TPIPE;
            }
        }
        temp = temp->next;
        node_no++;

        int fd[2]; /* in/out pipe ends */
        if(pipe(fd) == -1)
            perror("pipe");

        pid_t child = fork();

        if(child < 0){ /* Error */
			printf(RED"FATAL ERROR: CAN'T CREATE CHILD PROCESS\n"RESET);
			_exit(EXIT_FAILURE);
		}

        else if(child == 0){    /* run command in the child process */
            close(fd[0]);   
            bool ret = exec_curr_cmd(curr_cmd, prev_t, in, fd[1]);      
            _exit(!ret);

        }
        else{ /* parent */
            assert (child > 0);
            close(fd[1]);       /* close unused write end of the pipe */
            if(node_no != 3)
                close(in);          /* close unused read end of the previous pipe */
            in = fd[0];         /* the next command reads from here */
            prev_t = t;
        
            int status;
            for(;;){
                pid_t leader = waitpid(child, &status, WUNTRACED); 
                if(leader == -1){
                    break;
                }
            }
        }
    }
    /* Last Command in Pipeline */
    fprintf(stderr, "--------------------------------------------------\n");

    if(temp == NULL){
        fprintf(stderr, RED"ERROR: INVALID SYNTAX\n"RESET);
        _exit(0);
    }
    curr_cmd = temp->token;

    pid_t child = fork();
    if(child < 0){ /* Error */
        printf(RED"FATAL ERROR: CAN'T CREATE CHILD PROCESS\n"RESET);
        exit(EXIT_FAILURE);
    }
    else if(child == 0){    /* run command in the child process */
        bool ret = exec_curr_cmd(curr_cmd, prev_t, in, STDOUT_FILENO);
        _exit(!ret);
    }
    else{ /* parent */
        assert (child > 0);
        if(node_no != 3)
            close(in);          /* close unused read end of the previous pipe */    
        int status;
        waitpid(child, &status, WUNTRACED);
    }
    return 1;
    
}

