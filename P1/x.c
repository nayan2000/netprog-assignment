#include "hashmap.h"
char* sc_table[MAX_SC_SIZE];

bool add_to_sc_table(int i, char* cmd){
    if(sc_table[i] != NULL){
        printf("CHOOSE ANOTHER INDEX\n");
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

bool remove_from_sc_table(int i, char* cmd){
    if(sc_table[i] == NULL){
        printf("ENTRY DOES NOT EXIST\n");
        return false;
    }
    if(strcmp(cmd, sc_table[i]) != 0){
        printf("COMMAND AND INDEX MISMATCH\n");
        return false;
    }

    free(sc_table[i]);
    sc_table[i] = NULL;
    return true;
}

bool manage_sc_command(char** broken_cmd){
    if(strcmp(broken_cmd[1], "-i") != 0 && strcmp(broken_cmd[1], "-d")){
        printf(RED"Invalid Command\n"RESET);
        return false;
    }

    if(atoi(broken_cmd[2]) >= MAX_SC_SIZE){
        printf(RED"Invalid Index : Choose less than %d\n"RESET, MAX_SC_SIZE);
        return false;
    }
    if(strcmp(broken_cmd[1], "-i") == 0){
        bool ret = add_to_sc_table(atoi(broken_cmd[2]), broken_cmd[3]);
        return ret;
    }
    if(strcmp(broken_cmd[1], "-d") == 0){
        bool ret = remove_from_sc_table(atoi(broken_cmd[2]), broken_cmd[3]);
        return ret;
    }
}

void print_sc_table(){
    for(int i = 0; i < MAX_SC_SIZE; i++){
        if(sc_table[i]){
            fprintf(stderr, "[%d]\t%s\n", i, sc_table[i]);
        }
    }
}