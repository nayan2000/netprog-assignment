#include "exec_command.h"
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
        my_msgbuf chbuf;
        chbuf.mtype = 1;
        chbuf.mtext = getpid();
        if (msgsnd(msqid, &chbuf, sizeof(chbuf.mtext), IPC_NOWAIT) == -1){
            perror("msgsnd");
        }
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
	fprintf(stdout, "\tPID: %d\n", getpid());
	fprintf(stderr, "\tPGID: %d\n\n", getpgid(curr_pid));

	fprintf(stderr, "\tReading from fd %d\n", in);
	redirect_desc_io(in, STDIN_FILENO);
    fprintf(stderr, "\tWriting to fd %d\n", out);
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
        // printf("Pipe Descriptors :\n\tRead : %d\n\tWrite: %d\n", fd[0], fd[1]);
        pid_t child = fork();

        if(child < 0){ /* Error */
			printf(RED"FATAL ERROR: CAN'T CREATE CHILD PROCESS\n"RESET);
			_exit(EXIT_FAILURE);
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
                pid_t leader = waitpid(child, &status, WUNTRACED); 
                if(leader == -1){
                    break;
                }
            }
        }
    }
    /* Last Command in Pipeline */
    fprintf(stderr, "--------------------------------------------------\n");
    pid_t child = fork();
    if(child < 0){ /* Error */
        printf(RED"FATAL ERROR: CAN'T CREATE CHILD PROCESS\n"RESET);
        _exit(EXIT_FAILURE);
    }
    else if(child == 0){    /* run command in the child process */
        curr_cmd = temp->token;
        printf("Executing current command : %s\n", curr_cmd);
        preprocess_pipe_io(in, STDOUT_FILENO);
        
        curr_cmd = check_redirection(curr_cmd, in, STDOUT_FILENO);
        bool ret = exec_curr_cmd(curr_cmd);
        _exit(!ret);
    }
    else{ /* parent */
        assert (child > 0);
        if(node_no != 3)
            close(in);          /* close unused read end of the previous pipe */    
        int status;
        do{
            waitpid(child, &status, WUNTRACED);
        }while(
                !WIFSIGNALED(status)		&&
                !WIFEXITED(status)			&&
                !WIFSTOPPED(status)
               	);
        
    }
    return 1;
    
}

// int main(void) {
//     char command[] = "ls -l | sort -k9 >> d.txt";
//     execute(command);
// }
