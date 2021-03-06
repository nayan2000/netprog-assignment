#include "shell.h"
size_t max_cmd_sz = CMD_SZ + 1;
int msqid;
key_t key;

void remove_queue(){
    if (msgctl(msqid, IPC_RMID, NULL) == -1) perror("msgctl"); 
}

void grim_reaper(int sig){
    if(sig == SIGCHLD){
        my_msgbuf buf;
        int read;
        msgrcv(msqid, &(buf), sizeof(buf.mtext), 1, IPC_NOWAIT);

        int i = pgid_to_id(buf.mtext);
        if(i != -1){
            char *cmd = strdup(j_table[i]->cmd);
            printf(GREEN"\n[%d] \t Job Terminated\t %s\n"RESET, i, cmd);
        }
        remove_entry_by_pgid(buf.mtext);
    }
}

void shortcut_handler(int sig){
    printf(PURPLE"\n----ENTERED SHORTCUT MODE----\n"RESET);
    printf(YELLOW"\n>"RESET);
    fflush(stdout);

    int i;
    scanf("%d", &i);
    char* command = lookup_cmd(i);
    if(command == NULL) {
        printf("ERROR: ENTRY DOES NOT EXIST FOR %d\n", i);
        return;
    }

    bool isfg = true;
    for(int i = strlen(command) - 1; i >= 0; i--){
        if(command[i] == '\n')
            command[i] = 0;
        if(command[i] == '&'){
            isfg = false;
            command[i] = 0;
            break;
        }
    }
    bool ret = run_job(command);
    if(ret) return;
    int pid = fork();
    if(pid > 0){
        if(isfg){
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(STDIN_FILENO, pid);
            while(waitpid(pid, NULL, WUNTRACED) > 0){

            }

            tcsetpgrp(STDIN_FILENO, getpid());
            signal(SIGTTOU, SIG_DFL);
        }
        
    }
    else{
        setpgid(0, getpid());
        execute(command);
        fflush(stdin);

        _exit(0);
    }
}

void initial_setup(){
    signal(SIGCHLD, grim_reaper);
    signal(SIGINT, shortcut_handler);
    if (atexit(remove_queue) != 0) perror("atexit");
    if((key = ftok(MSGQ_PATH, 'B')) == -1){
        perror("ftok");
        exit(1);
    }
    if((msqid = msgget(key,  IPC_CREAT | 0644)) == -1){
        perror("msgget");
        exit(1);
    }
}

/*  Use    - Processes input shell command to a proper format
    Input  - 
        bool  : isfg    - Pointer to variable indicating fg or bg command
        char* : command - Pointer to buffer that contains the shell command
    Output -
        bool  : ignore  - Indicates whether to ignore the given shell input or not
*/
bool process_command(bool *isfg, char * command){
    bool ignore;
    
    
    /* Get command from the terminal */
    fflush(stdin);
    memset(command, '\0',sizeof(command));
    int cmd_sz = getline(&command, &max_cmd_sz, stdin);
    if(cmd_sz == -1 || cmd_sz == 0 || strcmp(command, "\n") == 0)
        return ignore = true;
    /* Remove newline character */

    command[cmd_sz - 1] = '\0';

    trim(command, " ");
    if(strlen(command) == 1 || strlen(command) == 0) {
        return true;
    }
    char s[3] = {0};
    s[0] = command[0];
    s[1] = command[1];
    /* Check if it is a background job */
    if(strcmp("sc", s) != 0 && command[cmd_sz - 2] == '&') {
        command[cmd_sz - 2] = '\0';
        *isfg = false;
    }
    if(strcmp("quit", command) && strcmp("exit", command))
        printf("Command : %s\n", command);
    return ignore = false;
}

void clear_screen(){
  const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
  write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}
