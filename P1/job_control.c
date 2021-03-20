#include "job_control.h"

void make_background(char* broken_cmd){
    char* temp = broken_cmd;
    int job_no = atoi(temp+1);
    printf("Job no : %d\n", job_no);

    command_details* cmd_rec = get_by_id(job_no);
    if(cmd_rec == NULL){
        printf(RED"FATAL ERROR : JOB DOES NOT EXIST\n"RESET);
        exit(EXIT_FAILURE);
    }
    if(cmd_rec->status != STOP) {
		printf(RED"FATAL ERROR : JOB ALREADY RUNNING IN BACKGROUND\n"RESET);
        exit(EXIT_FAILURE);
	}
    if(cmd_rec->type != BG) {
		printf(RED"FATAL ERROR : JOB DOES NOT EXIST IN BACKGROUND\n"RESET);
        exit(EXIT_FAILURE);
	}
	if(kill(cmd_rec->pgid, SIGCONT) < 0) {
		printf(RED"FATAL ERROR : CAN'T RESTART JOB IN BACKGROUND\n"RESET);
        exit(EXIT_FAILURE);
	}
	update_entry_by_pgid(cmd_rec->pgid, BG, RUN);
}

void make_foreground(char* broken_cmd){
    char* temp = broken_cmd;
    int job_no = atoi(temp+1);
    printf("Job no : %d\n", job_no);
    command_details* cmd_rec = get_by_id(job_no);
    if(cmd_rec == NULL){
        printf(RED"FATAL ERROR : JOB DOES NOT EXIST\n"RESET);
        exit(EXIT_FAILURE);
    }
    if(cmd_rec->type != BG) {
		printf(RED"FATAL ERROR : JOB DOES NOT EXIST IN BACKGROUND\n"RESET);
        exit(EXIT_FAILURE);
	}
	if(/*cmd_rec->status == STOP &&*/ kill(cmd_rec->pgid, SIGCONT) < 0) {
		printf(RED"FATAL ERROR : CAN'T RESTART JOB IN FOREGROUND\n"RESET);
        exit(EXIT_FAILURE);
	}
	update_entry_by_pgid(cmd_rec->pgid, FG, RUN);
    signal(SIGTTOU, SIG_IGN);
    if(isatty(STDIN_FILENO)){
        if(tcsetpgrp(STDIN_FILENO, cmd_rec->pgid) == -1) {
            printf("FATAL ERROR: CAN'T CREATE A NEW PROCESS GROUP\n");
            exit(EXIT_FAILURE);
        }
    }
    else{
        printf("FATAL ERROR: CAN'T CREATE A NEW PROCESS GROUP\n");
        exit(EXIT_FAILURE);
    }
        printf("Yes it is a foreground job now\n");

    int status;
    for(;;){ 
        pid_t leader = waitpid(cmd_rec->pgid, &status, WUNTRACED); 
        if(leader == -1){
            break;
        }
    }
    if(WIFSTOPPED(status)) {
        update_entry_by_pgid(cmd_rec->pgid, BG, STOP);
    }
    
    /* Foreground process gets terminated */
    else if(WIFEXITED(status) || WIFSIGNALED(status)) {
        remove_entry_by_pgid(cmd_rec->pgid);
    }
    /* fg command process leader exits */
    /* Set shell as foreground process for the terminal again */
    tcsetpgrp(STDIN_FILENO, getpgid(0));
    signal(SIGTTOU, SIG_DFL);
}

void print_jobs(){
    for(int i = 0; i < MAX_CMD; i++){
        if(j_table[i] != NULL){
            if(j_table[i]->type == BG){
                printf(PURPLE"%2d | %20s | %8d | %3d\n"RESET, i, j_table[i]->cmd, j_table[i]->pgid,  j_table[i]->status);
            }
            else{
                printf(GREEN"%2d | %20s | %8d | %3d\n"RESET, i, j_table[i]->cmd, j_table[i]->pgid,  j_table[i]->status);
            }
        }
    }
}