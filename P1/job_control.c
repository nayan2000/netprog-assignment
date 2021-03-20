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
    printf(PURPLE"%2d | %20s | %8d | %3d | %3d\n"RESET, job_no, cmd_rec->cmd, cmd_rec->pgid, cmd_rec->status, cmd_rec->type);
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
    update_entry_by_pgid(cmd_rec->pgid, FG, RUN);
    command_details* cmd_rec1 = get_by_id(job_no);
    printf(PURPLE"%2d | %20s | %8d | %3d | %3d\n"RESET, job_no, cmd_rec1->cmd, cmd_rec1->pgid, cmd_rec1->status, cmd_rec1->type);
    printf("Yes it is a foreground job now\n");

   
    /* fg command process leader exits */
    /* Set shell as foreground process for the terminal again */
    // tcsetpgrp(STDIN_FILENO, getpgid(0));
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