/*  Use    - Shell Driver function
             Runs the main shell program
*/
int main(int argc, char* argv[]){
    clear_screen();
    int num = 1;
    initial_setup();

    printf(GREEN"\n************* SHELL EXECUTION STARTED *************\n"RESET);
    printf(BLUE"\nShell Details:\n");
			printf("\tPID  : %d\n", getpid());
			printf("\tPGID : %d\n", getpgid(getpid()));
			printf("\n"RESET);
    /* Loop the shell forever */
    while(1){  
        printf(PROMPT);
        fflush(stdout);
        fflush(stdin);
        bool isfg = true;
        char *command = (char*)malloc(sizeof(char)*max_cmd_sz);
        
        /*Primitive processing */
        bool ignore = process_command(&isfg, command);
        if(ignore) continue;
        if(strcmp(command, "quit") == 0 || strcmp("exit", command) == 0) exit(EXIT_SUCCESS);
        if(command[0] == 'c' && command[1] == 'd'){
            if(strcmp("cd", command) == 0){
                strcat(command, " ");
                strcat(command, getenv("HOME"));
            }
            else if(command[3] = '~' && command[2] == ' '){
                strcat(command, " ");
                strcat(command, getenv("HOME"));
            }
            char *path = command + 3;
            if (chdir(path) < 0) {
                perror(RED"CD EXECUTION ERROR"RESET);
                _exit(EXIT_FAILURE);
            }
            continue;
        }
        /* Check for singular fg, bg, sc, and jobs command */
        bool ret = run_job(command);
        int status;

        for(int i = 0; i < MAX_CMD; i++){
            if(j_table[i]){
                if(j_table[i]->type == FG){
                    pid_t ch = j_table[i]->pgid;
                    while(waitpid(-1*ch, &status, WUNTRACED)>0);
                    break;
                }
            }
        }
        tcsetpgrp(STDIN_FILENO, getpid());
        if(kill(getpid(), SIGCONT) < 0) {
            printf(RED"FATAL ERROR : CAN'T RESTART SHELL IN FOREGROUND\n"RESET);
            exit(EXIT_FAILURE);
	    }
        signal(SIGTTOU, SIG_DFL);
        if(ret) continue;
        
        /* The shell should now make a new process group for each command
        and exec() it using a new child process */

        /* We perform process group creation and shifting it to bg/fg using the 
        parent shell process because if this is done in the child process itself,
        terminal signals won't be sent to groups not created by the shell */
		
        /* Child process must wait for shell to perform job control tasks, so we synchronise 
        using pipes. We can also use signals for synchronisation, but using pipes is easier */

		int p[2];
        int p_sync[2];
		pipe(p);
        command_details* cmd_rec = (command_details*)malloc(sizeof(command_details));   

		pid_t child = fork();

		if(child < 0){ /* Error */
			printf(RED"FATAL ERROR: CAN'T CREATE CHILD PROCESS\n"RESET);
			exit(EXIT_FAILURE);
		}
		else if(child == 0){ /* Child */
			if (close(p[1]) == -1) /* Close unused write end */
                _exit(EXIT_FAILURE); 
            int dummy;
            /* Wait till child sees EOF from pipe */
			int n = read(p[0], &dummy, 1);
            if (close(p[0]) == -1) 
                _exit(EXIT_FAILURE);

			/* Start execution of command */
            execute(command);
            if(!isfg){
                my_msgbuf chbuf;
                chbuf.mtype = 1;
                chbuf.mtext = getpid();
                if (msgsnd(msqid, &chbuf, sizeof(chbuf.mtext), IPC_NOWAIT) == -1){
                    perror("msgsnd");
                }
            }
            
		    pipe(p_sync);
            close(p_sync[0]); /* Close unused read end */
            close(p_sync[1]);
            _exit(EXIT_SUCCESS);
		}
        else{ /* Parent */
            close(p[0]); /* Close unused read end */
        
            if(!isfg){
                cmd_rec->pgid = child == 0? getpid() : child;
                cmd_rec->cmd = strdup(command);
                cmd_rec->type = isfg? FG : BG;
                cmd_rec->status = RUN;
                add_entry(cmd_rec);
            }
            
            
            /*Create a new process group for the command */
			if(setpgid(child, child) == -1) {
				printf("FATAL ERROR: CAN'T CREATE A NEW PROCESS GROUP\n");
				exit(EXIT_FAILURE);
			}

            /* Print Child Details */
            int curr_pid = child;
			printf(YELLOW"Command Details:\n"RESET);
			printf(YELLOW"\tPID  : %d\n"RESET, curr_pid);
			printf(YELLOW"\tPGID : %d\n"RESET, getpgid(curr_pid));
			printf(YELLOW"\tStatus : RUNNING\n"RESET);
            if(isfg)
                printf(YELLOW"\tType : FOREGROUND\n"RESET);
            else
                printf(YELLOW"\tType : BACKGROUND\n"RESET);

			printf("\n");

            /* Set child as the foreground process group for the terminal*/
			if (isfg){
                signal(SIGTTOU, SIG_IGN);
				if(isatty(STDIN_FILENO)){
                    if(tcsetpgrp(STDIN_FILENO, child) == -1) {
                        printf("FATAL ERROR: CAN'T CREATE A NEW PROCESS GROUP\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else{
                    printf("FATAL ERROR: CAN'T CREATE A NEW PROCESS GROUP\n");
                    exit(EXIT_FAILURE);
                }
			}
            /* Synchronise Child */
            if (close(p[1]) == -1) exit(EXIT_FAILURE);
            int status;

            /* If it is a fg command, shell process must wait for it ro complete exexution */
            if(isfg){
                for (;;) { 
                    pid_t leader = waitpid(-1 * child, &status, WUNTRACED); 
                    if(leader == -1){
                        break;
                    }
                }
                if(WIFSTOPPED(status)) {
                    cmd_rec->pgid = child == 0? getpid() : child;
                    cmd_rec->cmd = strdup(command);
                    cmd_rec->type = BG;
                    cmd_rec->status = STOP;
                    add_entry(cmd_rec);
			 	}
			 
			 	/* Foreground process gets terminated */
			 	else if(WIFEXITED(status) || WIFSIGNALED(status)) {
					// remove_entry_by_pgid(child);
				}
                /* fg command process leader exits */
                /* Set shell as foreground process for the terminal again */
                tcsetpgrp(STDIN_FILENO, getpid());
				signal(SIGTTOU, SIG_DFL);
            }
            close(p_sync[1]);/* Close unused write end */
            int syn_var;
            /* Wait till parent sees EOF from pipe */
            int n = read(p_sync[0], &syn_var, 1);
            close(p_sync[0]); 
        }
        
    }
    if (msgctl(msqid, IPC_RMID, NULL) == -1) perror("msgctl"); 
    exit(EXIT_SUCCESS);
}