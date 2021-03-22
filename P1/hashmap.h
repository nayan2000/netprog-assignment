/********************************************
2017B4A70696P   Abdul Kadir Khimani
********************************************/

/*Header Guard*/
#ifndef HASHMAP_H
#define HASHMAP_H

#include "basic.h"
#define MAX_SC_TABLE_SIZE 40

void print_sc_table();
bool manage_sc_command(char** broken_cmd);
bool remove_from_sc_table(int i, char* cmd);
char* lookup_cmd(int i);
bool add_to_sc_table(int i, char* cmd);

#endif