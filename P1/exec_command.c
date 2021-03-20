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
bool run_job(char* command){
    token_list *list = parse_cmd(command);
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

// int main(){
//     bool isfg = true;
//     char *command = (char*)malloc(sizeof(char)*max_cmd_sz);
//     /*Primitive processing */
//     bool ignore = process_command(&isfg, command);

//     run_job(command);
// }