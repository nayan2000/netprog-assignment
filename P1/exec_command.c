#include "exec_command.h"
void print_broken_cmd(char** args){
    int i = 0;
    while(args[i] != NULL){
        printf("%s ", args[i]);
        i++;
    }
    printf("\n");
}
char** break_loner_cmds(char* cmd){
    char **broken_cmd = (char**)malloc(MAX_SIZE_SINGLE_CMD * sizeof(char *));
	char *process = strdup(cmd);

	char *token = strtok(process, " ");
    if(strcmp(token, "fg") != 0 && strcmp(token, "bg") != 0 && strcmp(token, "sc") != 0 && strcmp(token, "jobs") != 0){
        free(process);
        free(broken_cmd);
        return NULL;
    }

    int i = 0;
	while(token != NULL) {
        // printf("%s ", token);
        broken_cmd[i] = strdup(token);
        token = strtok(NULL, " ");
        i++;
	}
    free(process);
    return broken_cmd;
}

bool fileExists(char* path) {
    struct stat fileinfo;
    if(!stat(path, &fileinfo)) { // GET FILE INFO
        // CHECK IF IT IS A REGULAR FILE AND THAT WE HAVE USER, GROUP AND OTHER EXECUTE PERMISSION
        if(S_ISREG(fileinfo.st_mode) && (fileinfo.st_mode && (S_IXUSR | S_IXGRP | S_IXOTH))) {
            return true;
        } else return false;
    }
}

bool checkPATH(token_list* list) {
    char* path;
    token_node* ptr = list->head;
    while(ptr != NULL) { // CHECK EVERY COMMAND
        if((ptr->token)[0] == '.' || (ptr->token)[0] == '/') { // LOCAL COMMAND
            path = strdup(ptr->token);
            if(!fileExists(path)) return false;
        } else {
            // REMOVE ARGUMENTS FROM COMMAND
            int cmd_size = 0;
            for(int i = 0; (ptr->token)[i] != '\0'; ++i) {
                if((ptr->token)[i] != ' ') cmd_size++;
                else break;
            }
            char* cmd = (char*) malloc(cmd_size);
            for(int i = 0; i< cmd_size; ++i) {
                cmd[i] = (ptr->token)[i];
            }
            bool exists = false;
            char* path_token = strtok(getenv("PATH"), ":");
            while(path_token != NULL) {
                char* possible_path = (char*) malloc(sizeof(path_token) + MAX_CMD_SIZE);
                strcat(possible_path, path_token);
                strcat(possible_path, "/");
                strcat(possible_path, cmd);
                printf("CHECKING: %s\n", possible_path);

                if(fileExists(possible_path)) {
                    exists = true;
                    printf("Exists\n");
                    break;
                } else path_token = strtok(NULL, ":");
            }
            // COMMAND NOT FOUND IN "PATH"
            if(!exists) {
                printf("Not found\n");
                return false;
            }
        }
        if(ptr->next != NULL) ptr = ptr->next->next;
        else ptr = ptr->next;
    }

    return true;
}

bool run_job(char* command){
    token_list *list = parse_cmd(command);
    print_list(list);
    if(checkPATH(list)) {
        printf("All commands are valid\n");
    } else {
        printf("One or more commands are invalid\n");
    }
    if(list->size == 1){ /*possibly fg, bg, shortcut*/
        token_node* node = list->head;
        char** broken_cmd = break_loner_cmds(node->token);
        // print_broken_cmd(broken_cmd);
        if(broken_cmd != NULL){
            if(strcmp(broken_cmd[0], "jobs") == 0){
                print_jobs();
            }
            else if(strcmp(broken_cmd[0], "fg") == 0){
                make_foreground(broken_cmd[1]);
            }
            else if(strcmp(broken_cmd[0], "bg") == 0){
                make_background(broken_cmd[1]);
            }
            else if(strcmp(broken_cmd[0], "sc") == 0){
                
            }
            for(int i = 0; i < MAX_SIZE_SINGLE_CMD; i++)
                if(broken_cmd[i]) free(broken_cmd[i]);
            free(broken_cmd);
            return true;
        }
        
    }
    return false;
}

