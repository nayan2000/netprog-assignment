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

// int main(){
//     bool isfg = true;
//     char *command = (char*)malloc(sizeof(char)*max_cmd_sz);
//     /*Primitive processing */
//     bool ignore = process_command(&isfg, command);

//     run_job(command);
// }