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
    printf(GREEN"%2d | %20s | %8d | %3d | %3d\n"RESET, job_no, cmd_rec->cmd, cmd_rec->pgid, cmd_rec->status, cmd_rec->type);
    printf("Yes, it is a foreground job now\n");
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
bool run_job(char* command){
    token_list *list = parse_cmd(command);
    if(list->size == 1){ /*possibly fg, bg, jobs, shortcut*/
        token_node* node = list->head;
        char** broken_cmd = break_loner_cmds(node->token); /* Returns a NULL if not fg, bg, sc, jobs */
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

// int main(){
//     bool isfg = true;
//     char *command = (char*)malloc(sizeof(char)*max_cmd_sz);
//     /*Primitive processing */
//     bool ignore = process_command(&isfg, command);

//     run_job(command);
// }