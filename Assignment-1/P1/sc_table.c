#include "sc_table.h"
char* sc_table[MAX_SC_TABLE_SIZE];

bool add_to_sc_table(int i, char* cmd){
    if(sc_table[i] != NULL){
        printf(RED"ERROR: CHOOSE ANOTHER INDEX\n"RESET);
        return false;
    }
    sc_table[i] = strdup(cmd);
    return true;
}

char* lookup_cmd(int i){
    if(sc_table[i] == NULL)
        return NULL;
    return sc_table[i];
}

bool remove_from_sc_table(int i){
    if(sc_table[i] == NULL){
        printf(RED"ERROR: ENTRY DOES NOT EXIST\n"RESET);
        return false;
    }

    free(sc_table[i]);
    sc_table[i] = NULL;
    return true;
}

bool manage_sc_command(char** broken_cmd, token_list* list){
    if(strcmp(broken_cmd[1], "-i") != 0 && strcmp(broken_cmd[1], "-d")){
        printf(RED"ERROR: INVALID COMMAND\n"RESET);
        return false;
    }
    if(strcmp(broken_cmd[1], "-d") == 0){
        bool ret = remove_from_sc_table(atoi(broken_cmd[2]));
        return ret;
    }

    if(atoi(broken_cmd[2]) >= MAX_SC_TABLE_SIZE){
        printf(RED"ERROR: INVALID INDEX - CHOOSE LESS THAN %d\n"RESET, MAX_SC_TABLE_SIZE);
        return false;
    }
    char *command = strdup(broken_cmd[3]);
    int i = 4;
    while(broken_cmd[i]){
        strcat(command, " ");
        strcat(command, broken_cmd[i]);
        i+=1;
    }
    token_node* node = list->head->next;
    while(node){
        strcat(command, " ");
        strcat(command, node->token);
        node = node->next;
    }
    puts(command);
    if(strcmp(broken_cmd[1], "-i") == 0){
        bool ret = add_to_sc_table(atoi(broken_cmd[2]), command);
        return ret;
    }
    
}

void print_sc_table(){
    for(int i = 0; i < MAX_SC_TABLE_SIZE; i++){
        if(sc_table[i]){
            fprintf(stderr, "[%d]\t%s\n", i, sc_table[i]);
        }
    }
}