bool exec_curr_cmd(char* command){
    char *process = strdup(command);
	int num_tokens = 0;
	if(process != NULL){
		char *token = strtok(process, " ");
		num_tokens = 0;
		while (token != NULL) {
			num_tokens++;
			token = strtok(NULL, " ");
		}

		char *args[num_tokens + 1];
        process = strdup(command);
        token = strtok(process, " ");
		int i = 0;
		while (token != NULL) {
			args[i++] = token;
			token = strtok(NULL, " ");
		}
		args[i] = (char*)NULL;

		// char *path = get_path(args[0]);
		// if (path == NULL){
		// 	printf("FATAL ERROR : %s: PROGRAM NOT FOUND\n", args[0]);
		// 	return false;
		// }
		if (execvp(args[0], args) == -1) {
			fprintf(stderr, RED"FATAL ERROR : %s: PROGRAM CANNOT BE EXECUTED\n"RESET, args[0]);
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
    
    if(*(command + strlen(token) + 1) == '>'){ /* >> */
        token = strtok(NULL, ">");
        file = token;
        trim(file, true);
        trim(file, false);

        int fd = open(file, O_CREAT | O_WRONLY | O_APPEND, 0664);
        
        fprintf(stderr, "File name : %s\n", file);
        fprintf(stderr, "\tOriginal Write fd : %d\n", out);
        if (fd < 0) 
            fprintf(stderr, RED"CANNOT OPEN FILE : %s\n", file);
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
            fprintf(stderr, RED"CANNOT OPEN FILE : %s\n", file);
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
            fprintf(stderr, RED"CANNOT OPEN FILE : %s\n", file);
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
	fprintf(stderr, "\tPID: %d\n", getpid());
	fprintf(stderr, "\tPGID: %d\n\n", getpgid(curr_pid));

	fprintf(stderr, "\tReading from fd %d\n", in);
	redirect_desc_io(in, STDIN_FILENO);
    fprintf(stderr, "\tWriting to fd %d\n", out);
    redirect_desc_io(out, STDOUT_FILENO);
    return true;
}

int execute(char* command){
    token_list* list = parse_cmd(command);
    printf("List size : %d\n", list->size);
    print_list(list);

    token_node* temp = list->head;
    int node_no = 1;
    int in = STDIN_FILENO; 
    int prev_t = -1;
    char *pipe_token, *curr_cmd;
    while(temp->next != NULL){
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
        printf("Pipe Descriptors :\n\tRead : %d\n\tWrite: %d\n", fd[0], fd[1]);
        pid_t child = fork();

        if(child < 0){ /* Error */
			printf(RED"FATAL ERROR: CAN'T CREATE CHILD PROCESS\n"RESET);
			exit(EXIT_FAILURE);
		}

        else if(child == 0){    /* run command in the child process */
            close(fd[0]);       /* close unused read end of the pipe */
            printf("Executing current command : %s\n", curr_cmd);
            preprocess_pipe_io(in, fd[1]);
            curr_cmd = check_redirection(curr_cmd, in, fd[1]);
            bool ret = exec_curr_cmd(curr_cmd);
            
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
                pid_t childPid = wait(0);
                if(childPid == -1){
                    if(errno == ECHILD){
                        break;
                    }
                    else{
                        perror("Wait error");
                        printf(RED"Unexpected error while waiting for process %d\n"RESET, childPid);
                        _exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
    printf("--------------------------------------------------\n");
    curr_cmd = temp->token;
    printf("Executing current command : %s\n", curr_cmd);
    preprocess_pipe_io(in, STDOUT_FILENO);
    
    curr_cmd = check_redirection(curr_cmd, in, STDOUT_FILENO);
    bool ret = exec_curr_cmd(curr_cmd); 
}

int main(void) {
    char command[] = "ls -l | sort -k9 >> d.txt";
    execute(command);
}